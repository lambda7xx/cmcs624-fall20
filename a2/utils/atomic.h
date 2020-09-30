
#ifndef _DB_UTILS_ATOMIC_H_
#define _DB_UTILS_ATOMIC_H_

#include <boost/thread/shared_mutex.hpp>
#include <mutex>
#include <queue>
#include <set>
#include <unordered_map>

#include <assert.h>

using std::queue;
using std::set;

/// @class AtomicMap<K, V>
///
/// Atomically readable, atomically mutable unordered associative container.
/// Implemented as a std::unordered_map guarded by a pthread rwlock.
/// Supports CRUD operations only. Iterators are NOT supported.
template <typename K, typename V>
class AtomicMap
{
   public:
    AtomicMap() {}
    // Returns the number of key-value pairs currently stored in the map.
    int Size()
    {
        boost::shared_lock<boost::shared_mutex> lock(mutex_);
        return map_.size();
    }

    // Returns true if the map contains a pair with key equal to 'key'.
    bool Contains(const K& key)
    {
        boost::shared_lock<boost::shared_mutex> lock(mutex_);
        return map_.count(key) > 0;
    }

    // If the map contains a pair with key 'key', sets '*value' equal to the
    // associated value and returns true, else returns false.
    bool Lookup(const K& key, V* value)
    {
        boost::shared_lock<boost::shared_mutex> lock(mutex_);
        auto search = map_.find(key);
        if (search != map_.end())
        {
            *value = search->second;
            return true;
        }
        return false;
    }

    // Atomically inserts the pair (key, value) into the map (clobbering any
    // previous pair with key equal to 'key'.
    void Insert(const K& key, const V& value)
    {
        boost::upgrade_lock<boost::shared_mutex> lock(mutex_);
        boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);
        map_[key] = value;
    }

    // Synonym for 'Insert(key, value)'.
    void Set(const K& key, const V& value) { Insert(key, value); }
    // Atomically erases any pair with key 'key' from the map.
    void Erase(const K& key)
    {
        boost::upgrade_lock<boost::shared_mutex> lock(mutex_);
        boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);
        map_.erase(key);
    }

   private:
    std::unordered_map<K, V> map_;
    boost::shared_mutex mutex_;
};

/// @class AtomicSet<K>
///
/// Atomically readable, atomically mutable container.
/// Implemented as a std::set guarded by a pthread rwlock.
/// Supports CRUD operations only. Iterators are NOT supported.
template <typename V>
class AtomicSet
{
   public:
    AtomicSet() {}
    // Returns the number of key-value pairs currently stored in the map.
    int Size()
    {
        boost::shared_lock<boost::shared_mutex> lock(mutex_);
        return set_.size();
    }

    // Returns true if the set contains V value.
    bool Contains(const V& value)
    {
        boost::shared_lock<boost::shared_mutex> lock(mutex_);
        return set_.count(value) > 0;
    }

    // Atomically inserts the value into the set.
    void Insert(const V& value)
    {
        boost::upgrade_lock<boost::shared_mutex> lock(mutex_);
        boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);
        set_.insert(value);
    }

    // Atomically erases the object value from the set.
    void Erase(const V& value)
    {
        boost::upgrade_lock<boost::shared_mutex> lock(mutex_);
        boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);
        set_.erase(value);
    }

    V GetFirst()
    {
        boost::shared_lock<boost::shared_mutex> lock(mutex_);
        return *(set_.begin());
    }

    // Returns a copy of the underlying set.
    set<V> GetSet()
    {
        boost::shared_lock<boost::shared_mutex> lock(mutex_);
        set<V> my_set(set_);
        return my_set;
    }

   private:
    set<V> set_;
    boost::shared_mutex mutex_;
};

/// @class AtomicQueue<T>
///
/// Queue with atomic push and pop operations.
///
/// @TODO(alex): This should use lower-contention synchronization.
template <typename T>
class AtomicQueue
{
   public:
    AtomicQueue() {}
    // AtomicQueue(const AtomicQueue&) = delete;
    // AtomicQueue& operator=(const AtomicQueue&) = delete;

    // Returns the number of elements currently in the queue.
    int Size()
    {
        std::unique_lock<std::mutex> mlock(mutex_);
        int size = queue_.size();
        return size;
    }

    // Atomically pushes 'item' onto the queue.
    void Push(const T& item)
    {
        std::unique_lock<std::mutex> mlock(mutex_);
        queue_.push(std::move(item));
    }

    void UnSafePush(const T& item) { queue_.push(item); }
    // If the queue is non-empty, (atomically) sets '*result' equal to the front
    // element, pops the front element from the queue, and returns true,
    // otherwise returns false.
    bool Pop(T* result)
    {
        std::unique_lock<std::mutex> mlock(mutex_);
        if (!queue_.empty())
        {
            *result = queue_.front();
            queue_.pop();
            return true;
        }
        else
        {
            return false;
        }
    }

    // If mutex is immediately acquired, pushes and returns true, else immediately
    // returns false.
    bool PushNonBlocking(const T& item)
    {
        if (mutex_.try_lock())
        {
            queue_.push(std::move(item));
            mutex_.unlock();
            return true;
        }
        else
        {
            return false;
        }
    }

    // If mutex is immediately acquired AND queue is nonempty, pops and returns
    // true, else returns false.
    bool PopNonBlocking(T* result)
    {
        if (mutex_.try_lock())
        {
            if (!queue_.empty())
            {
                *result = queue_.front();
                queue_.pop();
                mutex_.unlock();
                return true;
            }
            else
            {
                mutex_.unlock();
                return false;
            }
        }
        else
        {
            return false;
        }
    }

   private:
    queue<T> queue_;
    std::mutex mutex_;
};

#endif  // _DB_UTILS_ATOMIC_H_
