#ifndef COND_VAR_H
#define COND_VAR_H

#include <stdatomic.h>
#include "ticket_lock.h"

/*
 * Define the condition variable type.
 * Write your struct details in this file.
 */
typedef struct {
    ticket_lock lock; // Ticket lock for protecting the condition variable.
    atomic_int waiting; // Counter tracking the waiting threads.
} condition_variable ;

/*
 * Initializes the condition variable pointed to by 'cv'.
 */
void condition_variable_init(condition_variable* cv);

/*
 * Causes the calling thread to wait on the condition variable 'cv'.
 * The thread should release the external lock 'ext_lock' while waiting and reacquire it before returning.
 */
void condition_variable_wait(condition_variable* cv, ticket_lock* ext_lock);

/*
 * Wakes up one thread waiting on the condition variable 'cv'.
 */
void condition_variable_signal(condition_variable* cv);

/*
 * Wakes up all threads waiting on the condition variable 'cv'.
 */
void condition_variable_broadcast(condition_variable* cv);

#endif // COND_VAR_H
