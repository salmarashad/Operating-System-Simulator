#include <ctype.h>
#include <stdio.h>
#include "Queue.h"
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "Mutex.h"
#define RESET "\033[0m"
#define BOLD "\033[1m"
#define BLUE "\033[34m"
// make sure in stopping condition to check if nothing has not yet arrived as to not stop system
// fix interpreter to return 0 only if blocked? done
// write file
typedef struct
{
    Queue *Level1;
    Queue *Level2;
    Queue *Level3;
    Queue *Level4;
} Scheduler;

typedef struct
{
    char *name;
    char *value;
} MemoryElement;
Scheduler MLFQ;
MemoryElement *Memory[60] = {NULL}; // memory of size 60

int currClockCycle = 0;
int memoryInstructions = 0;
int currExecutingLevel = -1;
int newArrivedprocessID = 1;
Mutex *InputMutex;
Mutex *OutputMutex;
Mutex *FileMutex;
Queue *generalBlockingQueue;
void displayMemory()
{
    for (int i = 0; i < 60; i++)
    {
        if (Memory[i] != NULL)
            printf("memory at index %d is %s : %s \n", i, Memory[i]->name, Memory[i]->value);
    }
}
void ReadProgram(char *filename)
{
    FILE *fptr;
    fptr = fopen(filename, "r");
    char myString[100];
    while (fgets(myString, 100, fptr))
    {

        char *newValue = strdup(myString);
        if (newValue[strlen(newValue) - 1] == '\n')
        {
            newValue[strlen(newValue) - 2] = '\0';
        }
        Memory[memoryInstructions] = (MemoryElement *)malloc(sizeof(MemoryElement));
        Memory[memoryInstructions]->name = "inst";
        Memory[memoryInstructions]->value = strdup(newValue);

        memoryInstructions++;
    }
    Memory[memoryInstructions] = (MemoryElement *)malloc(sizeof(MemoryElement));
    Memory[memoryInstructions]->name = "var1";
    Memory[memoryInstructions]->value = strdup("");
    memoryInstructions++;
    Memory[memoryInstructions] = (MemoryElement *)malloc(sizeof(MemoryElement));
    Memory[memoryInstructions]->name = "var2";
    Memory[memoryInstructions]->value = strdup("");
    memoryInstructions++;
    Memory[memoryInstructions] = (MemoryElement *)malloc(sizeof(MemoryElement));
    Memory[memoryInstructions]->name = "var3";
    Memory[memoryInstructions]->value = strdup("");
    memoryInstructions++;
    // printf("memory instructions inside load program %d", memoryInstructions);
    fclose(fptr);
}
void StartScheduler(Scheduler *Scheduler)
{
    Scheduler->Level1 = createQueue(1);
    Scheduler->Level2 = createQueue(2);
    Scheduler->Level3 = createQueue(4);
    Scheduler->Level4 = createQueue(8);
}
int getPc(int pid)
{
    int memorySize = 60;
    int pcIndex = (memorySize - (3 - (pid - 1)) * 6) + 3;
    MemoryElement *pcStruct = Memory[pcIndex];
    int pc = atoi(pcStruct->value);
    return pc;
}
int getLowerBound(int pid)
{
    int memorySize = 60;
    int lowerBoundIndex = (memorySize - (3 - (pid - 1)) * 6) + 4;
    MemoryElement *lowerBoundStruct = Memory[lowerBoundIndex];
    int lowerBound = atoi(lowerBoundStruct->value);
    return lowerBound;
}
int getUpperBound(int pid)
{
    int memorySize = 60;
    int upperBoundIndex = (memorySize - (3 - (pid - 1)) * 6) + 5;
    MemoryElement *upperBoundStruct = Memory[upperBoundIndex];
    int upperBound = atoi(upperBoundStruct->value);
    return upperBound;
}
int getPriority(int pid)
{
    int memorySize = 60;
    int priorityIndex = (memorySize - (3 - (pid - 1)) * 6) + 2;
    MemoryElement *priorityStruct = Memory[priorityIndex];
    int priority = atoi(priorityStruct->value);
    return priority;
}
char *getState(int pid)
{
    int memorySize = 60;
    int stateIndex = (memorySize - (3 - (pid - 1)) * 6) + 1;
    MemoryElement *priorityStruct = Memory[stateIndex];
    char *state = priorityStruct->value;
    return state;
}
char *getCurrentInstruction(int memoryLocation)
{
    MemoryElement *instStruct = Memory[memoryLocation];
    char *result;

    return strdup(instStruct->value);
}
void setPc(int pid, int newPC)
{
    int memorySize = 60;
    int pcIndex = (memorySize - (3 - (pid - 1)) * 6) + 3;
    MemoryElement *pcStruct = Memory[pcIndex];
    char pc[20];
    sprintf(pc, "%d", newPC);
    pcStruct->value = strdup(pc);
    return;
}
void setPriority(int pid, int newPriority)
{
    int memorySize = 60;
    int priorityIndex = (memorySize - (3 - (pid - 1)) * 6) + 2;
    MemoryElement *priorityStruct = Memory[priorityIndex];
    char priority[20];
    sprintf(priority, "%d", newPriority);
    priorityStruct->value = strdup(priority);
    return;
}
void setState(int pid, char *newState)
{
    int memorySize = 60;
    int stateIndex = (memorySize - (3 - (pid - 1)) * 6) + 1;
    MemoryElement *priorityStruct = Memory[stateIndex];
    priorityStruct->value = strdup(newState);
}
int remainingExecution(int pid)
{
    // pc// 0 upperbound-lowerbound-3
    int pc = getPc(pid);
    int totalInst = getUpperBound(pid) - getLowerBound(pid) - 3;
    return totalInst - (pc) + 1;
}

