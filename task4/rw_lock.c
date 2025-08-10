#include "rw_lock.h"
#include <sched.h> // For sched_yield()

/** 
 * Initializes the read-write lock structure.
 * Sets the readers count, writer flag, and spinlock to initial values.
 * @param lock Pointer to the rwlock structure to initialize.
 */
void rwlock_init(rwlock* lock) {
    ticketlock_init(&lock->lock); // Initialize internal ticket lock.
    atomic_init(&lock->readers, 0); // No active readers.
    atomic_init(&lock->writers, 0); // No active writer.
    atomic_init(&lock->waiting_writers, 0); // No waiting writers initially.
}

/**
 * Acquires the lock for reading.
 * Allows multiple readers to enter concurrently as long as no writer holds the lock.
 * Prevents reader preference by blocking new readers if writers are waiting,
 * ensuring fairness and preventing writer starvation.
 * Uses double-checking to ensure consistency when incrementing the reader count.
 * @param lock Pointer to the rwlock structure.
 */
void rwlock_acquire_read(rwlock* lock) {
    while (1) {
        ticketlock_acquire(&lock->lock); 
        if (atomic_load(&lock->writers) == 0 && atomic_load(&lock->waiting_writers) == 0) { // Check that no writer is active or waiting.
            atomic_fetch_add(&lock->readers, 1); // Increment reader count.
            ticketlock_release(&lock->lock); // First we acquire, now we release.
            break; // Exiting infinite loop.
        }
        ticketlock_release(&lock->lock); // If writer is active (means we didn't entered the first block).
        sched_yield(); // Use sched_yield() to avoid busy-waiting and allow other threads to run.
    }
}

/**
 * Releases the lock after reading.
 * Decrements the readers count atomically.
 * No need for additional synchronization here since the readers count is managed atomically,
 * and writers wait until all readers finish before acquiring the lock.
 * @param lock Pointer to the rwlock structure.
 */
void rwlock_release_read(rwlock* lock) {
    atomic_fetch_sub(&lock->readers, 1);
}

/**
 * Acquires the lock for writing.
 * Ensures exclusive access by waiting for all readers and other writers to finish.
 * Implements fairness by tracking waiting writers, which blocks new readers until writers finish.
 * Uses double-checking and a spinlock to ensure mutual exclusion.
 * @param lock Pointer to the rwlock structure.
 */
void rwlock_acquire_write(rwlock* lock) {
    atomic_fetch_add(&lock->waiting_writers, 1); // Wants to acquire write -> waiting.
    while (1) {
        ticketlock_acquire(&lock->lock); 
        if (atomic_load(&lock->readers) == 0 && atomic_load(&lock->writers) == 0) {
            atomic_store(&lock->writers, 1); // New writer.
            atomic_fetch_sub(&lock->waiting_writers, 1); // No longer waiting.
            ticketlock_release(&lock->lock);
            break;
        }
    ticketlock_release(&lock->lock);
    sched_yield();
    }
}

/**
 * Releases the lock after writing.
 * Clears the writer flag to allow readers or another writer to proceed.
 * @param lock Pointer to the rwlock structure.
 */
void rwlock_release_write(rwlock* lock) {
    ticketlock_acquire(&lock->lock); 
    atomic_store(&lock->writers, 0); // Clear writer flag.
    ticketlock_release(&lock->lock);
}
