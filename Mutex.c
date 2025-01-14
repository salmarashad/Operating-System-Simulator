#include "Queue.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "Mutex.h"
Mutex *createMutex()
{
    Mutex *m = (Mutex *)malloc(sizeof(Mutex));
    m->Available = 1; // mutex nafso.  / -> pointer to a mutex
    m->owner = NULL;
    m->BlockingQueue = createQueueNoQuantum();
    return m;
}
void semWait(Mutex *m, QueueData *process)
{
    if (m->Available == 1)
    {
        m->owner = process;
        m->Available = 0;
    }
    else
    {
        enqueue(m->BlockingQueue, process);
    }
}
bool semSignal(Mutex *m, int pID)
{
    if (m->owner->id == pID)
    {
        if (isEmpty(m->BlockingQueue))
        {
            m->Available = 1;
            m->owner = NULL;
        }
        else
        {
            int size = m->BlockingQueue->size;
            QueueData *temp = dequeue(m->BlockingQueue);
            for (int i = 0; i < size - 1; i++)
            {
                QueueData *curr = dequeue(m->BlockingQueue);
                if ((temp->priority) > (curr->priority))
                {
                    QueueData *temp2 = temp;
                    enqueue(m->BlockingQueue, temp2);
                    temp = curr;
                }
                else
                {
                    enqueue(m->BlockingQueue, curr);
                }
            }
            m->owner = temp;

            // dequeue
            /* remove a process P from m.queue and place it on ready list*/
            /* update ownerID to be equal to Process P’s ID */
        }
        return 1;
    }
    else
    {
        return 0;
    }
}