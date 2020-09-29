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