int findVariable(int pID, char *name)
{
    int varLocation = getUpperBound(pID) - 2;

    if (strcmp(Memory[varLocation]->name, name) == 0)
    {
        return varLocation;
    }

    else if (strcmp(Memory[varLocation++]->name, "var1") == 0)
    {

        Memory[varLocation - 1]->name = strdup(name);
        return varLocation - 1;
    }
    else if (strcmp(Memory[varLocation]->name, name) == 0)
    {
        return varLocation;
    }
    else if (strcmp(Memory[varLocation++]->name, "var2") == 0)
    {
        Memory[varLocation - 1]->name = strdup(name);
        return varLocation - 1;
    }
    else if (strcmp(Memory[varLocation]->name, name) == 0)
    {
        return varLocation;
    }
    else if (strcmp(Memory[varLocation++]->name, "var3") == 0)
    {
        Memory[varLocation - 1]->name = strdup(name);
        return varLocation - 1;
    }
    return -1;
}

void writeFile(char *filename, char *data)
{
    FILE *file = fopen(filename, "w");
    fprintf(file, "%s", data);
    fclose(file);
}

char *readFile(char *filename)
{

    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("The file that you are trying to read doesn't exist \n");
        return NULL;
    }

    // Finding the file size using file pointers
    fseek(file, 0, SEEK_END);
    int filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Memory allocation keeping in mind the null terminator
    char *data = (char *)malloc(filesize + 1);

    // Read the file contents into the allocated memory and adding the null terminator
    fread(data, 1, filesize, file);
    data[filesize] = '\0';

    fclose(file);
    return data;
}
void printFromTo(int a, int b)
{
    for (int i = a; i <= b; i++)
    {
        printf("%d ", i);
    }
    printf("\n");
}
void print(char *value)
{
    printf("%s \n", value);
}

