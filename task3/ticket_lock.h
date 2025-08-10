#ifndef TICKET_LOCK_H
#define TICKET_LOCK_H

#include <stdatomic.h>

// -----------------------------------------------------
// Ticket Lock Header (task2)
// Used by task2 (ticket semaphore) and task3 (cond var)
// -----------------------------------------------------

typedef struct {
    atomic_int ticket;
    atomic_int cur_ticket;
} ticket_lock;

void ticketlock_init(ticket_lock* lock);
void ticketlock_acquire(ticket_lock* lock);
void ticketlock_release(ticket_lock* lock);

#endif
