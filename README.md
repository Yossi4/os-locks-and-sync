# Low-Level Synchronization and Concurrency Primitives in C

This repository contains implementations of core synchronization mechanisms and concurrency patterns in C, developed as part of an Operating Systems 
assignment. The code covers six tasks, each focused on a different primitive or pattern, designed without using pthreads or dynamic memory (except where 
explicitly allowed).

---

## Project Structure

- `task1/` — Semaphore implemented using Test-And-Set (TAS) spinlock  
- `task2/` — Semaphore implemented using Ticket Lock mechanism  
- `task3/` — Condition Variable implementation  
- `task4/` — Read-Write Lock with reader/writer fairness considerations  
- `task5/` — Thread-Local Storage (TLS) with static allocation, no compiler-specific keywords  
- `task6/` — Producer-Consumer pattern checking numbers divisible by 6, including synchronized output  

Each task directory contains the `.c` and `.h` files for the implementation, excluding test mains.

---

## Key Features & Restrictions

- Implementations use C23 standard, compiled with gcc13 on Ubuntu 24.04 LTS.  
- Synchronization primitives are built using atomic operations and custom spinlocks without pthread synchronization primitives.  
- No dynamic memory allocation except in the producer-consumer task, where it is carefully managed.  
- Thread safety ensured via custom synchronization mechanisms taught in class.  
- Clean, well-commented code structured for clarity and maintainability.  
- Unit tests (not included) were used during development to validate correctness.

---

## Compilation & Usage

- Compile each task separately with gcc using C23 standard, e.g.:  
  ```bash
  gcc -std=c23 -o task1/task1 task1/tas_semaphore.c
```
