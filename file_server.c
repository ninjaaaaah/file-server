#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define BUFFER 50
#define INVALID 0
#define READ 1
#define WRITE 2
#define EMPTY 3

// == S T R U C T S == //

// Command structure that contains
// the necessary information that
// the functions will use
struct cmd
{
	int type;
	char input[2 * BUFFER + 8];
	char dir[BUFFER];
	char str[BUFFER];
	struct cmd *next;
};

// Queue structure that holds all
// the values of commands that have
// been saved from user input.
struct queue
{
	struct cmd *head;
	struct cmd *tail;
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
	struct cmd *ncmd;
	ncmd = malloc(sizeof(struct cmd));

	ncmd->type = cmd->type;
	strcpy(ncmd->input, cmd->input);
	strcpy(ncmd->str, cmd->str);
	strcpy(ncmd->dir, cmd->dir);
	ncmd->next = NULL;

	if (!q->head)
		q->head = ncmd;
	else
		q->tail->next = ncmd;
	q->tail = ncmd;
}

// Pops the queue and returns the head cmd.
struct cmd *pop_queue(struct queue *q)
{
	struct cmd *cmd;
	cmd = malloc(sizeof(struct cmd));

	cmd->type = q->head->type;
	strcpy(cmd->input, q->head->input);
	strcpy(cmd->dir, q->head->dir);
	strcpy(cmd->str, q->head->str);

	struct cmd *tcmd;
	tcmd = q->head;
	q->head = tcmd->next;
	free(tcmd);

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

// Fetches the system time
// and outputs a string
// formatted timestamp.
char *gettime()
{
	time_t *current;
	current = malloc(sizeof(time_t));
	time(current);

	return ctime(current);
}

// Logs the command input
// onto the commands.txt file.
void *logcmd(char *buf)
{
	FILE *f_commands = fopen("commands.txt", "a");
	fprintf(f_commands, "%s | %s", buf, gettime());
	fclose(f_commands);
}

// Error handling in case command from
// input is not supported.
void invalidcmd(struct cmd *cmd)
{
	printf("Unsupported command!\n");
}

// Reads the contents of a file and
// writes it onto read.txt.
void readcmd(struct cmd *cmd)
{
	FILE *file = fopen(cmd->dir, "r");
	FILE *f_read = fopen("read.txt", "a");
	char cont[BUFFER];
	fprintf(f_read, "%s:\t", cmd->input);
	if (file)
	{
		while (fgets(cont, BUFFER, file))
			fprintf(f_read, "%s", cont);
		fclose(file);
	}
	else
		fprintf(f_read, "FILE DNE\n");
	fclose(f_read);
}

// Writes a string on the specified
// file and creates the file if it
// doesn't exist.
void writecmd(struct cmd *cmd)
{
	FILE *file = fopen(cmd->dir, "a");
	fprintf(file, "%s\n", cmd->str);
	fclose(file);
	sleep(strlen(cmd->str) * 0.025);
}

// Writes a content of a file to empty.txt
// and empties the file.
void emptycmd(struct cmd *cmd)
{
	FILE *file = fopen(cmd->dir, "r");
	FILE *f_empty = fopen("empty.txt", "a");
	char cont[BUFFER];
	fprintf(f_empty, "%s:\t", cmd->input);
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

	if (cmd->type)
		logcmd(cmd->input);

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

// Array of function pointers of
// the supported functions.
void (*functions[])(struct cmd *) = {invalidcmd, readcmd, writecmd, emptycmd};

// Works as a worker thread that is spawned
// each time the master thread dispatches
// on request.
void *worker(struct queue *q)
{
	randsleep(1);

	sem_wait(&queue_lock);
	struct cmd *cmd = pop_queue(q);
	functions[cmd->type](cmd);
	sem_post(&queue_lock);

	free(cmd);
	sem_destroy(&mutex);
	sem_destroy(&queue_lock);
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
	char buf[2 * BUFFER + 8];
	struct cmd *cmd;

	while (getcmd(buf, sizeof(buf)))
	{
		if (!strlen(buf))
			continue;

		cmd = parsecmd(buf);
		push_queue(queue, cmd);

		pthread_create(&t_worker, NULL, (void *)worker, queue);
		pthread_detach(t_worker);
	}
	return 0;
}