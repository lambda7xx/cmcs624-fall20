#ifndef THREAD_POOL2_H
#define THREAD_POOL2_H

#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <thread>

template <typename T>
class SyncQueue
{
   public:
    SyncQueue(int maxSize) : maxSize_(maxSize), needStop_(false) {}
    void Put(const T& x) { Add(x); }
    void Put(T&& x) { Add(std::forward<T>(x)); }
    void Take(T& t)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        notEmpty_.wait(lock, [this] { return needStop_ || NotEmpty(); });
        if (needStop_) return;
        t = queue_.front();
        queue_.pop_front();
        notFull_.notify_one();
    }

    void Stop()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            needStop_ = true;
        }
        notFull_.notify_all();
        notEmpty_.notify_all();
    }
    bool Empty()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }
    bool Full()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size() == maxSize_;
    }
    size_t Size()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }
    int Count() { return queue_.size(); }
   private:
    bool NotFull() const
    {
        bool full = queue_.size() >= maxSize_;
        return !full;
    }
    bool NotEmpty() const
    {
        bool empty = queue_.empty();
        return !empty;
    }

    template <typename F>
    void Add(F&& x)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        notFull_.wait(lock, [this]() { return needStop_ || NotFull(); });
        if (needStop_) return;
        queue_.push_back(std::forward<F>(x));
        notEmpty_.notify_one();
    }

   private:
    std::list<T> queue_;
    std::mutex mutex_;
    std::condition_variable notEmpty_;
    std::condition_variable notFull_;
    int maxSize_;
    bool needStop_;
};

const int g_maxThreadCnt = 100;

class ThreadPool
{
   public:
    using Task = std::function<void()>;
    ThreadPool(int numThreads = std::thread::hardware_concurrency()) : queue_(g_maxThreadCnt) { Start(numThreads); }
    ~ThreadPool() { Stop(); }
    void Stop()
    {
        std::call_once(flag_, [this] { StopThreadGroup(); });
    }

    void AddTask(Task&& task) { queue_.Put(std::forward<Task>(task)); }
    void AddTask(const Task& task) { queue_.Put(task); }
   private:
    void Start(int numThreads)
    {
        running_ = true;
        // create threads
        for (int i = 0; i < numThreads; ++i)
        {
            threadgroup_.push_back(std::make_shared<std::thread>(&ThreadPool::RunInThread, this));
        }
    }

    void RunInThread()
    {
        while (running_)
        {
            Task task;
            queue_.Take(task);
            if (!running_) return;
            task();
        }
    }

    void StopThreadGroup()
    {
        running_ = false;
        queue_.Stop();
        for (auto thread : threadgroup_)
        {
            if (thread)
            {
                thread->join();
            }
        }
        threadgroup_.clear();
    }

    std::list<std::shared_ptr<std::thread>> threadgroup_;
    SyncQueue<Task> queue_;
    std::atomic_bool running_;
    std::once_flag flag_;
};

#endif

#ifndef THREAD_POOL2_H
#define THREAD_POOL2_H

#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>

class ThreadPool
{
   public:
    ThreadPool(size_t);
    template <class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;
    ~ThreadPool();

   private:
    // need to keep track of threads so we can join them
    std::vector<std::thread> workers;
    // the task queue
    std::queue<std::function<void()>> tasks;

    // synchronization
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};

// the constructor just launches some amount of workers
inline ThreadPool::ThreadPool(size_t threads) : stop(false)
{
    for (size_t i = 0; i < threads; ++i)
        workers.emplace_back([this] {
            for (;;)
            {
                std::function<void()> task;

                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex);
                    this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
                    if (this->stop && this->tasks.empty()) return;
                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }

                task();
            }
        });
}

// add new work item to the pool
template <class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>
{
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task =
        std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);

        // don't allow enqueueing after stopping the pool
        if (stop) throw std::runtime_error("enqueue on stopped ThreadPool");

        tasks.emplace([task]() { (*task)(); });
    }
    condition.notify_one();
    return res;
}

// the destructor joins all threads
inline ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread& worker : workers) worker.join();
}

#endif
