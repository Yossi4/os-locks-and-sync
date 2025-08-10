#include "local_storage.h"
#include "ticket_lock.h"
#include <stdio.h>  // For printf.
#include <stdlib.h> // For exit.

ticket_lock tls_lock; // Protect access to the global g_tls array.

/*
 * Global TLS array for storing thread-specific data.
 * Each entry holds a thread ID (-1 if unused) and a data pointer.
 * Requires synchronization for safe access.
 */
tls_data_t g_tls[MAX_THREADS];

/**
 * Initializes the global TLS array.
 * Sets each entry's thread_id to -1 (unused) and data to NULL.
 * Also initializes the ticket lock for synchronization.
 */
void init_storage(void) {
    ticketlock_init(&tls_lock); // Initialize the lock protecting g_tls.
    for (int i = 0; i < MAX_THREADS; i++) {
        g_tls[i].thread_id = -1; // Mark slot as unused.
        g_tls[i].data = NULL; // Clears the data pointer.
    }
}

/**
 * Allocates a TLS (Thread-Local Storage) entry for the calling thread.
 * Ensures that each thread gets a unique slot in the global TLS array (g_tls).
 * Uses a ticket lock to synchronize access to the shared g_tls array.
 * If the thread already has an allocated slot, the function returns immediately.
 */
void tls_thread_alloc(void) {
    int64_t tid = (int64_t)pthread_self(); // Get's the calling thread's ID.
    ticketlock_acquire(&tls_lock); // Acquiring the global lock.
    // Check's if the thread already has an allocated slot in the array.
    for (int i = 0; i < MAX_THREADS; i++) {
        if (g_tls[i].thread_id == tid) {
            ticketlock_release(&tls_lock); // if found -> release lock.
            return;
        }
    } 
    // If we reached here we still need to find a slot.
    for (int i = 0; i < MAX_THREADS; i++) {
        if (g_tls[i].thread_id == -1) { // Means we got place.
            g_tls[i].thread_id = tid; // Assign the entry thread_id to the current ID.
            ticketlock_release(&tls_lock); // if we assign successfully -> release.
            return;
        }
    }
    // If we reached here there is no free space.
    printf("thread [%ld] failed to initialize, not enough space\n", tid);
    exit(1);
}

/**
 * Retrieves the TLS data pointer for the calling thread.
 * Searches the global TLS array (g_tls) for the entry corresponding to the calling thread.
 * If found, returns the associated data pointer.
 * If the thread has not been initialized in the TLS, prints an error message and exits with code 2.
 * Uses the ticket lock to ensure synchronized access to g_tls.
 */
void* get_tls_data(void) {
    int64_t tid = (int64_t)pthread_self(); // Get's the calling thread's ID.
    ticketlock_acquire(&tls_lock); // Acquiring the global lock.
    // Search for the calling thread entry.
    for (int i = 0; i < MAX_THREADS; i++) {
        if (g_tls[i].thread_id == tid) {
            void* data = g_tls[i].data; // Retrieve data.
            ticketlock_release(&tls_lock);
            return data; 
        }
    }
    // If we reached here no corresponding entry has been found.
    printf("thread [%ld] hasn’t been initialized in the TLS\n", tid);
    ticketlock_release(&tls_lock);
    exit(2);
}

/**
 * Sets the TLS data pointer for the calling thread.
 * Searches the global TLS array (g_tls) for the entry corresponding to the calling thread.
 * If found, updates the data pointer.
 * If the thread has not been initialized in the TLS, prints an error message and exits with code 2.
 * Uses the ticket lock to ensure synchronized access to g_tls.
 * @param data Pointer to the data to set for the calling thread's TLS entry.
 */
void set_tls_data(void* data) {
    int64_t tid = (int64_t)pthread_self(); // Get's the calling thread's ID.
    ticketlock_acquire(&tls_lock); // Acquiring the global lock.
    // Search for the calling thread entry.
    for (int i = 0; i < MAX_THREADS; i++) {
        if (g_tls[i].thread_id == tid) {
            g_tls[i].data = data; // Set's the data.
            ticketlock_release(&tls_lock);
            return; 
        }
    }
    // If we reached here no corresponding entry has been found.
    printf("thread [%ld] hasn’t been initialized in the TLS\n", tid);
    ticketlock_release(&tls_lock);
    exit(2);
}

/**
 * Frees the TLS entry for the calling thread.
 * Searches the global TLS array (g_tls) for the entry corresponding to the calling thread.
 * If found, resets the thread ID to -1 and the data pointer to NULL.
 * If the thread has not been initialized in the TLS, prints an error message and exits with code 2.
 * Uses the ticket lock to ensure synchronized access to g_tls.
 */
void tls_thread_free(void) {
    int64_t tid = (int64_t)pthread_self(); // Get's the calling thread's ID.
    ticketlock_acquire(&tls_lock); // Acquiring the global lock.
    // Search for the calling thread entry.
    for (int i = 0; i < MAX_THREADS; i++) {
        if (g_tls[i].thread_id == tid) {
            g_tls[i].thread_id = -1;
            g_tls[i].data = NULL;
            ticketlock_release(&tls_lock);
            return; 
        }
    }
    // If we reached here no corresponding entry has been found.
    printf("thread [%ld] hasn’t been initialized in the TLS\n", tid);
    ticketlock_release(&tls_lock);
    exit(2);
}
