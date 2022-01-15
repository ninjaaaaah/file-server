#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "commands.c"

// == G L O B A L  V A R I A B L E S == //
int active_files = 0;

// Thread that is assigned to run
// the worker function.
pthread_t t_worker;

// == F U N C T I O N S == //

// Initializes the values of
// the FileQueue.
FileQueue *initFQ()
{
	FileQueue *fq = malloc(sizeof(FileQueue));
	fq->head = NULL;
	fq->tail = NULL;
	return fq;
}

// Initializes the values of
// the CommandQueue.
CMDQueue *initCQ()
{
	CMDQueue *cq = malloc(sizeof(CMDQueue));
	cq->head = NULL;
	cq->tail = NULL;
	return cq;
}

// Creates a copy of CMD cmd and enqueues
// it into CMDQueue cq.
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

// Returns the cmd at head and
// dequeues the head of q.
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

// Creates a file that assigns
// the value of the command dir
// to its dir field.
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

// Pushes a file f into the FileQueue fq.
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

// Loops through all the active files
// in the FileQueue fq and returns the
// file with its file->dir that matches
// the cmd->dir.
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

// Array of function pointers that
// corresponds to the function that
// is being executed.
void (*execute[])(CMD *) = {invalidcmd, readcmd, writecmd, emptycmd};

// Works as a worker thread that is spawned
// each time the master thread dispatches
// on request.
void *worker(File *file)
{
	randsleep(1);

	sem_wait(file->sem);
	CMD *cmd = dequeueCMD(file->commands);
	execute[cmd->type](cmd);
	logcmd(cmd->input, "dump.txt");
	sem_post(file->sem);

	sem_destroy(file->sem);
}

// Acts as the master thread as C implicitly
// creates a thread that handles this part
// of the program.
int main()
{
	FileQueue *active = initFQ();
	char buf[2 * BUFFER + 8];

	emptyfiles();

	while (getcmd(buf, sizeof(buf)))
	{
		CMD *cmd = parsecmd(buf);
		if (!cmd->type)
			continue;

		File *file = getFile(active, cmd);
		enqueueCMD(file->commands, cmd);

		pthread_create(&t_worker, NULL, (void *)worker, file);
		pthread_detach(t_worker);
	}
	return 0;
}