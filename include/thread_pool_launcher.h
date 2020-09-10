#ifndef THREAD_POOL_LAUNCHER_H_
#define THREAD_POOL_LAUNCHER_H_

#include <launcher.h>
#include <pthread.h>
#include <semaphore.h>

struct thread_state
{
    Request *req_; /* request to process */

    pthread_mutex_t thread_mutex_; /* thread mutex exclusive lock */
    pthread_cond_t thread_cond_;   /* thread conditional variable */
    bool thread_done_;             /* notify thread of a request */

    uint32_t *nthreads_idle_;              /* # idle threads */
    pthread_mutex_t *nthreads_idle_mutex_; /* mutex exclusive lock for idle threads */
    pthread_cond_t *nthreads_idle_cond_;   /* conditional variable for idle threads */

    pthread_t *thread_id_;        /* identifier of current thread */
    pthread_mutex_t *list_mutex_; /* lock thread free list */
    thread_state **free_list_;    /* ptr to launcher's free list */
    thread_state *list_ptr_;      /* thread_state free list link */

    volatile uint64_t *txns_executed_; /* ptr to txn executed counter */
};

class ThreadPoolLauncher : public Launcher
{
   private:
    uint32_t pool_sz_; /* pool size */

    uint32_t nthreads_idle_;              /* # idle threads */
    pthread_mutex_t nthreads_idle_mutex_; /* mutex exclusive lock for idle threads */
    pthread_cond_t nthreads_idle_cond_;   /* conditional variable for idle threads */

    pthread_mutex_t free_list_mutex_; /* free list lock */
    thread_state *free_list_;         /* free list of idle threads */

    /* Executed by threads in the thread pool */
    static void *ExecutorFunc(void *arg);

   public:
    ThreadPoolLauncher(int pool_sz);
    ~ThreadPoolLauncher();
    void ExecuteRequest(Request *req);
};

#endif  // THREAD_POOL_LAUNCHER_H_
