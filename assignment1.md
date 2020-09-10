---
description: CMSC624
---

# Assignment 1

In this assignment, you will get some hands-on experience exploring the performance and architectural differences between database system process models. The main challenge of this assignment will be to understand the existing code that we are providing you. By reading through this code, we hope you will get a sense of some of the basic implementation differences between thread-based and process-based process models.

## Overview

The amount of code you have to write for this assignment is quite small. In short, we are providing implementations of thread-per worker and process-per-worker process models \(see chapter two of the [Architecture of a Database System](https://dsf.berkeley.edu/papers/fntdb07-architecture.pdf) paper that is in the assigned reading for this semester\) over a simple database stored as an array in main memory. Your goal in this assignment is to implement the process-pool and thread-pool variants of these process models. Even for these variants, we have written most of the code for you --- we have just left **the most interesting parts of the code left for you** to implement. However, if you do not understand the thread-per worker and process-per-worker code that we have provided you, you will likely be unable to add the missing code \(even though it is small\) for the thread-pool and process-pool variants.

Aside from writing code, this assignment involves **running some performance experiments** and **answering some questions** we ask below. Please **don't** git clone this repository. Instead, we will create a private git repository for each student, and each student can commit changes and push them to this private repository. Our grading server will run your code regularly, generate the output files, and automatically commit them back to you. This will enable the code from each student to run on an identical server (with the same performance characteristics), so that students do not have to be concerned that any strange performance numbers they may see is the result of their particular choice of hardware. 

## Execution models

The assignment uses Posix Threads to implement the thread-per-request and thread-pool models. This [Linux Posix Threads Tutorial](http://www.yolinux.com/TUTORIALS/LinuxTutorialPosixThreads.html) provides a good reference for programming with pthreads on Linux. The assignment codebase uses pthreads for managing threads' lifecycles \(thread creation and deletion\). Inter-thread synchronization is mediated via pthread **mutex lock** and **signal/wait**.

The process-per-request and process-pool models are implemented using Linux processes. Processes communicate with each other using shared memory segments via **mmap\(\)**. The provided database class implementation has a few examples of uses of **mmap\(\)**.

## Synchronization

This assignment uses mutexes to mediate inter-thread and inter-process communication. Concurrency is usually covered \(at least theoretically\) in the intro to systems programming courses. We recommend you read through Remzi's OS book \([Thread API](http://pages.cs.wisc.edu/~remzi/OSTEP/threads-api.pdf), [Condition Variables](http://pages.cs.wisc.edu/~remzi/OSTEP/threads-cv.pdf)\) for some background on pthread's mutex locks, and condition variables if you have not seen them in the past.

## Codebase

The assignment models database processing requests that manipulate several records. The database consists of an array of `1000-byte-sized` records. The database exports three important functions: **GetRecord**, **LockRecord**, and **UnlockRecord**. 

* GetRecord: returns a reference to a record. 
* LockRecord and UnlockRecord: serve as mutex-locks, giving the calling process or thread exclusive access to a particular record.

Each request updates several records in the database. In order to run a request, a process or thread calls a request's execute function in order to process it. `Launcher` is the base class from which each execution model will derive \(`ProcessPoolLauncher`, `ProcessLauncher`, `ThreadPoolLauncher`, and `ThreadLauncher`\). Each of these classes overrides the launcher's  **ExecuteRequest** function.

**Your task** is to complete the implementations of the `ProcessPoolLauncher`, and `ThreadPoolLauncher` classes. Look for **"YOUR CODE HERE"** annotations in the comments in `src/ProcessPoolLauncher.cc` and `src/ThreadPoolLauncher.cc`. As mentioned above, we have provided implementations of the process-per-request, and thread-per-request execution models in `ProcessLauncher.cc`, and `ThreadLauncher.cc`, respectively. You should use these implementations as references while building your thread-pool, and process-pool implementations.

## Command-line Arguments

The provided makefile will compile your executable into a directory `build/db`. The executable takes parameters `--exp_type`, `--max_outstanding`, and `--pool_size`. Each of these three parameters takes **integers** as arguments.

{% hint style="info" %}
The command-line parsing code is not robust enough to deal with incorrectly specified parameter arguments!
{% endhint %}

### max\_outstanding and pool\_size

The **pool-based** schemes \(thread- and process-pools\) take a `pool_size` parameter as input \(specified in their constructors\). The parameter is used to limit the maximum number of threads and processes available to execute requests. The **non-pool-based** schemes \(thread- and process-per-request\) take a `max_outstanding` parameter as input \(again, specified in their constructors\). This parameter is used to limit the maximum number of in-flight requests at any given time.

The `pool_size` and `max_outstanding` parameters above are specified as **command-line arguments**, and will be used to measure the relative performance of each of the four execution models.

### exp\_type

`--exp_type` specifies the type of execution model.

| value | type |
| :--- | :--- |
| 0 | PROCESS\_POOL |
| 1 | PROCESS\_PER\_REQUEST |
| 2 | THREAD\_POOL |
| 3 | THREAD\_PER\_REQUEST |

* If you wish to test your process- or thread-pool implementations, specify the appropriate argument to `exp_type` and additionally specify a `pool_size` \(as an integer\).
* If you wish to test your process- or thread-per-request implementations, specify the appropriate argument to `exp_type` and additionally specify the number of `max_outstanding` requests.

## Experiments

The executable will run for about a minute, and measure the throughput \(requests per second\) of the specified execution model. **The measured throughput is written to a file automatically** --- `results.txt`.

Finally, you will **need to report** measurements for running requests under varying levels of conflicts between requests. Two requests conflict if they try to access the same record in the database. We have provided a `--contention` option to induce conflicts among transactions. To run an experiment under high contention \(lots of conflicts\), add the `--contention` flag as an argument to the executable. Experiments run under low contention \(few conflicts\) without the `--contention` flag.

If a pair of requests conflict, then one of the requests block while the other makes progress. Thus, under high contention, we would expect requests to block each other frequently. On the other hand, under low contention, the probability of two requests blocking due to conflict is very low.

## Test

We have also provided code to test each of your implementations. In order to run a test, run the appropriately configured execution model with the `--test` option. If provided with the `--test` option, the executable will check for correctness using a serial execution model. Running your code with the `--test` option will not measure throughput.

If you use your own test scripts, make sure to include `killall build/db` in those scripts, otherwise, the processes that were spun off by the assignment will result in hundreds of zombie processes on machines. Alternatively, we have provided test scripts \(`lowcontention.sh` and `highcontention.sh`\) that you can use to generate results.

## Hints

The header files corresponding to the four execution models contain the appropriate structures for communication between the main launcher thread, and processes/threads used to execute requests.

The thread- and process-pools are managed via a `free-list`. Each process or thread has a corresponding `process_state` or `thread_state` struct \(`include/ProcessPoolLauncher.h` and `include/ThreadPoolLauncher.h`\). These structs are linked together into a free-list, which is used to indicate whether or not a process is currently in use.

## Deliverables - Part 1

Compiled and working code for the process-pool, and thread-pool execution models. We will evaluate your code using the `--test` option described above. Each execution model will be tested at pool sizes of 1, 2, 4, 8, 16, 32, 64, 128.

{% hint style="info" %}
Passing these test cases is the only way to earn credit for your code. We will award no points for code that does not pass test cases.
{% endhint %}

{% hint style="info" %}
Please do not write a serial program to bypass the test. We will check your code manually.
{% endhint %}

{% hint style="success" %}
20 points for each execution model. Total 40 points.
{% endhint %}

## Deliverables - Part 2

For each execution model, report throughput while varying the pool size or the maximum outstanding requests parameter. You must report throughput for all four execution models \(process-per-request, thread-per-request, thread-pool, and process-pool\). Measure throughput at the following parameter values \[1, 2, 4, 8, 16, 32, 64, 128\]. As mentioned above, we have provided test scripts \(`lowcontention.sh` and `highcontention.sh`\) which varies all of these parameters for you.

Report these throughput measurements under both high contention and low contention in text files **high-contention.txt** and **low-contention.txt** respectively. Each line of the file should begin with **the name of the execution model**, and **the list of measured throughput values** \(in increasing order of pool size or max outstanding requests\). 

For example:

```text
process_pool 2000 3000 4000 5000 6000 7000 ...
```

Provide a brief explanation \(between three to four paragraphs\) for the throughput trends you observe in a separate file, **responses.txt**. In particular, explain the differences between each process model:

* Why do some models get higher throughput than other models? 
* Which process model is the fastest and why is it the fastest? 
* Which one is slowest and why is it slowest? 
* Why are the ones in the middle slower/faster than the two at the extremes? 
* How and why does throughout change as the maximum number of outstanding requests or pool size change? 
* In addition, explain the differences between high-contention and low-contention experiments.

{% hint style="success" %}
44 points for performance results / analysis.
{% endhint %}

## Deliverables - Part 3

Finally, answer each of the following questions briefly \(at most two paragraphs per question\) in `responses.txt`.

Questions

* The process-pool implementation must copy a request into a process's request buffer. The process-per-request implementation, however, does not need to copy or pass requests between processes. Why?
* The process-pool implementation requires a request to be copied into a process-local buffer before the request can be executed. On the other hand, the thread-pool implementation can simply use a pointer to the appropriate request. Why?

{% hint style="success" %}
8 points for each response. Total 16 points.
{% endhint %}

## How to submit

Please submit all text files discussed above to the Assignment 1 link on ELMS. You do not need to submit any code, since we have access to your repositories.
