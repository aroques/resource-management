/*
 *      Adapted from: https://www3.cs.stonybrook.edu/~skiena/392/programs/
 */

#include "queue.h"
#include <stdio.h>

void init_queue(struct Queue *q) {
    q->first = 0;
    q->last = QUEUESIZE-1;
    q->count = 0;
}

void enqueue(struct Queue *q, blocked_id x) {
    if (q->count >= QUEUESIZE)
    printf("Warning: queue overflow enqueue x=%d\n",x.pid);
    else {
            q->last = (q->last+1) % QUEUESIZE;
            q->q[ q->last ] = x;    
            q->count = q->count + 1;
    }
}

blocked_id dequeue(struct Queue *q) {
    blocked_id x;

    if (q->count <= 0) printf("Warning: empty queue dequeue.\n");
    else {
        x = q->q[ q->first ];
        q->first = (q->first+1) % QUEUESIZE;
        q->count = q->count - 1;
    }

    return(x);
}

bool empty(struct Queue *q) {
    if (q->count <= 0) return (1);
    else return (0);
}

void print_queue(struct Queue *q) {
    int i;

    i = q->first; 
    
    while (i != q->last) {
        printf("Blocked ID: %d: {PID: %d, RSC: %d}\n", i, q->q[i].pid, q->q[i].resource);
        i = (i+1) % QUEUESIZE;
    }

    printf("Blocked ID: %d: {PID: %d, RSC: %d}\n", i, q->q[i].pid, q->q[i].resource);
}