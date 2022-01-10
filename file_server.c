#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

// == G L O B A L  V A R I A B L E S == //

// Used as a lock variable to
// prevent data races while
// files are being processed.
sem_t mutex;

// Thread that is assigned to run
// the worker function.
pthread_t t_worker;

// == F U N C T I O N S == //

// Works as a worker thread that is spawned
// each time the master thread dispatches
// on request.
void *worker(struct cmd *cmd);

// Logs the command input
// onto the commands.txt file.
void *logcmd(char *buf);

// Reads the contents of a file and
// writes it onto read.txt.
void *readcmd(struct cmd *cmd);

// Writes a string on the specified
// file.
void *writecmd(struct cmd *cmd);

// Writes a content of a file to empty.txt
// and empties the file.
void *emptycmd(struct cmd *cmd);

// Deconstructs the input buffer into
// command, directory, and str which
// will be used to execute certain
// functions.
struct cmd *parsecmd(char *buf);

// Resets the buf memory address and
// places the result of fgets then
// truncates the endline character.
int getcmd(char *buf, int nbuf);

// Acts as the master thread as C implicitly
// creates a thread that handles this part
// of the program.
int main()
{
	sem_init(&mutex, 0, 1);
	char buf[BUFFER];
	struct cmd *cmd;

	while (getcmd(buf, sizeof(buf)))
	{
		logcmd(buf);
		cmd = parsecmd(buf);

		pthread_create(&t_worker, NULL, (void *)worker, cmd);
		pthread_detach(t_worker);
	}
}

int getcmd(char *buf, int nbuf)
{
	memset(buf, 0, nbuf);
	fgets(buf, nbuf, stdin);
	if (buf[0] == 0)
		return -1;
	buf[strlen(buf) - 1] = '\0';
	return 1;
}

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

void *worker(struct cmd *cmd)
{
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
	sem_destroy(&mutex);
}

void *logcmd(char *buf)
{
	FILE *f_commands = fopen("commands.txt", "a");
	sem_wait(&mutex);
	fprintf(f_commands, "%s\n", buf);
	fclose(f_commands);
	sem_post(&mutex);
}

void *writecmd(struct cmd *cmd)
{
	FILE *file = fopen(cmd->dir, "a");
	sem_wait(&mutex);
	fprintf(file, "%s\n", cmd->str);
	fclose(file);
	sem_post(&mutex);
}

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

void *emptycmd(struct cmd *cmd)
{
	FILE *file = fopen(cmd->dir, "r");
	FILE *f_empty = fopen("empty.txt", "a");
	char cont[BUFFER];
	fprintf(f_empty, "%s: ", cmd->input);
	sem_wait(&mutex);
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
}