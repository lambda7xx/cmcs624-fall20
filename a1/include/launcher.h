#ifndef LAUNCHER_H_
#define LAUNCHER_H_

#include <request.h>

#include <stdint.h>

/* Size of a single cache line. 64 bytes */
#define CACHE_LINE_SZ 64

class Launcher
{
   protected:
    volatile uint64_t *txns_executed_; /* # txns completed */
    uint64_t _num_requests;            /* # txns issued */

    /* Atomically increment the 64-bit int done_ptr points to */
    static void IncrDoneTxns(volatile uint64_t *done_ptr);

   public:
    Launcher();
    ~Launcher();

    /* Returns the latest value of *txns_executed_ */
    uint64_t ReadTxnsExecuted();

    /* Wait for outstanding requests to finish executing */
    void WaitOutstanding();

    /* Execute a single request */
    virtual void ExecuteRequest(Request *req);
};

#endif  // LAUNCHER_H_
