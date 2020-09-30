---
description: CMSC624
---

# Assignment 2: Understanding Locking, OCC and MVCC

Before beginning this assignment, please be sure you have a clear understanding of the goals and challenges of concurrency control mechanisms in database systems.

In this assignment you will be implementing **five** concurrency control schemes:

* Two versions of locking schemes, both of which are considerably simpler than standard two-phase locking.
* A version of OCC very similar to the serial-validation version described in the OCC paper you read for class.
* A version of OCC somewhat similar to the parallel-validation version described in the OCC paper.
* A version of MVCC timestamp ordering that is a simplified version of the PostgreSQL scheme we studied in class.


{% hint style="info" %}
Similar to assignment 1, aside from writing code, this assignment involves **running some performance experiments** and **answering some questions** we ask below. We use the same private git repository (a1) for each student, and each student can commit changes and push them to this private repository. Our grading server will run your code regularly, generate the output files, and automatically commit them back to you. This will enable the code from each student to run on an identical server (with the same performance characteristics), so that students do not have to be concerned that any strange performance numbers they may see is the result of their particular choice of hardware. 
{% endhint %}

## Framework

You'll be implementing these concurrency control schemes within a transaction processing framework that implements a simple, main-memory resident key-value store. This is a prototype system designed specially for this assignment.

In the code, you'll see that it contains two subdirectories---`txn` and `utils`. Nearly all of the source files you need to worry about are in the `txn` subdirectory, though you might need to occasionally take a peek at a file or two in the `util` subdirectory.

To build and test the system, you can run

```shell
make test
```

At any time, this will first compile the system; if this succeeds with no errors, it will also run two test scripts: one which performs some basic correctness tests of your lock manager implementation, and a second which profiles performance of the system. This second one takes a number of minutes to run, but you can cancel it at any time by pressing `ctrl-C`. 

{% hint style="info" %}
You can not pass the lock manager test right now until you complete the lock manager implementation.
{% endhint %}

When implementing your solutions, please:

* Comment your header files & code thoroughly in the manner demonstrated in the rest of the framework.
* Organize your code logically.
* Use descriptive variable names.

## Codebase

In this assignment, you will need to make changes to the following files/classes/methods:

| source file | methods |
| :--- | :--- |
| txn/lock_manager.cc | All methods for classes `LockManagerA` (Part 1A) and `LockManagerB` (Part 1B) |
| txn/txn_processor.cc | `TxnProcessor::RunOCCScheduler` method (Part 2), `TxnProcessor::RunOCCParallelScheduler` method (Part 3), and `TxnProcessor::RunMVCCScheduler` method (Part 4) |
| txn/mvcc_storage.cc | `MVCCStorage::Read` method (Part 4), `MVCCStorage::Write` method (Part 4), and `MVCCStorage::CheckWrite` method (Part 4) |

However, to understand what's going on in the framework, you will need to look through most of the files in the `txn/` directory. We suggest looking first at the **TxnProcessor** object (`txn/txn_processor.h`) and in particular the **TxnProcessor::RunSerialScheduler()** and **TxnProcessor::RunLockingScheduler()** methods (`txn/txn_processor.cc`) and examining how it interacts with various objects in the system.


## Part 1A: Simple Locking (exclusive locks only)

{% hint style="success" %}
10 points for Part 1A.
{% endhint %}

Once you've looked through the code and are somewhat familiar with the overall structure and flow, you'll implement a simplified version of two-phase locking. The protocol goes like this:

1. Upon entering the system, each transaction requests an **EXCLUSIVE** lock on **EVERY** item that it will either read or write.
2. If any lock request is denied:
   2a) If the entire transaction involves just a single read or write request, then have the transaction simply wait until the request is granted and then proceed to step (3).
   2b) Otherwise, immediately release all locks that were granted before this denial, and immediately abort and queue the transaction for restart at a later point.
3. Execute the program logic. Note: We only get to this point if we didn't get to step (2b) which aborts the transaction.
4. Release **ALL** locks at **commit/abort time**.

In order to avoid the complexities of creating a thread-safe lock manager in this assignment, our implementation only has a single thread that manages the state of the lock manager. This thread performs all the lock requests on behalf of the transactions and then hands over control to a separate execution thread in step (3) above. Note that for workloads where transactions make heavy use of the lock manager, this single lock manager thread may become a performance bottleneck as it has to request and release locks on behalf of ALL transactions.

To help you get comfortable using the transaction processing framework, most of this algorithm is already implemented in `TxnProcessor::RunLockingScheduler()`. Locks are requested and released at all the right times, and all necessary data structures for an efficient lock manager are already in place. All you need to do is implement the **WriteLock**, **Release**, and **Status** methods in the class `LockManagerA`. Make sure you look at the file `lock_manager.h` which explains the data structures that you will be using to queue up requests for locks in the lock manager.

The test file `txn/lock_manager_test.cc` provides some rudimentary correctness tests for your lock manager implementations, but additional tests may be added when we grade the assignment. We therefore suggest that you augment the tests with any additional cases you can think of that the existing tests do not cover.
