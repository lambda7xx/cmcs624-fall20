#ifndef PROCESS_LAUNCHER_H_
#define PROCESS_LAUNCHER_H_

#include <launcher.h>
#include <semaphore.h>

/*
 * ProcessLauncher implements the process-per-request execution model.
 */
class ProcessLauncher : public Launcher
{
   private:
    /* indicates the max # outstanding requests */
    volatile int* max_outstanding_;

    /*
     * mutex lock and conditional variable guarantee that max outstanding requests
     * never exceeds max_outstanding_.
     */
    pthread_mutex_t* done_mutex_;
    pthread_cond_t* done_cond_;

   public:
    ProcessLauncher(int max_outstanding);

    ~ProcessLauncher();

    /* Run a single request */
    void ExecuteRequest(Request* req);
};

#endif  // PROCESS_LAUNCHER_H_
