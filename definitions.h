#include <semaphore.h>

#define BUFFER 50
#define INVALID 0
#define WRITE 1
#define READ 2
#define EMPTY 3

/**
 * Command structure that contains
 * the necessary information that
 * the functions will use
 */
typedef struct cmd CMD;
struct cmd
{
    int type;
    char input[2 * BUFFER + 8];
    char dir[BUFFER];
    char str[BUFFER];
    CMD *next;
};

/**
 * CMDQueue structure that holds all
 * the values of commands that have
 * been saved from user input.
 */
typedef struct cqueue CMDQueue;
struct cqueue
{
    int count;
    CMD *head;
    CMD *tail;
};

/**
 * File structure holds a corresponding
 * CMDQueue that would be dequeued
 * to get the command to execute and
 * a semaphore for locking the critical
 * section of modifying a file.
 */
typedef struct file File;
struct file
{
    char dir[BUFFER];
    CMDQueue *commands;
    sem_t *sem;
    File *next;
};

/**
 * FileQueue structure acts as a queue
 * of files which links the Files together
 * in a queue.
 */
typedef struct fqueue FileQueue;
struct fqueue
{
    int count;
    File *head;
    File *tail;
};