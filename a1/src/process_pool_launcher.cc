#include <process_pool_launcher.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utils.h>
#include <cassert>
#include <iostream>

ProcessPoolLauncher::ProcessPoolLauncher(uint32_t nprocs) : Launcher()
{
    uint32_t i;
    char *req_bufs;
    proc_state *pstates;

    /*
     * Setup launcher state. Initialize the launcher's proc_mgr struct.
     */
    launcher_state_ = (proc_mgr *)mmap(NULL, sizeof(proc_mgr), PROT_FLAGS, MAP_FLAGS, 0, 0);

    /*
     * YOUR CODE HERE
     *
     * Initialize launcher_state_->nprocs_idle_, nprocs_idle_mutex_ and
     * nprocs_idle_cond_ with an appropriate starting value/attributes.
     * nprocs_idle_ is used to track the number of idle processes.
     * ProcessPoolLauncher's use of nprocs_idle_ is similar to
     * ProcessLauncher's use of max_outstanding_ to control the number
     * of outstanding processes.
     *
     * Hint: Since they are shared among multiple processes, their
     * argument must be set appropriately.
     *
     * When your code is ready, remove the assert(false) statement
     * below.
     */
    assert(false);

    /*
     * YOUR CODE HERE
     *
     * free_list_mutex_ is used as a mutex lock. It protects the free-list of
     * idle processes from concurrent modifications.
     * Initialize free_list_mutex_ with an appropriate starting value.
     *
     * Hint: Since free_list_mutex_ is shared among multiple processes, its
     * second argument must be set appropriately.
     *
     * When your code is ready, remove the assert(false) statement
     * below.
     */
    assert(false);

    /*
     * Setup request buffers. Each process in the pool has its own private
     * request buffer. In order to assign a request to a process the
     * launcher process must copy the request into the process' request
     * buffer.
     */
    req_bufs = (char *)mmap((NULL), nprocs * RQST_BUF_SZ, PROT_FLAGS, MAP_FLAGS, 0, 0);

    /*
     * YOUR CODE HERE
     *
     * Each process in the pool has a corresponding mutex, conditional variable
     * and value (proc_dones) which are used to signal the process to begin executing
     * a new request.
     *
     * Initialize proc_dones, proc_mutexs and proc_condis
     *
     * When your code is ready, remove the assert(false) statement
     * below.
     */
    bool *proc_dones             = NULL;
    pthread_mutex_t *proc_mutexs = NULL;
    pthread_cond_t *proc_condis  = NULL;
    assert(false);

    /*
     * Setup proc_states for each process in the pool. proc_states are
     * linked via the list_ptr_ field. The free-list of proc_states is
     * stored in launcher_state_->free_list_.
     */
    pstates = (proc_state *)mmap(NULL, sizeof(proc_state) * nprocs, PROT_FLAGS, MAP_FLAGS, 0, 0);
    for (i = 0; i < nprocs; ++i)
    {
        pstates[i].request_        = (Request *)&req_bufs[i * RQST_BUF_SZ];
        pstates[i].proc_done_      = &proc_dones[i];
        pstates[i].proc_mutex_     = &proc_mutexs[i];
        pstates[i].proc_cond_      = &proc_condis[i];
        pstates[i].launcher_state_ = launcher_state_;
        pstates[i].txns_executed_  = txns_executed_;
        pstates[i].list_ptr_       = &pstates[i + 1];
    }
    pstates[i - 1].list_ptr_    = NULL;
    launcher_state_->free_list_ = pstates;

    /*
     * YOUR CODE HERE
     *
     * Launch the processes in the process pool.
     * Also, remember to update pool_sz_
     *
     * When your code is ready, remove the assert(false) statement
     * below.
     */
    assert(false);
}

ProcessPoolLauncher::~ProcessPoolLauncher()
{
    int err;
    err = munmap((void *)launcher_state_->nprocs_idle_, sizeof(uint32_t));
    assert(err == 0);

    pthread_mutex_destroy(launcher_state_->nprocs_idle_mutex_);
    err = munmap((void *)launcher_state_->nprocs_idle_mutex_, sizeof(pthread_mutex_t));
    assert(err == 0);

    pthread_cond_destroy(launcher_state_->nprocs_idle_cond_);
    err = munmap((void *)launcher_state_->nprocs_idle_cond_, sizeof(pthread_cond_t));
    assert(err == 0);

    pthread_mutex_destroy(launcher_state_->free_list_mutex_);
    err = munmap((void *)launcher_state_->free_list_mutex_, sizeof(pthread_mutex_t));
    assert(err == 0);

    while (free_list_ != NULL)
    {
        err = munmap((void *)launcher_state_->free_list_[0].proc_done_, sizeof(bool));
        assert(err == 0);

        pthread_mutex_destroy(launcher_state_->free_list_[0].proc_mutex_);
        err = munmap((void *)launcher_state_->free_list_[0].proc_mutex_, sizeof(pthread_mutex_t));
        assert(err == 0);

        pthread_cond_destroy(launcher_state_->free_list_[0].proc_cond_);
        err = munmap((void *)launcher_state_->free_list_[0].proc_cond_, sizeof(pthread_cond_t));
        assert(err == 0);

        err = munmap((void *)launcher_state_->free_list_[0].request_, RQST_BUF_SZ);
        assert(err == 0);
        
        err = munmap((void *)launcher_state_->free_list_[0], sizeof(proc_state));
        assert(err == 0);
        free_list_ = free_list_->list_ptr_;
    }

    err = munmap((void *)launcher_state_, sizeof(proc_mgr));
    assert(err == 0);
}

void ProcessPoolLauncher::ExecutorFunc(proc_state *st)
{
    while (true)
    {
        /*
         * YOUR CODE HERE
         *
         * Wait for a new request, and execute it. After executing the
         * request return proc_state to the launcher's proc_state
         * free-list. Remember to signal/wait the appropriate proc_done_,
         * and nprocs_idle_.
         *
         * When your code is ready, remove the assert(false) statement
         * below.
         */
        assert(false);

        st->request_->Execute();
        fetch_and_increment(st->txns_executed_);

        /*
         * YOUR CODE HERE
         */
    }
    exit(0);
}

void ProcessPoolLauncher::ExecuteRequest(Request *req)
{
    proc_state *st;
    Launcher::ExecuteRequest(req);

    st = NULL;
    /*
     * YOUR CODE HERE
     *
     * Find an idle process from free_list_, copy the request into the
     * process' request buffer, and execute the request on the idle process.
     *
     * Hint: Use the process' proc_done_, and the launcher's nprocs_idle_ to
     * initiate a new request on the idle process, and ensure that there
     * exist idle processes.
     *
     * When your code is ready, remove the assert(false) statement
     * below.
     */
    assert(false);

    /* Copy request into proc's request buffer */
    assert(Request::CopySize(req) <= RQST_BUF_SZ);
    assert(st != NULL);
    Request::CopyRequest((char *)st->request_, req);

    /*
     * YOUR CODE HERE
     */
}
