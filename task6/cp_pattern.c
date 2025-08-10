#include "cp_pattern.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include "ticket_lock.h" // My ticket lock.
#include "cond_var.h" // My custom condition variable.

#define MAX_NUMBER 1000000

char generated_flags[MAX_NUMBER] = {0};    // Array to track generated numbers (0 = not generated, 1 = generated)
atomic_int generated_count = 0;            // Counter for how many unique numbers were generated
ticket_lock generated_flags_lock;          // Lock to protect access to generated_flags array
// atomic_int consumed_count = 0;  // testing.

// Queue node structure. will use us for communication between producers to consumers.
typedef struct node {
    int value;
    struct node* next;
} node_t;

// For iteration, join and clean up.
pthread_t* producers_threads;
pthread_t* consumers_threads;
long* producer_ids;
long* consumer_ids;

// Queue and sync.
node_t* queue_head = NULL;
node_t* queue_tail = NULL;

ticket_lock queue_lock; // Protects access to the queue so multiple producers/consumers don’t corrupt it.
condition_variable queue_cond; // Custom condition variable from task 3.
ticket_lock print_lock; // Protects print_msg.

int producers_done = 0; // Signals consumers when all producers have finished generating numbers, so consumers can 'shut down'.
atomic_int producers_finished = 0;  // Counts finished producers.
int total_producers = 0;             // Total number of producers.

/**
 * Enqueues a value into the shared queue.
 * Locks the queue for safe access and signals consumers that work is available.
 * @param value The number to enqueue.
 */
void enqueue(int value) {
    // Allocate a new node.
    node_t* new_node = malloc(sizeof(node_t));
    new_node->value = value;
    new_node->next = NULL;

    ticketlock_acquire(&queue_lock); // Lock the queue.

    if (queue_tail == NULL) { 
        // Queue is empty.
        queue_head = queue_tail = new_node;
    } else {
        // Add to tail.
        queue_tail->next = new_node;
        queue_tail = new_node;
    }

    condition_variable_broadcast(&queue_cond);  // Wake up *all* consumers.
    ticketlock_release(&queue_lock); // Unlock the queue.
}

/**
 * Dequeues (removes) a number from the front of the queue.
 * Ensures thread-safe access using a mutex lock.
 * @return The dequeued number, or -1 if the queue is empty.
 */
int dequeue() {
    // Check if the queue is empty.
    if (queue_head == NULL) {
        return -1; // Indicate queue is empty.
    }

    // Retrieve value from the head node.
    int value = queue_head->value;
    node_t* old_head = queue_head; // Save the current head node.
    queue_head = queue_head->next; // Move head to the next node.

    // If queue becomes empty, also reset the tail pointer.
    if (queue_head == NULL) {
        queue_tail = NULL;
    }

    free(old_head); // Free the old head node memory.
    return value; // Return the dequeued value.
}

/**
 * Producer thread function.
 * Generates numbers, enqueues them, and prints messages.
 * @param arg The producer's thread ID (passed as a long cast to void*).
 */
void* producer_thread(void* arg) {
    long id = *(long*)arg;
    while (true) {
        int number = rand() % MAX_NUMBER;  // Generate random number.
        // Check if number is already generated.
        ticketlock_acquire(&generated_flags_lock);
        if (generated_flags[number] == 1) {
            // Check global exit condition.
            if (atomic_load(&generated_count) >= MAX_NUMBER) {
                ticketlock_release(&generated_flags_lock);
                break;  // Exit producer.
            }
            ticketlock_release(&generated_flags_lock);
            continue; // Already generated, pick another.
        }
        // Mark number as generated.
        generated_flags[number] = 1;
        int count = atomic_fetch_add(&generated_count, 1) + 1;
        ticketlock_release(&generated_flags_lock);
        enqueue(number); // Adding to queue.
        char msg[100];
        snprintf(msg, sizeof(msg), "Producer %ld generated number: %d", id, number); // Ensures atomic message formatting.
        print_msg(msg);
        // Stop condition: when we've generated all numbers.
        if (count >= MAX_NUMBER) {
        break; // Exit this producer, but do NOT touch producers_done.
        }
    }
    // After exiting the loop, increment producers_finished.
    if (atomic_fetch_add(&producers_finished, 1) + 1 == total_producers) {
        // Last producer sets producers_done and wakes up consumers.
        ticketlock_acquire(&queue_lock);
        producers_done = 1;
        condition_variable_broadcast(&queue_cond);  // Wake up all consumers.
        ticketlock_release(&queue_lock);
    }
    return NULL;
}

/**
 * Consumer thread function.
 * Continuously dequeues numbers from the shared queue.
 * For each number, checks if it's divisible by 6 and prints the result.
 * Terminates when producers are done and the queue is empty.
 * 
 * @param arg Unused (required for pthread compatibility).
 * @return NULL.
 */
void* consumer_thread(void* arg) {
    //print_msg("debug consumers enter");
    long id = *(long*)arg;
    while (true) {
        ticketlock_acquire(&queue_lock); // ensuring that only one thread (either a producer or a consumer) can access the queue.
        // Wait while queue is empty
        while (queue_head == NULL) {
            if (producers_done) {
                ticketlock_release(&queue_lock);
                return NULL;
            }
            condition_variable_wait(&queue_cond, &queue_lock);
        }
        int value = dequeue(); 
        ticketlock_release(&queue_lock); // Release after dequeue.
        // Checking the needed consumer condition.
        int is_divisible = (value % 6 == 0);
        char msg[100];
        snprintf(msg, sizeof(msg), "Consumer %ld checked %d. Is it divisible by 6? %s", id, value, is_divisible ? "True" : "False");
        print_msg(msg);
        // atomic_fetch_add(&consumed_count, 1);  // Increment after consuming - testing.
    }
        return NULL;
}

