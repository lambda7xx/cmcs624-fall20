---
description: CMSC624
---

# Assignment 2: Understanding Locking, OCC and MVCC

Before beginning this assignment, please be sure you have a clear understanding of the goals and challenges of concurrency control mechanisms in database systems.

In this assignment you will be implementing **five** concurrency control schemes:

* Two versions of locking schemes, both of which implement variations of the standard two-phase locking algorithm we discussed in class.
* A version of OCC very similar to the serial-validation version described in the OCC paper you read for class.
* A version of OCC somewhat similar to the parallel-validation version described in the OCC paper.
* A version of MVCC timestamp ordering that is a simplified version of the PostgreSQL scheme we studied in class.

## Pull the Code

Change the current working directory to your existing folder from assignment 1, then pull updates from the official repository.

```shell
git remote add a2 https://github.com/umd-db/cmcs624-fall20
git pull a2 master # you may have to fix merge conflicts
```

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

```shell
1. Upon entering the system, each transaction requests an EXCLUSIVE lock on EVERY item that it will either read or write.
2. Wait until all requests have been granted. 
3. Execute the program logic.
4. Release ALL locks at commit/abort time.
```

it instead proceed to the next lock request

In order to avoid the complexities of creating a thread-safe lock manager in this assignment, our implementation only has a single thread that manages the state of the lock manager. This thread performs all the lock requests on behalf of the transactions and then hands over control to a separate execution thread in step (3) above. Note that for workloads where transactions make heavy use of the lock manager, this single lock manager thread may become a performance bottleneck as it has to request and release locks on behalf of ALL transactions.

To help you get comfortable using the transaction processing framework, most of this algorithm is already implemented in `TxnProcessor::RunLockingScheduler()`. Locks are requested and released at all the right times, and all necessary data structures for an efficient lock manager are already in place. All you need to do is implement the **WriteLock**, **Release**, and **Status** methods in the class `LockManagerA`. Make sure you look at the file `lock_manager.h` which explains the data structures that you will be using to queue up requests for locks in the lock manager.

The test file `txn/lock_manager_test.cc` provides some rudimentary correctness tests for your lock manager implementations, but additional tests may be added when we grade the assignment. We therefore suggest that you augment the tests with any additional cases you can think of that the existing tests do not cover.


## Part 1B: Slightly Less Simple Locking (adding in shared locks)

{% hint style="success" %}
15 points for Part 1B.
{% endhint %}

To increase concurrency, we can allow transactions with overlapping readsets but **disjoint** write sets to execute concurrently. We do this by adding in **SHARED** locks. Again, all data structures already exist, and all you need to implement are the **WriteLock**, **ReadLock**, **Release**, and **Status** methods in the class 'LockManagerB'.

Again, `txn/lock_manager_test.cc` provides some basic correctness tests, but you should go beyond these in checking the correctness of your implementation.

## Part 2: Serial Optimistic Concurrency Control (OCC)

{% hint style="success" %}
10 points for Part 2.
{% endhint %}

For OCC, you will have to implement the `TxnProcessor::RunOCCScheduler` method. This is a simplified version of OCC compared to the one presented in the paper. Pseudocode for the OCC algorithm to implement (in the `RunOCCScheduler` method):

```c++
  while (tp_.Active()) {
    Get the next new transaction request (if one is pending) and pass it to an execution thread.
    Deal with all transactions that have finished running (see below).
  }

  In the execution thread (we are providing you this code):
    Record start time
    Perform "read phase" of transaction:
       Read all relevant data from storage
       Execute the transaction logic (i.e. call Run() on the transaction)

  Dealing with a finished transaction (you must write this code):
    // Validation phase:
    for (each record whose key appears in the txn's read and write sets) {
      if (the record was last updated AFTER this transaction's start time) {
        Validation fails!
      }
    }

    // Commit/restart
    if (validation failed) {
      Cleanup txn
      Completely restart the transaction.
    } else {
      Apply all writes
      Mark transaction as committed
    }

  cleanup txn:
    txn->reads_.clear();
    txn->writes_.clear();
    txn->status_ = INCOMPLETE;

  Restart txn:
    mutex_.Lock();
    txn->unique_id_ = next_unique_id_;
    next_unique_id_++;
    txn_requests_.Push(txn);
    mutex_.Unlock();
```


