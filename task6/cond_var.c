#include "cond_var.h"
#include <sched.h> // For sched_yield().
#include "ticket_lock.h"

/**
 * Initializes the condition variable.
 * Sets up the internal ticket lock and initializes the waiting counter.
 * @param cv Pointer to the condition variable to initialize.
 */
void condition_variable_init(condition_variable* cv) {
    ticketlock_init(&cv->lock); // Initialize the internal ticket.
    atomic_init(&cv->waiting, 0); // Initialize waiting counter to 0.
}

/**
 * Causes the calling thread to wait on the condition variable.
 * The thread releases the external lock while waiting and reacquires it before returning.
 * @param cv Pointer to the condition variable.
 * @param ext_lock Pointer to the external lock that protects shared data.
 */
void condition_variable_wait(condition_variable* cv, ticket_lock* ext_lock) {
    ticketlock_acquire(&cv->lock); // Locks the condition variable internal lock.
    atomic_fetch_add(&cv->waiting, 1); // Increase waiting threads number.
    ticketlock_release(&cv->lock);
    ticketlock_release(ext_lock); // Release external lock.
    sched_yield(); // Simulate waiting.
    ticketlock_acquire(ext_lock); // After being signaled, reacquire before proceeding.
}

/**
 * Wakes up one thread waiting on the condition variable, if any.
 * Decrements the waiting counter and simulates waking one thread.
 * @param cv Pointer to the condition variable.
 */
void condition_variable_signal(condition_variable* cv) {
    ticketlock_acquire(&cv->lock);
    // Checks for waiting threads.
    if (atomic_load(&cv->waiting) > 0) {
        // Decreasing for "waking" one thread.
        atomic_fetch_sub(&cv->waiting, 1);
    }
    // Release condition variable internal lock.
    ticketlock_release(&cv->lock);
}

/**
 * Wakes up all threads waiting on the condition variable.
 * Resets the waiting counter to 0, simulating waking all threads.
 * @param cv Pointer to the condition variable.
 */
void condition_variable_broadcast(condition_variable* cv) {
    ticketlock_acquire(&cv->lock);
    atomic_store(&cv->waiting, 0); // Reset waiting counter to 0 means all threads woke up.
    ticketlock_release(&cv->lock);
}