/**
 * Starts the consumer and producer threads.
 * - Prints the configuration (number of consumers, producers, seed).
 * - Seeds the random number generator with the given seed.
 * - Creates the specified number of consumer and producer threads.
 * @param consumers Number of consumer threads to create.
 * @param producers Number of producer threads to create.
 * @param seed Seed value for random number generation.
 */
void start_consumers_producers(int consumers, int producers, int seed) {
    // Configuration printing. 
    printf("Number of Consumers: %d\n", consumers);
    printf("Number of Producers: %d\n", producers);
    printf("Seed: %d\n", seed);
    // Seeds the random number generator.
    srand(seed);
    total_producers = producers;  // Save total producers globally.

    // Initialzie custom locks and condition variable.
    ticketlock_init(&queue_lock);
    ticketlock_init(&print_lock);
    ticketlock_init(&generated_flags_lock);  // Initialize the generated flags lock.
    condition_variable_init(&queue_cond);

    producers_threads = malloc(sizeof(pthread_t) * producers);
    consumers_threads = malloc(sizeof(pthread_t) * consumers);
    producer_ids = malloc(sizeof(long) * producers);   // Allocate IDs
    consumer_ids = malloc(sizeof(long) * consumers);   // Allocate IDs

    for (long i = 0; i < producers; i++) {
        producer_ids[i] = i;   // Set ID.
        pthread_create(&producers_threads[i], NULL, producer_thread, &producer_ids[i]);
    }

    for (long i = 0; i < consumers; i++) {
        consumer_ids[i] = i;   // Set ID.
        pthread_create(&consumers_threads[i], NULL, consumer_thread, &consumer_ids[i]);
    }
}

/**
 * Signals all consumers to stop after producers are done.
 * Sets the global flag 'producers_done' to true.
 * Broadcasts on the condition variable to wake up all waiting consumers.
 */
void stop_consumers() {
    ticketlock_acquire(&queue_lock);
    producers_done = 1; // Setting the flag -> producers are done.
    condition_variable_broadcast(&queue_cond); // wake up all the consumers waiting on the condition variable.
    ticketlock_release(&queue_lock);
}

/**
 * Prints a message to stdout in a thread-safe manner.
 * Ensures that only one thread prints at a time by using a mutex,
 * preventing output from overlapping between threads.
 * @param msg The formatted message string to print.
 */
void print_msg(const char* msg) {
    ticketlock_acquire(&print_lock);  // Acquire for synchronized printing.
    printf("%s\n", msg);                // Print the full message in one go.
    ticketlock_release(&print_lock); // Release after printing.
}

/**
 * Waits until all numbers between 0 and MAX_NUMBER have been produced.
 * Continuously checks the atomic counter 'produced_numbers'.
 * This ensures the main thread waits for all producer threads to finish.
 */
void wait_until_producers_produced_all_numbers() {
    // A busy wait loop that continuously checks if all numbers have been produced.
    while (atomic_load(&generated_count) < MAX_NUMBER) {
        sched_yield();
    }
}

/**
 * Waits until the consumer queue becomes empty.
 * Continuously checks the queue head and yields the CPU to other threads.
 * Exits immediately if the queue is already empty.
 */
void wait_consumers_queue_empty() {
    while (true) {
        ticketlock_acquire(&queue_lock);
        if (queue_head == NULL) {  
            ticketlock_release(&queue_lock);
            break;  // Queue is empty and producers are done.
        }
        ticketlock_release(&queue_lock);
        sched_yield();
    }
}

/**
 * Main function:
 * (1) Parses arguments (consumers, producers, seed).
 * (2) Starts producers and consumers.
 * (3) Waits for producers and consumers to finish.
 * (4) Cleans up and exits.
 */
int main(int argc, char* argv[]) {
    // Validates argument count.
    if (argc != 4) {
        printf("usage: cp_pattern [consumers] [producers] [seed]\n");
        exit(1);
    }
    // Parsing the arguments.
    int consumers = atoi(argv[1]);
    int producers = atoi(argv[2]);
    int seed = atoi(argv[3]);

    // Starting method.
    start_consumers_producers(consumers, producers, seed);
    
    // Producers Waiting method.
    wait_until_producers_produced_all_numbers();

    // Empty queue wait.
    wait_consumers_queue_empty();
   
    // Signaling method.
    stop_consumers();
   
    // Join producers.
    for (int i =0; i < producers; i++) {
        pthread_join(producers_threads[i], NULL);
    }
    
    // Join consumers.
    for (int i =0; i < consumers; i++) {
        pthread_join(consumers_threads[i], NULL);
    }
    
    // Handle memory allocation.
    free(producers_threads);
    free(consumers_threads);
    free(producer_ids);
    free(consumer_ids);

    // Testing.
    //printf("Total produced: %d\n", atomic_load(&generated_count));
    //printf("Total consumed: %d\n", atomic_load(&consumed_count));


    // Exit code as needed.
    exit(0);  
}