void assign(int pID, char *variable, char *value)
{
    int varIndex = findVariable(pID, variable);
    if (strcmp(value, "input") == 0)
    {
        printf("Please enter a value: ");
        scanf("%s", Memory[varIndex]->value);
        return;
    }
    Memory[varIndex]->value = strdup(value);
    return;
}
void unblock(QueueData *process)
{
    int id = process->id;
    int generalBlockingQueueSize = generalBlockingQueue->size;
    QueueData *temp = (QueueData *)malloc(sizeof(QueueData));
    for (int i = 0; i < generalBlockingQueueSize; i++)
    {
        temp = dequeue(generalBlockingQueue);
        if (temp->id == id)
        {
            break;
        }
        enqueue(generalBlockingQueue, temp);
    }
    int remainingCycles = remainingExecution(id);

    if (remainingCycles > 0)
    {
        int currPriority = getPriority(id);
        setState(id, "Ready");
        QueueData *unblockedProcess = (QueueData *)malloc(sizeof(QueueData));
        unblockedProcess->id = id;
        unblockedProcess->priority = currPriority;
        printf("Im unblocking process with id %d and priority %d \n", id, currPriority);
        switch (currPriority)
        {
        case 1:
            enqueue(MLFQ.Level1, unblockedProcess);
            break;
        case 2:
            enqueue(MLFQ.Level2, unblockedProcess);
            break;
        case 3:
            enqueue(MLFQ.Level3, unblockedProcess);
            break;
        case 4:
            enqueue(MLFQ.Level4, unblockedProcess);
            break;
        default:
            break;
        }
    }
}

