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

