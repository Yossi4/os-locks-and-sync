#include "tl_semaphore.h"
#include <sched.h>  // Required for sched_yield.

/*
 * Initializes the semaphore pointed to by 'sem' with the specified initial value.
 * Initializes the ticket lock values.
 */
void semaphore_init(semaphore* sem, int initial_value) {
    atomic_init(&sem->value, initial_value); // Initialize the counter.
    atomic_init(&sem->ticket, 0); // First ticket to give is 0.
    atomic_init(&sem->cur_ticket, 0); // First ticket being served is 0.
}

/*
 * Decrements the semaphore (wait operation) using the Ticket Lock mechanism.
 * The thread must wait until its ticket is the current one being served.
 */
void semaphore_wait(semaphore* sem) {
   // Get my ticket.
   int my_ticket = atomic_fetch_add(&sem->ticket, 1); // Incrementing atomically the ticket number and get my ticket number.
   // Waiting for my turn.
   while (atomic_load(&sem->cur_ticket) != my_ticket) {
    sched_yield(); // Yield CPU - allowing others to execute.
   } 
   // Decrement semaphore value.
   atomic_fetch_sub(&sem->value, 1);
}

/*
 * Decrements the semaphore (wait operation) using the Ticket Lock mechanism.
 * The thread must wait until its ticket is the current one being served.
 */
void semaphore_signal(semaphore* sem) {
    // Releasing resource by incrementing the semaphore value.
    atomic_fetch_add(&sem->value, 1);
    // Incrementing current ticket for allowing the next thread to process.
    atomic_fetch_add(&sem->cur_ticket, 1);
}