void trim_trailing_whitespace(char *str)
{
    int length = strlen(str);
    while (length > 0 && isspace((unsigned char)str[length - 1]))
    {
        str[length - 1] = '\0';
        length--;
    }
}
bool interpreter(int pID, char *instruction)
{
    // printf("Instruction to be executed %s",instruction );
    int MAX_TOKENS = 4;
    int numTokens = 0;
    char *tokens[MAX_TOKENS]; // Array to store pointers to tokens

    char *token = strtok(instruction, " ");
    while (token != NULL && numTokens < MAX_TOKENS)
    {
        trim_trailing_whitespace(token);
        tokens[numTokens] = token; // Store pointer to the token
        numTokens++;
        token = strtok(NULL, " ");
    }

    printf("The command at hand currently is ( %s) coming from process %d \n", Memory[getPc(pID) + getLowerBound(pID)]->value, pID);

    QueueData *currentProcess = (QueueData *)malloc(sizeof(QueueData));
    currentProcess->id = pID;
    currentProcess->priority = getPriority(pID);
    if (numTokens > 0)
    {
        if (strcmp(tokens[0], "semWait") == 0 && numTokens == 2)
        {
            if (strcmp(tokens[1], "userInput") == 0)
            {
                semWait(InputMutex, currentProcess);
                if ((InputMutex->owner->id) == pID) // if i got access to the resource so im not blocked
                {
                    return 1; // successful request
                }
                else
                {
                    enqueue(generalBlockingQueue, currentProcess);
                    return 0;
                }
            }
            else if (strcmp(tokens[1], "userOutput") == 0)
            {
                semWait(OutputMutex, currentProcess);
                if ((OutputMutex->owner->id) == pID)
                {
                    return 1;
                }
                else
                {
                    enqueue(generalBlockingQueue, currentProcess);
                    return 0;
                }
            }
            else if (strcmp(tokens[1], "file") == 0)
            {
                semWait(FileMutex, currentProcess);
                if ((FileMutex->owner->id) == pID)
                {
                    return 1;
                }
                else
                {
                    enqueue(generalBlockingQueue, currentProcess);
                    return 0;
                }
            }
            else
            {
                fprintf(stderr, "Error: %s\n", "Invalid Resource");
                return 1;
            }
        }
        else if (strcmp(tokens[0], "semSignal") == 0 && numTokens == 2)
        {
            if (strcmp(tokens[1], "userInput") == 0)
            {
                semSignal(InputMutex, pID);
                QueueData *unblockedProcess = (QueueData *)malloc(sizeof(QueueData));
                unblockedProcess = InputMutex->owner; // the one who was blocked and took resource now
                if (unblockedProcess != NULL)
                {
                    unblock(unblockedProcess);
                }
            }
            else if (strcmp(tokens[1], "userOutput") == 0)
            {
                semSignal(OutputMutex, pID);
                QueueData *unblockedProcess = (QueueData *)malloc(sizeof(QueueData));
                unblockedProcess = OutputMutex->owner;
                if (unblockedProcess != NULL)
                {
                    unblock(unblockedProcess);
                }
            }
            else if (strcmp(tokens[1], "file") == 0)
            {
                semSignal(FileMutex, pID);
                QueueData *unblockedProcess = (QueueData *)malloc(sizeof(QueueData));
                unblockedProcess = FileMutex->owner;
                if (unblockedProcess != NULL)
                {
                    unblock(unblockedProcess);
                }
            }
            else
            {
                fprintf(stderr, "Error: %s\n", "Invalid Resource");
                return 1;
            }
        }
        else if (strcmp(tokens[0], "printFromTo") == 0 && numTokens == 3)
        {
            int var1 = atoi(Memory[findVariable(pID, tokens[1])]->value);
            int var2 = atoi(Memory[findVariable(pID, tokens[2])]->value);
            printf("Printing a range from %d to %d:", var1, var2);
            printFromTo(var1, var2);
            return 1;
        }
        else if (strcmp(tokens[0], "print") == 0 && numTokens == 2)
        {
            char *value = Memory[findVariable(pID, tokens[1])]->value;
            printf("Printing a value:");
            print(value);

            return 1;
        }
        else if (strcmp(tokens[0], "assign") == 0)
        {
            if (numTokens == 3)
            {
                assign(pID, tokens[1], tokens[2]);
                return 1;
            }
            else if (numTokens == 4)
            {
                if (strcmp(tokens[2], "readFile") == 0)
                {
                    int index = findVariable(pID, tokens[3]);

                    char *data = readFile(Memory[index]->value);
                    if (data == NULL)
                    {
                        // printf("You're trying to read from a file that doesn't exist");
                        return 1;
                    }
                    printf("Data %s read from file %s \n", data, Memory[index]->value);
                    assign(pID, tokens[1], data);
                    return 1;
                }
            }
            else
            {
                fprintf(stderr, "Error: %s\n", "Invalid Instruction");
                return 1;
            }
        }
        else if (strcmp(tokens[0], "writeFile") == 0 && numTokens == 3)
        {

            int varIndexFileName = findVariable(pID, tokens[1]);
            int varIndexFileData = findVariable(pID, tokens[2]);
            writeFile(Memory[varIndexFileName]->value, Memory[varIndexFileData]->value);
            printf("Data %s written to file %s \n", Memory[varIndexFileData]->value, Memory[varIndexFileName]->value);
            return 1;
        }
        else
        {
            fprintf(stderr, "Error: %s\n", "Invalid Command");
            return 1; // cancel review
        }
    }
    return 1;
}

