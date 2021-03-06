#include <stdio.h>

#include "definitions.h"

/**
 * Initializes the values of
 * the FileQueue.
 */
FileQueue *initFQ()
{
    FileQueue *fq = malloc(sizeof(FileQueue));
    fq->count = 0;
    fq->head = NULL;
    fq->tail = NULL;
    return fq;
}

/**
 * Initializes the values of
 * the CommandQueue.
 */
CMDQueue *initCQ()
{
    CMDQueue *cq = malloc(sizeof(CMDQueue));
    cq->count = 0;
    cq->head = NULL;
    cq->tail = NULL;
    return cq;
}

/**
 * Creates a copy of CMD cmd and enqueues
 * it into CMDQueue cq.
 */
void enqueueCMD(CMDQueue *cq, CMD *cmd)
{
    CMD *ncmd = malloc(sizeof(CMD));
    strcpy(ncmd->dir, cmd->dir);
    strcpy(ncmd->input, cmd->input);
    strcpy(ncmd->str, cmd->str);
    ncmd->type = cmd->type;
    ncmd->next = NULL;
    cq->count++;

    if (!cq->head)
        cq->head = ncmd;
    else
        cq->tail->next = ncmd;
    cq->tail = ncmd;
}

/**
 * Returns the cmd at head and
 * dequeues the head of q.
 */
CMD *dequeueCMD(CMDQueue *cq)
{
    CMD *cmd = malloc(sizeof(CMD));
    cmd->type = cq->head->type;
    strcpy(cmd->input, cq->head->input);
    strcpy(cmd->dir, cq->head->dir);
    strcpy(cmd->str, cq->head->str);
    cq->count--;

    CMD *tcmd = cq->head;
    cq->head = tcmd->next;
    free(tcmd);

    return cmd;
}

/**
 * Creates a file that assigns
 * the value of the command dir
 * to its dir field.
 */
File *createFile(CMD *cmd)
{
    File *f = malloc(sizeof(File));
    f->commands = initCQ();
    strcpy(f->dir, cmd->dir);

    sem_t *sem = malloc(sizeof(sem_t));
    f->sem = sem;
    sem_init(f->sem, 0, 1);

    return f;
}

/**
 * Pushes a file f into the FileQueue fq.
 */
void enqueueFile(FileQueue *fq, File *f)
{
    File *nf = malloc(sizeof(File));
    strcpy(nf->dir, f->dir);
    nf->commands = f->commands;
    nf->sem = f->sem;
    nf->next = NULL;
    fq->count++;

    if (!fq->head)
        fq->head = nf;
    else
        fq->tail->next = nf;
    fq->tail = nf;
}

/**
 * Loops through all the active files
 * in the FileQueue fq and returns the
 * file with its file->dir that matches
 * the cmd->dir.
 */
File *getFile(FileQueue *fq, CMD *cmd)
{
    File *cursor = fq->head;
    int i;
    for (i = 0; i < fq->count; i++)
        if (strcmp(cursor->dir, cmd->dir) == 0)
            return cursor;
        else
            cursor = cursor->next;

    cursor = createFile(cmd);
    enqueueFile(fq, cursor);

    return cursor;
}