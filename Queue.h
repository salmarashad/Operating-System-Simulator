#ifndef QUEUE_H
#define QUEUE_H
#include "QueueData.h"
#define MAX_SIZE 500

typedef struct Queue
{
    QueueData *items[MAX_SIZE];
    int front;
    int rear;
    int size;
    int OriginalQuantum;
    int remainingQuantum;
} Queue;

Queue *createQueue(int Q);
Queue *createQueueNoQuantum();
int isEmpty(Queue *queue);
void enqueue(Queue *queue, QueueData *value);
QueueData *dequeue(Queue *queue);
void DisplayQueue(Queue *queue);

#endif /* QUEUE_H */