
/*
 *      Adapted from: https://www3.cs.stonybrook.edu/~skiena/392/programs/
 */

#include <stdbool.h>

#define QUEUESIZE 18

struct Queue {
        int q[QUEUESIZE+2];		        /* body of queue */
        int first;                      /* position of first element */
        int last;                       /* position of last element */
        int count;                      /* number of queue elements */
};

void init_queue(struct Queue *q);
void enqueue(struct Queue *q, int x);
int dequeue(struct Queue *q);
bool empty(struct Queue *q);
void print_queue(struct Queue *q);
int peek(struct Queue *q);
char* get_queue_string(struct Queue *q);