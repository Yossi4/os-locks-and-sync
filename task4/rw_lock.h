#ifndef RW_LOCK_H
#define RW_LOCK_H

#include <stdatomic.h>
#include "ticket_lock.h"  // Include ticket_lock for the internal lock

/*
 * Define the read-write lock type.
 * Write your struct details in this file..
 */
typedef struct {
    ticket_lock lock; // For synchronizing access to the counters.
    atomic_int readers; // Active readers (can be multiple).
    atomic_int writers; // 0 or 1 for showing if a writer holds the lock.
    atomic_int waiting_writers; // Number of writers waiting - for considering fairness and preventing "writer starvation".
} rwlock;

/*
 * Initializes the read-write lock.
 */
void rwlock_init(rwlock* lock);

/*
 * Acquires the lock for reading.
 */
void rwlock_acquire_read(rwlock* lock);

/*
 * Releases the lock after reading.
 */
void rwlock_release_read(rwlock* lock);

/*
 * Acquires the lock for writing. This operation should ensure exclusive access.
 */
void rwlock_acquire_write(rwlock* lock);

/*
 * Releases the lock after writing.
 */
void rwlock_release_write(rwlock* lock);

#endif // RW_LOCK_H
