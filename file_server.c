#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define BUFFER 420
#define INVALID 0
#define READ 2
#define WRITE 4
#define EMPTY 8

// == S T R U C T S == //

// Command structure that contains
// the necessary information that
// the functions will use
struct cmd
{
	int type;
	char input[BUFFER];
	char dir[BUFFER];
	char str[BUFFER];
};

// Queue structure that holds all
// the values of commands that have
// been saved from user input.
struct queue
{
	struct chain *head;
	struct chain *tail;
};

// Chain structure that links cmds
// in the queue.
struct chain
{
	struct cmd *cmd;
	struct chain *next;
};

// == G L O B A L  V A R I A B L E S == //

// Used as a lock variable to
// prevent data races while
// files are being processed.
sem_t mutex;

// Used as a lock variable to
// prevent data races while
// the queue is being accessed.
sem_t queue_lock;

// Thread that is assigned to run
// the worker function.
pthread_t t_worker;

// == F U N C T I O N S == //

// Pushes cmd struct cmd into queue struct q.
void push_queue(struct queue *q, struct cmd *cmd)
{
	struct chain *nc;
	nc = malloc(sizeof(struct chain));
	nc->cmd = cmd;
	nc->next = NULL;

	if (!q->head)
		q->head = nc;
	else
		q->tail->next = nc;
	q->tail = nc;
}

// Pops the queue and returns the head cmd.
struct cmd *pop_queue(struct queue *q)
{
	struct cmd *cmd;
	cmd = malloc(sizeof(struct cmd));

	cmd->type = q->head->cmd->type;
	strcpy(cmd->input, q->head->cmd->input);
	strcpy(cmd->dir, q->head->cmd->dir);
	strcpy(cmd->str, q->head->cmd->str);

	struct chain *tc;
	tc = q->head;
	q->head = tc->next;
	free(tc);

	return cmd;
}

// Sleeps for simulating file access.
void randsleep(int type)
{
	srand(time(0));
	int r;
	switch (type)
	{
	case 1:
		r = rand() % 100;
		if (r < 80)
			sleep(1);
		else
			sleep(6);
		break;
	case 2:
		r = rand() % 3;
		sleep(7 + r);
		break;
	}
}

// Logs the command input
// onto the commands.txt file.
void *logcmd(char *buf)
{
	FILE *f_commands = fopen("commands.txt", "a");
	sem_wait(&mutex);
	fprintf(f_commands, "%s\n", buf);
	fclose(f_commands);
	sem_post(&mutex);
}

// Reads the contents of a file and
// writes it onto read.txt.
void *readcmd(struct cmd *cmd)
{
	FILE *file = fopen(cmd->dir, "r");
	FILE *f_read = fopen("read.txt", "a");
	char cont[BUFFER];
	fprintf(f_read, "%s: ", cmd->input);
	sem_wait(&mutex);
	if (file)
	{
		while (fgets(cont, BUFFER, file))
			fprintf(f_read, "%s", cont);
		fclose(file);
	}
	else
		fprintf(f_read, "FILE DNE\n");
	fclose(f_read);
	sem_post(&mutex);
}

// Writes a string on the specified
// file and creates the file if it
// doesn't exist.
void *writecmd(struct cmd *cmd)
{
	FILE *file = fopen(cmd->dir, "a");
	sem_wait(&mutex);
	fprintf(file, "%s\n", cmd->str);
	fclose(file);
	sleep(strlen(cmd->str) * 0.025);
	sem_post(&mutex);
}

// Writes a content of a file to empty.txt
// and empties the file.
void *emptycmd(struct cmd *cmd)
{
	FILE *file = fopen(cmd->dir, "r");
	FILE *f_empty = fopen("empty.txt", "a");
	char cont[BUFFER];
	sem_wait(&mutex);
	fprintf(f_empty, "%s: ", cmd->input);
	if (file)
	{
		while (fgets(cont, BUFFER, file))
			fprintf(f_empty, "%s", cont);
		fclose(file);
		file = fopen(cmd->dir, "w");
		fclose(file);
	}
	else
		fprintf(f_empty, "FILE ALREADY EMPTY\n");
	fclose(f_empty);
	sem_post(&mutex);
	randsleep(2);
}

// Deconstructs the input buffer into
// command, directory, and str which
// will be used to execute certain
// functions.
struct cmd *parsecmd(char *buf)
{
	char *command;
	char *dir;
	char *str;

	struct cmd *cmd = malloc(sizeof(struct cmd));

	cmd->type = INVALID;
	strcpy(cmd->input, buf);

	command = strtok(buf, " ");
	if (strcmp(command, "write") == 0)
	{
		cmd->type = WRITE;
		dir = strtok(NULL, " ");
		str = strtok(NULL, "\0");
		if (str)
			strcpy(cmd->str, str);
	}
	else
	{
		if (strcmp(command, "read") == 0)
			cmd->type = READ;
		else if (strcmp(command, "empty") == 0)
			cmd->type = EMPTY;
		dir = strtok(NULL, "\0");
	}

	if (dir)
		strcpy(cmd->dir, dir);

	return cmd;
}

// Resets the buf memory address and
// places the result of fgets then
// truncates the endline character.
int getcmd(char *buf, int nbuf)
{
	memset(buf, 0, nbuf);
	fgets(buf, nbuf, stdin);
	if (strlen(buf))
	{
		buf[strlen(buf) - 1] = '\0';
		return 1;
	}
	return 0;
}

// Works as a worker thread that is spawned
// each time the master thread dispatches
// on request.
void *worker(struct queue *q)
{
	sem_wait(&queue_lock);
	struct cmd *cmd = pop_queue(q);
	sem_post(&queue_lock);

	randsleep(1);
	switch (cmd->type)
	{
	case WRITE:
		writecmd(cmd);
		break;
	case READ:
		readcmd(cmd);
		break;
	case EMPTY:
		emptycmd(cmd);
		break;
	default:
		printf("unsupported command\n");
	}

	free(cmd);
	sem_destroy(&mutex);
}

// Acts as the master thread as C implicitly
// creates a thread that handles this part
// of the program.
int main()
{
	struct queue *queue;
	queue = malloc(sizeof(struct queue));
	sem_init(&mutex, 0, 1);
	sem_init(&queue_lock, 0, 1);
	char buf[BUFFER];
	struct cmd *cmd;

	while (getcmd(buf, sizeof(buf)))
	{
		logcmd(buf);
		if (!strlen(buf))
			continue;
		cmd = parsecmd(buf);
		push_queue(queue, cmd);

		pthread_create(&t_worker, NULL, (void *)worker, queue);
		pthread_detach(t_worker);
	}
	return 0;
}