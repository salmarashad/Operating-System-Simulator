#define MAX_SIZE 500
#include <stdio.h>
#include <stdlib.h>
#include "Queue.h"
#include "QueueData.h"
Queue *createQueue(int Q)
{
    Queue *queue = (Queue *)malloc(sizeof(Queue));
    queue->front = -1;
    queue->rear = -1;
    queue->size = 0;
    queue->OriginalQuantum = Q;
    queue->remainingQuantum = Q;
    return queue;
}
Queue *createQueueNoQuantum()
{
    Queue *queue = (Queue *)malloc(sizeof(Queue));
    queue->front = -1;
    queue->rear = -1;
    queue->size = 0;
    return queue;
}

int isEmpty(Queue *queue)
{
    return queue->size == 0;
}

void enqueue(Queue *queue, QueueData *value)
{
    if (queue->rear == MAX_SIZE - 1)
    {
        printf("Queue is full\n");
        return;
    }
    if (queue->front == -1)
        queue->front = 0;
    queue->rear++;
    queue->items[queue->rear] = value;
    queue->size++;
}
void DisplayQueue(Queue *queue)
{
    printf("[");
    for (int i = 0; i < queue->size; i++)
    {

        printf("{%d,%d}", (queue->items[queue->front])->id, (queue->items[queue->front])->priority);
        enqueue(queue, dequeue(queue));
    }
    printf("]\n");
}
QueueData *dequeue(Queue *queue)
{
    if (isEmpty(queue))
    {
        printf("Queue is empty\n");
        return NULL;
    }
    QueueData *item = queue->items[queue->front];
    queue->front++;
    queue->size--;
    return item;
}
