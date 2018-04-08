
/*
 *      Adapted from: https://www3.cs.stonybrook.edu/~skiena/392/programs/
 */

#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void init_queue(struct Queue *q) {
    q->first = 0;
    q->last = QUEUESIZE-1;
    q->count = 0;
}

void enqueue(struct Queue *q, int x) {
    if (q->count >= QUEUESIZE)
    printf("Warning: queue overflow enqueue x=%d\n",x);
    else {
        q->last = (q->last+1) % QUEUESIZE;
        q->q[ q->last ] = x;    
        q->count = q->count + 1;
    }
}

int dequeue(struct Queue *q) {
    int x;

    if (q->count <= 0) printf("Warning: empty queue dequeue.\n");
    else {
        x = q->q[ q->first ];
        q->first = (q->first+1) % QUEUESIZE;
        q->count = q->count - 1;
    }

    return(x);
}

int peek(struct Queue *q) {
    int x;

    if (q->count <= 0) printf("Warning: empty queue peek.\n");
    else {
        x = q->q[ q->first ];
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
        printf("%2d ", q->q[i]);
        i = (i+1) % QUEUESIZE;
    }

    printf("%2d ",q->q[i]);
    printf("\n");
}

char* get_queue_string(struct Queue *q) {
    char* out = malloc(sizeof(char) * 255);
    
    sprintf(out, " ");

    int i;

    i = q->first; 
    
    while (i != q->last) {
        sprintf(out + strlen(out), "%2d ", q->q[i]);
        i = (i+1) % QUEUESIZE;
    }

    sprintf(out + strlen(out), "%2d ",q->q[i]);
    sprintf(out + strlen(out), "\n");
    return out;
}