## Part 3: Optimistic Concurrency Control with Parallel Validation.

{% hint style="success" %}
12 points for Part 3.
{% endhint %}

OCC with parallel validation means that the validation/write steps for OCC are done in parallel across transactions. There are several different ways to do the parallel validation -- here we give a simplified version of the pseudocode from the paper, or you can write your own pseudocode based on the paper's presentation of parallel validation and argue why it's better than the ones presented here (see analysis question 4).

{% hint style="info" %}
The `util/atomic.h` file contains data structures that may be useful for this section.
{% endhint %}

Pseudocode to implement in RunOCCParallelScheduler:

```c++
  while (tp_.Active()) {
    Get the next new transaction request (if one is pending) and pass it to an execution thread that executes the txn logic *and also* does the validation and write phases.
  }

  In the execution thread:
    Record start time
    Perform "read phase" of transaction:
       Read all relevant data from storage
       Execute the transaction logic (i.e. call Run() on the transaction)
    <Start of critical section>
    Make a copy of the active set save it
    Add this transaction to the active set
    <End of critical section>
    Do validation phase:
      for (each record whose key appears in the txn's read and write sets) {
        if (the record was last updated AFTER this transaction's start time) {
          Validation fails!
        }
      }

      for (each txn t in the txn's copy of the active set) {
        if (txn's write set intersects with t's write sets) {
          Validation fails!
        }
        if (txn's read set intersects with t's write sets) {
          Validation fails!
        }
      }

      if valid :
        Apply writes;
        Remove this transaction from the active set
        Mark transaction as committed;
      else if validation failed:
        Remove this transaction from the active set
        Cleanup txn
        Completely restart the transaction.

    cleanup txn:
       txn->reads_.clear();
       txn->writes_.clear();
       txn->status_ = INCOMPLETE;

    Restart txn:
      mutex_.Lock();
      txn->unique_id_ = next_unique_id_;
      next_unique_id_++;
      txn_requests_.Push(txn);
      mutex_.Unlock();
```

## Multiversion Timestamp Ordering Concurrency Control

{% hint style="success" %}
20 points for Part 4.
{% endhint %}

For MVCC, you will have to implement the `TxnProcessor::RunMVCCScheduler` method based on the pseudocode below. The pseudocode implements a simplified version of MVCC relative to the material we presented in class.

Although we give you a version of pseudocode, if you want, you can write your own pseudocode and argue why it's better than the code presented here (see analysis question 6).

In addition you will have to implement the **MVCCStorage::Read**, **MVCCStorage::Write**, **MVCCStorage::CheckWrite**.

Pseudocode for the algorithm to implement (in the `RunMVCCScheduler` method):

```c++
  while (tp_.Active()) {
    Get the next new transaction request (if one is pending) and pass it to an execution thread.
  }

  In the execution thread:

    Read all necessary data for this transaction from storage (Note that unlike the version of MVCC from class, you should lock the key before each read)
    Execute the transaction logic (i.e. call Run() on the transaction)
    Acquire all locks for keys in the write_set_
    Call MVCCStorage::CheckWrite method to check all keys in the write_set_
    If (each key passed the check)
      Apply the writes
      Release all locks for keys in the write_set_
    else if (at least one key failed the check)
      Release all locks for keys in the write_set_
      Cleanup txn
      Completely restart the transaction.

  cleanup txn:
    txn->reads_.clear();
    txn->writes_.clear();
    txn->status_ = INCOMPLETE;

  Restart txn:
    NewTxnRequest(txn);
    // const std::lock_guard<std::mutex> lock(mutex_);
    // txn->unique_id_ = next_unique_id_;
    // next_unique_id_++;
    // txn_requests_.UnSafePush(txn);
```

## Part 5: Analysis

{% hint style="success" %}
3-8 points for each response, total 33 points.
{% endhint %}

