#ifndef TAS_SEMAPHORE_H
#define TAS_SEMAPHORE_H

#include <stdatomic.h>

/*
 * Define the semaphore type.
 * Write your struct details in this file.
 */
typedef struct {
    atomic_int value; // Semaphore counter.
    atomic_int lock; // TAS spinlock : 0 for unlocked ,1 for locked (for mutual exclusion).
} semaphore;

/*
 * Initializes the semaphore pointed to by 'sem' with the specified initial value.
 */
void semaphore_init(semaphore* sem, int initial_value);

/*
 * Decrements the semaphore (wait operation).
 */
void semaphore_wait(semaphore* sem);

/*
 * Increments the semaphore (signal operation).
 */
void semaphore_signal(semaphore* sem);

#endif // TAS_SEMAPHORE_H
