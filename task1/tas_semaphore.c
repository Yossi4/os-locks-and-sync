#include "tas_semaphore.h"

/*
 * Initialize the semaphore with an initial value and unlock the spinlock.
 */
void semaphore_init(semaphore* sem, int initial_value) {
    sem->value = initial_value; // Setting the initial counter value.
    sem->lock = 0; // Setting the TAS spinlock to unlocked.
}

/*
 * Implement semaphore_wait using the TAS spinlock mechanism.
 */
void semaphore_wait(semaphore* sem) {
    // Step 1: acquire the spinlock with TAS.
    while (atomic_exchange(&sem->lock, 1)) {
        // spin until lock is released (becomes 0)
    }
    // Step 2: Checks if semaphore value is greater then 0.
    while (sem->value <= 0) {
        // Release the spinlock so others can signal.
        atomic_store(&sem->lock, 0);
        // Spins outside CS until the value is positive.
        while (sem->value <= 0) {
            // Busy wait.
        }
        // Re-acquire the spinlock before checking again.
        while (atomic_exchange(&sem->lock, 1)) {
            // Spin until lock is acquired.
        }
    }
    // Step 3: safe to decrement the semaphore value.
    sem->value--;
    // Step 4: release the spinlock.
    atomic_store(&sem->lock, 0);
}

/*
 * Implement semaphore_signal using the TAS spinlock mechanism.
 */
void semaphore_signal(semaphore* sem) {
    // Step 1: acquire the spinlock with TAS.
    while (atomic_exchange(&sem->lock, 1)) {
        // spin until lock is released (becomes 0)
    }
    // Step 2: increment the semaphore value.
    sem->value++;
    // Step 3: release the spinlock.
    atomic_store(&sem->lock, 0);
}