void setCurrExecutingLevel()
{
    if (currExecutingLevel == -1)
    {
        if (!isEmpty(MLFQ.Level1))
        {
            currExecutingLevel = 1;
        }
        else if (!isEmpty(MLFQ.Level2))
        {
            currExecutingLevel = 2;
        }
        else if (!isEmpty(MLFQ.Level3))
        {
            currExecutingLevel = 3;
        }
        else if (!isEmpty(MLFQ.Level4))
        {
            currExecutingLevel = 4;
        }
    }
}
void execute()
{
    // two cases
    // tabee3y lesa feeh quantum w lesa feeh instructions
    // feeh quantum bas mafeesh instructions
    // feeh inst bas mafeesh quantum
    // sem wait fe level 1 ya3ny quantum 5eles el wa7eeda go down
    // sem wait fe 1 w lesa fadel reset quantum
    if (currExecutingLevel == 1)
    {
        QueueData *currProcess = (QueueData *)malloc(sizeof(QueueData));
        currProcess = MLFQ.Level1->items[MLFQ.Level1->front];
        int processID = currProcess->id;
        printf("ProcessID currently %d\n", processID);
        setState(processID, "Running");
        int pc = getPc(processID);
        int lowerBound = getLowerBound(processID);
        char *currentInstruction = getCurrentInstruction(pc + lowerBound);
        bool isNotBlocked = interpreter(processID, currentInstruction);
        setPc(processID, getPc(processID) + 1);
        MLFQ.Level1->remainingQuantum = (MLFQ.Level1->remainingQuantum) - 1;
        int remainingCycles = remainingExecution(processID);
        printf("Process state in the beginning : %s \n", getState(processID));
        if (isNotBlocked)
        {
            if (remainingCycles != 0)
            {
                if (MLFQ.Level1->remainingQuantum == 0)
                {
                    QueueData *q = dequeue(MLFQ.Level1);
                    q->priority = 2;
                    enqueue(MLFQ.Level2, q);                            // enqueue in next level
                    setPriority(processID, getPriority(processID) + 1); // go to next level
                    setState(processID, "Ready");
                    currExecutingLevel = -1;
                    MLFQ.Level1->remainingQuantum = MLFQ.Level1->OriginalQuantum;
                    printf("Process state right now: %s \n", getState(processID));
                }
            }
            else
            {
                dequeue(MLFQ.Level1);
                setState(processID, "Finished");
                currExecutingLevel = -1;
                MLFQ.Level1->remainingQuantum = MLFQ.Level1->OriginalQuantum;
                printf("Process state right now: %s \n", getState(processID));
            }
        }
        else
        {
            // sem wait fe level 1 ya3ny quantum 5eles el wa7eeda go down
            // sem wait fe 1 w lesa fadel reset quantum
            dequeue(MLFQ.Level1);
            setState(processID, "Blocked");
            currExecutingLevel = -1;
            if (MLFQ.Level1->remainingQuantum == 0)
            {
                setPriority(processID, getPriority(processID) + 1);
            }
            MLFQ.Level1->remainingQuantum = MLFQ.Level1->OriginalQuantum;
            printf("Process state right now: %s \n", getState(processID));
        }
    }

    else if (currExecutingLevel == 2)
    {
        QueueData *currProcess = (QueueData *)malloc(sizeof(QueueData));
        currProcess = MLFQ.Level2->items[MLFQ.Level2->front];
        int processID = currProcess->id;
        setState(processID, "Running");
        printf("ProcessID currently %d\n", processID);
        int pc = getPc(processID);
        int lowerBound = getLowerBound(processID);
        char *currentInstruction = getCurrentInstruction(pc + lowerBound);
        bool isNotBlocked = interpreter(processID, currentInstruction);
        setPc(processID, getPc(processID) + 1);
        MLFQ.Level2->remainingQuantum = (MLFQ.Level2->remainingQuantum) - 1;
        int remainingCycles = remainingExecution(processID);
        printf("Process state in the beginning: %s \n", getState(processID));

        if (isNotBlocked)
        {
            if (remainingCycles != 0)
            {
                if (MLFQ.Level2->remainingQuantum == 0)
                {
                    QueueData *q = dequeue(MLFQ.Level2);
                    q->priority = 3;
                    enqueue(MLFQ.Level3, q);
                    setPriority(processID, getPriority(processID) + 1);
                    setState(processID, "Ready");
                    currExecutingLevel = -1;
                    MLFQ.Level2->remainingQuantum = MLFQ.Level2->OriginalQuantum;
                    printf("Process state right now: %s \n", getState(processID));
                }
            }
            else
            {
                dequeue(MLFQ.Level2);
                setState(processID, "Finished");
                currExecutingLevel = -1;
                MLFQ.Level2->remainingQuantum = MLFQ.Level2->OriginalQuantum;
                printf("Process state right now: %s \n", getState(processID));
            }
        }
        else
        {
            dequeue(MLFQ.Level2);
            setState(processID, "Blocked");
            currExecutingLevel = -1;
            if (MLFQ.Level2->remainingQuantum == 0)
            {
                setPriority(processID, getPriority(processID) + 1);
            }
            MLFQ.Level2->remainingQuantum = MLFQ.Level2->OriginalQuantum;
            printf("Process state right now: %s \n", getState(processID));
        }
    }
    else if (currExecutingLevel == 3)
    {
        QueueData *currProcess = (QueueData *)malloc(sizeof(QueueData));
        currProcess = MLFQ.Level3->items[MLFQ.Level3->front];
        int processID = currProcess->id;
        setState(processID, "Running");
        int pc = getPc(processID);
        printf("ProcessID currently %d\n", processID);
        int lowerBound = getLowerBound(processID);
        char *currentInstruction = getCurrentInstruction(pc + lowerBound);
        bool isNotBlocked = interpreter(processID, currentInstruction);
        setPc(processID, getPc(processID) + 1);
        MLFQ.Level3->remainingQuantum = (MLFQ.Level3->remainingQuantum) - 1;
        int remainingCycles = remainingExecution(processID);
        printf("Process %d has remaining %d instructions to execute\n", processID, remainingCycles);
        printf("Process state in the beginning: %s \n", getState(processID));

        if (isNotBlocked)
        {
            if (remainingCycles != 0)
            {
                if (MLFQ.Level3->remainingQuantum == 0)
                {
                    QueueData *q = dequeue(MLFQ.Level3);
                    q->priority = 4;
                    enqueue(MLFQ.Level4, q);
                    setPriority(processID, getPriority(processID) + 1);
                    setState(processID, "Ready");
                    currExecutingLevel = -1;
                    MLFQ.Level3->remainingQuantum = MLFQ.Level3->OriginalQuantum;
                    printf("Process state right now: %s \n", getState(processID));
                }
            }
            else
            {
                dequeue(MLFQ.Level3);
                setState(processID, "Finished");
                currExecutingLevel = -1;
                MLFQ.Level3->remainingQuantum = MLFQ.Level3->OriginalQuantum;
                printf("Process state right now: %s \n", getState(processID));
            }
        }
        else
        {
            dequeue(MLFQ.Level3);
            setState(processID, "Blocked");
            currExecutingLevel = -1;
            if (MLFQ.Level3->remainingQuantum == 0)
            {
                setPriority(processID, getPriority(processID) + 1);
            }
            MLFQ.Level3->remainingQuantum = MLFQ.Level3->OriginalQuantum;
            printf("Process state right now: %s \n", getState(processID));
        }
    }
    else if (currExecutingLevel == 4)
    {
        QueueData *currProcess = (QueueData *)malloc(sizeof(QueueData));
        currProcess = MLFQ.Level4->items[MLFQ.Level4->front];
        int processID = currProcess->id;
        setState(processID, "Running");
        int pc = getPc(processID);
        printf("ProcessID currently %d\n", processID);
        int lowerBound = getLowerBound(processID);
        char *currentInstruction = getCurrentInstruction(pc + lowerBound);
        bool isNotBlocked = interpreter(processID, currentInstruction);
        setPc(processID, getPc(processID) + 1);
        MLFQ.Level4->remainingQuantum = (MLFQ.Level4->remainingQuantum) - 1;
        int remainingCycles = remainingExecution(processID);
        printf("Process state in the beginning: %s \n", getState(processID));

        if (isNotBlocked)
        {
            if (remainingCycles != 0)
            {
                if (MLFQ.Level4->remainingQuantum == 0)
                {
                    enqueue(MLFQ.Level4, dequeue(MLFQ.Level4));
                    setState(processID, "Ready");
                    currExecutingLevel = -1;
                    MLFQ.Level4->remainingQuantum = MLFQ.Level4->OriginalQuantum;
                    printf("Process state right now: %s \n", getState(processID));
                }
            }
            else
            {
                dequeue(MLFQ.Level4);
                setState(processID, "Finished");
                currExecutingLevel = -1;
                MLFQ.Level4->remainingQuantum = MLFQ.Level4->OriginalQuantum;
                printf("Process state right now: %s \n", getState(processID));
            }
        }
        else
        {
            dequeue(MLFQ.Level4);
            setState(processID, "Blocked");
            currExecutingLevel = -1;
            if (MLFQ.Level4->remainingQuantum == 0)
            {
                setPriority(processID, getPriority(processID) + 1);
            }
            MLFQ.Level4->remainingQuantum = MLFQ.Level4->OriginalQuantum;
            printf("Process state right now: %s \n", getState(processID));
        }
    }
}

