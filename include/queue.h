/*
 *      Adapted from: https://www3.cs.stonybrook.edu/~skiena/392/programs/
 */

#include <stdbool.h>

#define QUEUESIZE 18

typedef struct blocked_id {
    int pid;
    int resource;
} blocked_id;

struct Queue {
        blocked_id q[QUEUESIZE+1];              /* body of queue */
        int first;                              /* position of first element */
        int last;                               /* position of last element */
        int count;                              /* number of queue elements */
};

void init_queue(struct Queue *q);
void enqueue(struct Queue *q, blocked_id x);
blocked_id dequeue(struct Queue *q);
bool empty(struct Queue *q);
void print_queue(struct Queue *q);