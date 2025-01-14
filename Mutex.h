#ifndef MUTEX_H
#define MUTEX_H
#include "Queue.h"
#include "QueueData.h"
typedef struct Mutex
{
    bool Available;
    Queue *BlockingQueue;
    QueueData *owner;
} Mutex;
Mutex *createMutex();
void semWait(Mutex *m, QueueData *process);
bool semSignal(Mutex *m, int pID);

#endif