int main()
{
    // writeFile("test123", "try");
    // printf("%s",readFile("test"));
    generalBlockingQueue = createQueueNoQuantum();
    InputMutex = createMutex();
    OutputMutex = createMutex();
    FileMutex = createMutex();
    StartScheduler(&MLFQ);
    int arrivalP1;
    printf("Please enter arrival time for P1:\n");
    scanf("%d", &arrivalP1);
    int arrivalP2;
    printf("Please enter arrival time for P2:\n");

    scanf("%d", &arrivalP2);
    int arrivalP3;
    printf("Please enter arrival time for P3:\n");
    scanf("%d", &arrivalP3);
    printf("--------------------------------------------\n");
    while (1)
    {
        if (currClockCycle == arrivalP1)
        {
            Memory[42] = (MemoryElement *)malloc(sizeof(MemoryElement));
            Memory[42]->name = "PID";
            Memory[42]->value = strdup("1");
            Memory[43] = (MemoryElement *)malloc(sizeof(MemoryElement));
            Memory[43]->name = "ProcessState";
            Memory[43]->value = strdup("Ready");
            Memory[44] = (MemoryElement *)malloc(sizeof(MemoryElement));
            Memory[44]->name = "ProcessPriority";
            Memory[44]->value = strdup("1");
            Memory[45] = (MemoryElement *)malloc(sizeof(MemoryElement));
            char ProcessPC[20];
            sprintf(ProcessPC, "%d", 0);
            Memory[45]->name = "ProcessPC";
            Memory[45]->value = strdup("0");
            Memory[46] = (MemoryElement *)malloc(sizeof(MemoryElement));
            Memory[46]->name = "ProcessLB";
            char lower[20];
            // printf("memory instruction %d", memoryInstructions);
            sprintf(lower, "%d", memoryInstructions);
            Memory[46]->value = strdup(lower);
            ReadProgram("Program_1.txt");
            char UpperBound[20];
            sprintf(UpperBound, "%d", memoryInstructions - 1);
            Memory[47] = (MemoryElement *)malloc(sizeof(MemoryElement));
            Memory[47]->name = "ProcessUB";
            Memory[47]->value = strdup(UpperBound);
            QueueData *process = (QueueData *)malloc(sizeof(QueueData));
            process->id = atoi(Memory[42]->value);
            process->priority = 1;
            enqueue(MLFQ.Level1, process);
        }
        if (currClockCycle == arrivalP2)
        {
            Memory[48] = (MemoryElement *)malloc(sizeof(MemoryElement));
            Memory[48]->name = "PID";
            // char ProcessId[20];
            // sprintf(ProcessId, "%d", newArrivedprocessID);
            // newArrivedprocessID++;
            Memory[48]->value = strdup("2");

            Memory[49] = (MemoryElement *)malloc(sizeof(MemoryElement));
            Memory[49]->name = "ProcessState";
            Memory[49]->value = strdup("Ready");
            Memory[50] = (MemoryElement *)malloc(sizeof(MemoryElement));
            Memory[50]->name = "ProcessPriority";
            Memory[50]->value = strdup("1");
            char Process2PC[20];
            sprintf(Process2PC, "%d", memoryInstructions);
            Memory[51] = (MemoryElement *)malloc(sizeof(MemoryElement));
            Memory[51]->name = "ProcessPC";
            Memory[51]->value = strdup("0");
            Memory[52] = (MemoryElement *)malloc(sizeof(MemoryElement));
            Memory[52]->name = "ProcessLB";
            Memory[52]->value = strdup(Process2PC);
            ReadProgram("Program_2.txt");
            char UpperBound[20];
            sprintf(UpperBound, "%d", memoryInstructions - 1);
            Memory[53] = (MemoryElement *)malloc(sizeof(MemoryElement));
            Memory[53]->name = "ProcessUB";
            Memory[53]->value = strdup(UpperBound);
            QueueData *process = (QueueData *)malloc(sizeof(QueueData));
            process->id = atoi(Memory[48]->value);
            process->priority = 1;
            enqueue(MLFQ.Level1, process);
        }
        if (currClockCycle == arrivalP3)
        {
            Memory[54] = (MemoryElement *)malloc(sizeof(MemoryElement));
            Memory[54]->name = "PID";
            Memory[54]->value = strdup("3");
            Memory[55] = (MemoryElement *)malloc(sizeof(MemoryElement));
            Memory[55]->name = "ProcessState";
            Memory[55]->value = strdup("Ready");
            Memory[56] = (MemoryElement *)malloc(sizeof(MemoryElement));
            Memory[56]->name = "ProcessPriority";
            Memory[56]->value = strdup("1");
            // printf("memory instruction %d \n", memoryInstructions);
            char Process3PC[20];
            sprintf(Process3PC, "%d", memoryInstructions);
            Memory[57] = (MemoryElement *)malloc(sizeof(MemoryElement));
            Memory[57]->name = "ProcessPC";
            Memory[57]->value = strdup("0");
            Memory[58] = (MemoryElement *)malloc(sizeof(MemoryElement));
            Memory[58]->name = "ProcessLB";
            Memory[58]->value = strdup(Process3PC);
            // printf("memory instruction %d \n", memoryInstructions);
            ReadProgram("Program_3.txt");
            char UpperBound[20];
            // printf("memory instruction %d", memoryInstructions);
            sprintf(UpperBound, "%d", memoryInstructions - 1);
            Memory[59] = (MemoryElement *)malloc(sizeof(MemoryElement));
            Memory[59]->name = "ProcessUB";
            Memory[59]->value = strdup(UpperBound);
            QueueData *process = (QueueData *)malloc(sizeof(QueueData));
            process->id = atoi(Memory[54]->value);
            process->priority = 1;
            enqueue(MLFQ.Level1, process);
        }
        printf(BOLD BLUE "Clock Cycle %d \n" RESET, currClockCycle);
        setCurrExecutingLevel();
        printf("General Blocked Queue: ");
        DisplayQueue(generalBlockingQueue);
        printf("Queue Level 1: ");
        DisplayQueue(MLFQ.Level1);
        printf("Queue Level 2: ");
        DisplayQueue(MLFQ.Level2);
        printf("Queue Level 3: ");
        DisplayQueue(MLFQ.Level3);
        printf("Queue Level 4: ");
        DisplayQueue(MLFQ.Level4);
        execute();
        // print memory at the end of cycle
        // displayMemory();
        printf("------------------------\n");

        if (isEmpty(MLFQ.Level1) && isEmpty(MLFQ.Level2) &&
            isEmpty(MLFQ.Level3) && isEmpty(MLFQ.Level4) && (currClockCycle > arrivalP1) && (currClockCycle > arrivalP2) && (currClockCycle > arrivalP3))
        {
            break;
        }
        currClockCycle++;
    }
    // displayMemory();
}