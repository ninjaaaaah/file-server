#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "definitions.h"
#include "commands.c"

// == G L O B A L  V A R I A B L E S == //
int active_files = 0;

// Thread that is assigned to run
// the worker function.
pthread_t t_worker;

// == F U N C T I O N S == //

// Pushes CMD cmd into CMDQueue queue.
void enqueueCMD(CMDQueue *cq, CMD *cmd)
{
	CMD *ncmd = malloc(sizeof(CMD));
	strcpy(ncmd->dir, cmd->dir);
	strcpy(ncmd->input, cmd->input);
	strcpy(ncmd->str, cmd->str);
	ncmd->type = cmd->type;
	ncmd->next = NULL;

	if (!cq->head)
		cq->head = ncmd;
	else
		cq->tail->next = ncmd;
	cq->tail = ncmd;
}

// Pops the queue and returns the head cmd.
CMD *dequeueCMD(CMDQueue *q)
{
	CMD *cmd = malloc(sizeof(CMD));

	cmd->type = q->head->type;
	strcpy(cmd->input, q->head->input);
	strcpy(cmd->dir, q->head->dir);
	strcpy(cmd->str, q->head->str);

	CMD *tcmd;
	tcmd = q->head;
	q->head = tcmd->next;
	free(tcmd);

	return cmd;
}

FileQueue *initFQ()
{
	FileQueue *fq = malloc(sizeof(FileQueue));
	fq->head = NULL;
	fq->tail = NULL;
	return fq;
}

CMDQueue *initCQ()
{
	CMDQueue *cq = malloc(sizeof(CMDQueue));
	cq->head = NULL;
	cq->tail = NULL;
	return cq;
}

File *createFile(CMD *cmd)
{
	File *f = malloc(sizeof(File));
	f->commands = malloc(sizeof(CMDQueue));
	f->commands->head = NULL;
	f->commands->tail = NULL;
	strcpy(f->dir, cmd->dir);

	sem_t *sem = malloc(sizeof(sem_t));
	f->sem = sem;
	sem_init(f->sem, 0, 1);

	return f;
}

File *enqueueFile(FileQueue *fq, File *f)
{
	File *nf = malloc(sizeof(File));
	strcpy(nf->dir, f->dir);
	nf->commands = f->commands;
	nf->sem = f->sem;
	nf->next = NULL;

	if (!fq->head)
		fq->head = nf;
	else
		fq->tail->next = nf;
	fq->tail = nf;

	return nf;
}

File *getFile(FileQueue *fq, CMD *cmd)
{
	File *cursor = fq->head;
	int i;
	for (i = 0; i < active_files; i++)
		if (strcmp(cursor->dir, cmd->dir) == 0)
			return cursor;
		else
			cursor = cursor->next;

	active_files++;
	return enqueueFile(fq, createFile(cmd));
}

// Array of function pointers of
// the supported functions.
void (*execute[])(CMD *) = {invalidcmd, readcmd, writecmd, emptycmd};

// Works as a worker thread that is spawned
// each time the master thread dispatches
// on request.
void *worker(Args *q)
{
	randsleep(1);
	CMD *cmd = dequeueCMD(q->cq);
	File *file = getFile(q->fq, cmd);

	sem_wait(file->sem);
	cmd = dequeueCMD(file->commands);
	execute[cmd->type](cmd);
	logcmd(cmd->input, "done.txt");
	sem_post(file->sem);

	sem_destroy(file->sem);
}

void *traverseCommands(CMDQueue *cq)
{
	CMD *cursor = cq->head;

	do
		printf("%s\n", cursor->input);
	while ((cursor = cursor->next) != NULL);

	printf("===\n");
}

void *traverseFiles(FileQueue *fq)
{
	File *cursor = fq->head;

	do
	{
		printf("FILE: %s\n===\n", cursor->dir);
		traverseCommands(cursor->commands);
	} while ((cursor = cursor->next) != NULL);

	printf("= END OF FILE QUEUE =\n");
}

// Acts as the master thread as C implicitly
// creates a thread that handles this part
// of the program.
int main()
{
	FileQueue *active = initFQ();
	CMDQueue *request = initCQ();

	Args *args = malloc(sizeof(Args));
	args->fq = active;
	args->cq = request;

	emptyfiles();

	char buf[2 * BUFFER + 8];

	while (getcmd(buf, sizeof(buf)))
	{
		if (!strlen(buf))
			continue;

		CMD *cmd = parsecmd(buf);

		enqueueCMD(request, cmd);
		File *file = getFile(active, cmd);
		enqueueCMD(file->commands, cmd);

		pthread_create(&t_worker, NULL, (void *)worker, args);
		pthread_detach(t_worker);
	}
	return 0;
}