After implementing both locking schemes, both OCC schemes, and the MVCC scheme, please respond to the following questions in **analysis.txt** (text only, no formatting please).

### 1. Carpe datum (2 points)

Run `make test` and report the performance numbers given by `txn_processor_test`. 

{% hint style="info" %}
When you finish the code part, we will run your code on our test server, and commit the results back to you.
{% endhint %}

### 2. Simulations are doomed to succeed. (4 points)

Transaction durations are accomplished simply by forcing the thread executing each transaction to run in a busy loop for approximately the amount of time specified. This is supposed to simulate transaction logic --- e.g. the application might run some propietary customer scoring function to estimate the total value of the customer after reading in the customer record from the database. Please list **at least two weaknesses** of this simulation --- i.e. give two reasons why performance of the different concurrency control schemes you experimented with for this assignment would change relative to each other if we ran actual application code instead of just simulating it with a busy loop.

### 3. Locking manager (4 points)

Explain the performance difference between Locking A and Locking B: 

- [ ] When does Locking A perform better than Locking B? Why? 
- [ ] When does Locking B perform better than Locking A? Why? 

Neither of these locking schemes is equivalent to standard two-phase locking. Compare and contrast Locking B with standard two-phase locking (where each lock is acquired immediately before it is needed instead of acquiring all locks at the beginning of a transaction).

- [ ] When would two-phase locking perform better than Locking B?
- [ ] When would Locking B perform better than two-phase locking?

### 4. OCCam's Razor (4 points)

The OCC with serial validation is simpler than OCC with parallel validation.  

- [ ] How did the two algorithms compare with each other in this simulation? Why do you think that is the case?
- [ ] How does this compare to the OCC paper that we read for class?
- [ ] What is the biggest reason for the difference between your results and the what you expected after reading the OCC paper?

If you did not follow the given pseudocode for OCC with parallel validation, give your pseudocode and argue why it is better.

### 5. OCC vs. Locking B  (7 points)

If your code is correct, you probably found that OCC and Locking B were approximately the same performance for the **high contention** read-only (5-records) test. 

- [ ] But OCC beat Locking B for the **high contention** read-only (30-records) test. What is the reason for OCC suddenly being better for 30-record transactions? 
- [ ] Furthermore, OCC beat Locking B for the **high contention** read-write test (both for 5 record transactions and 10 record transactions). Why? 
- [ ] Furthermore, why does the relative difference between OCC and Locking B get larger for 10 record transactions than 5 record transactions in the **high contention** read-write test?

### 6. MVCC vs. OCC/Locking (7 points)

- [ ] For the read-write tests, MVCC performs worse than OCC and Locking. Why? 
- [ ] MVCC even sometimes does worse than serial. Why?
- [ ] Yet for the mixed read-only/read-write experiment it performs the best, even though it wasn't the best for either read-only nor read-write. Why?

If you wrote your own version, please explain why it's better than the ones presented here.

### 7. MVCC pseudocode (4 points)

- [ ] Why did our MVCC pseudocode request read locks before each read?
- [ ] In particular, what would happen if you didn't acquire these read locks?
- [ ] How long do these locks have to be held?



## A few important notes on the implementation

1. You should only put the committed transaction into the `txn_results_` queue. Don't put the aborted transaction into the `txn_results_` queue. Instead, put it into `txn_requests_` queue.

```c++
    mutex_.Lock();
    txn->unique_id_ = next_unique_id_;
    next_unique_id_++;
    txn_requests_.Push(txn);
    mutex_.Unlock();
```

2. Inside your `Execution` method of MVCC:  when you call `read()` method to read values from database, please don't forget to provide the third parameter(txn->unique_id_), otherwise the default value is 0 and you always read the oldest version.

3. For MVCC, the performance would be much better if you organize the versions in **decreasing order**. You can implement this by inserting the new version into the right place inside the `write()` method.


## Submission

Please submit all text files (`analysis.txt`) discussed above to the Assignment 2 link on ELMS. You do not need to submit any code, since we have access to your repositories.

