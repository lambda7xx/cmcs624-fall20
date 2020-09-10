#ifndef PROCESS_POOL_LAUNCHER_H_
#define PROCESS_POOL_LAUNCHER_H_

#define RQST_BUF_SZ (1 << 10) /* 1K per request */

#include <launcher.h>
#include <request.h>
#include <semaphore.h>

struct proc_state;

struct proc_mgr
{
    volatile uint32_t *nprocs_idle_;     /* # idle processes */
    pthread_mutex_t *nprocs_idle_mutex_; /* mutex exclusive lock for idle processes */
    pthread_cond_t *nprocs_idle_cond_;   /* conditional variable for idle processes */

    pthread_mutex_t *free_list_mutex_; /* locks free list below */
    proc_state *free_list_;            /* free list of processes */
};

struct proc_state
{
    Request *request_; /* request location */

    pthread_mutex_t *proc_mutex_; /* proc mutex exclusive lock */
    pthread_cond_t *proc_cond_;   /* proc conditional variable */
    bool *proc_done_;             /* notify proc of a request */

    proc_mgr *launcher_state_;         /* global pool mgmt state */
    volatile uint64_t *txns_executed_; /* ptr to txn executed counter */
    proc_state *list_ptr_;             /* links proc states */
};

class ProcessPoolLauncher : public Launcher
{
   private:
    uint32_t pool_sz_;
    proc_mgr *launcher_state_;

    static void ExecutorFunc(proc_state *st); /* executed by pooled procs */

   public:
    ProcessPoolLauncher(uint32_t pool_sz);
    ~ProcessPoolLauncher();
    void ExecuteRequest(Request *req);
};

#endif  // PROCESS_POOL_LAUNCHER_H_
