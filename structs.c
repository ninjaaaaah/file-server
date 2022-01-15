#include <semaphore.h>

#include "definitions.h"

// == S T R U C T S == //

// Command structure that contains
// the necessary information that
// the functions will use
typedef struct cmd CMD;
struct cmd
{
    int type;
    char input[2 * BUFFER + 8];
    char dir[BUFFER];
    char str[BUFFER];
    CMD *next;
};

// CMDQueue structure that holds all
// the values of commands that have
// been saved from user input.
typedef struct cqueue CMDQueue;
struct cqueue
{
    CMD *head;
    CMD *tail;
};

typedef struct files File;
struct files
{
    char dir[BUFFER];
    CMDQueue *commands;
    sem_t *sem;
    File *next;
};

typedef struct fqueue FileQueue;
struct fqueue
{
    File *head;
    File *tail;
};

typedef struct args Args;
struct args
{
    FileQueue *fq;
    CMDQueue *cq;
};