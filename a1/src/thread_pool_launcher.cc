#include <stdlib.h>
#include <thread_pool_launcher.h>
#include <unistd.h>
#include <utils.h>
#include <cassert>
#include <iostream>

ThreadPoolLauncher::ThreadPoolLauncher(int pool_sz) : Launcher()
{
    assert(pool_sz > 0);

    int i;
    pthread_t *thread;
    thread_state *states;

    /* Initialize ThreadPoolLauncher fields */
    pool_sz_ = pool_sz;

    /*
     * YOUR CODE HERE
     *
     * Initialize nthreads_idle_ (nthreads_idle_mutex_ and nthreads_idle_cond_)
     * and free_list_mutex_.
     *
     * free_list_mutex_ protects the free-list of thread_states, which
     * corresponds to the list of idle threads.
     *
     * nthreads_idle_ is used to track the number of idle/busy processes.
     *
     * When your code is ready, remove the assert(false) statement
     * below.
     */
    assert(false);

    /*
     * Initialize thread_state structs. Each thread in the thread pool is
     * assigned a thread_state struct.
     */
    states = (thread_state *)malloc(sizeof(thread_state) * pool_sz);
    for (i = 0; i < pool_sz; ++i)
    {
        thread = (pthread_t *)malloc(sizeof(pthread_t));

        /*
         * Each thread in the pool has a corresponding semaphore which
         * is used to signal the thread to begin executing a new
         * request. We initialize the value of this semaphore to 0.
         */
        states[i].thread_cond_         = PTHREAD_COND_INITIALIZER;
        states[i].thread_mutex_        = PTHREAD_MUTEX_INITIALIZER;
        states[i].thread_done_         = false;
        states[i].req_                 = NULL;
        states[i].nthreads_idle_       = &nthreads_idle_;
        states[i].nthreads_idle_mutex_ = &nthreads_idle_mutex_;
        states[i].nthreads_idle_cond_  = &nthreads_idle_cond_;
        states[i].thread_id_           = thread;
        states[i].list_mutex_          = &free_list_mutex_;
        states[i].txns_executed_       = txns_executed_;
        states[i].list_ptr_            = &states[i + 1];
        states[i].free_list_           = &free_list_;
    }
    states[i - 1].list_ptr_ = NULL;
    free_list_              = states;

    /*
     * YOUR CODE HERE
     *
     * Create threads in the thread pool.
     *
     * When your code is ready, remove the assert(false) statement
     * below.
     */
    assert(false);
}

ThreadPoolLauncher::~ThreadPoolLauncher()
{
    int err;

    pthread_mutex_destroy(&nthreads_idle_mutex_);
    err = munmap((void *)&nthreads_idle_mutex_, sizeof(pthread_mutex_t));
    assert(err == 0);

    pthread_cond_destroy(&nthreads_idle_cond_);
    err = munmap((void *)&nthreads_idle_cond_, sizeof(pthread_cond_t));
    assert(err == 0);

    pthread_mutex_destroy(&free_list_mutex_);
    err = munmap((void *)&free_list_mutex_, sizeof(pthread_mutex_t));
    assert(err == 0);

    while (free_list_ != NULL)
    {
        pthread_mutex_destroy(&nthreads_idle_mutex_);
        err = munmap((void *)&nthreads_idle_mutex_, sizeof(pthread_mutex_t));
        assert(err == 0);

        pthread_cond_destroy(&nthreads_idle_cond_);
        err = munmap((void *)&nthreads_idle_cond_, sizeof(pthread_cond_t));
        assert(err == 0);

        pthread_mutex_destroy(&free_list_mutex_);
        err = munmap((void *)&free_list_mutex_, sizeof(pthread_mutex_t));
        assert(err == 0);

        pthread_join(*free_list_->thread_id_, NULL);
        free_list_ = free_list_->list_ptr_;
    }
    free(free_list_);
}

void ThreadPoolLauncher::ExecuteRequest(Request *req)
{
    Launcher::ExecuteRequest(req);

    /*
     * YOUR CODE HERE
     *
     * Find an idle thread from the free-list, and execute the request on
     * the idle process.
     *
     * Hint:
     * 1. Use nthreads_idle_, nthreads_idle_mutex_, and nthreads_idle_cond_
     * 2. Use thread_done_, thread_mutex_, thread_cond_
     * to coordinate the execution of threads in the pool and the launcher.
     *
     * When your code is ready, remove the assert(false) statement
     * below.
     */
    assert(false);
}

void *ThreadPoolLauncher::ExecutorFunc(void *arg)
{
    thread_state *st;

    st = (thread_state *)arg;
    while (true)
    {
        /*
         * YOUR CODE HERE
         *
         * Wait for a new request, and execute it. After executing the
         * request return thread_state to the launcher's thread_state
         * free-list.
         *
         * When your code is ready, remove the assert(false) statement
         * below.
         */
        assert(false);

        /* exec request */
        st->req_->Execute();
        fetch_and_increment(st->txns_executed_);

        /*
         * YOUR CODE HERE
         */
        assert(false);
    }
    return NULL;
}
