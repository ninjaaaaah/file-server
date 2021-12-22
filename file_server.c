#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INVALID 0
#define READ 2
#define WRITE 4
#define EMPTY 8

// structs
struct cmd
{
	int type;
	char input[100];
	char dir[100];
	char str[100];
};

// global variables
sem_t mutex;
pthread_t t_master;
pthread_t t_worker;

// function declarations
void *master(void);
void *worker(struct cmd *cmd);

void *logcmd(char *buf);

void *read(struct cmd *cmd);
void *write(struct cmd *cmd);
void *empty(struct cmd *cmd);

struct cmd *parsecmd(char *buf);

int getcmd(char *buf, int nbuf);

int main()
{
	sem_init(&mutex, 0, 1);
	pthread_create(&t_master, NULL, (void *)master, NULL);
	pthread_join(t_master, NULL);
}

int getcmd(char *buf, int nbuf)
{
	memset(buf, 0, nbuf);
	fgets(buf, nbuf, stdin);
	if (buf[0] == 0)
		return -1;
	buf[strlen(buf) - 1] = 0;
	return 1;
}

struct cmd *parsecmd(char *buf)
{
	char *command;
	char *dir;
	char *str;

	struct cmd *cmd = malloc(sizeof(cmd));

	cmd->type = INVALID;
	strcpy(cmd->input, buf);

	command = strtok(buf, " ");
	if (strcmp(command, "write") == 0)
	{
		cmd->type = WRITE;
		dir = strtok(NULL, " ");
		str = strtok(NULL, "\0");
		strcpy(cmd->str, str);
	}
	else
	{
		if (strcmp(command, "read") == 0)
			cmd->type = READ;
		if (strcmp(command, "empty") == 0)
			cmd->type = EMPTY;
		dir = strtok(NULL, "\0");
	}

	strcpy(cmd->dir, dir);

	return cmd;
}

void *master(void)
{
	char buf[100];
	struct cmd *cmd = malloc(sizeof(cmd));

	while (getcmd(buf, sizeof(buf)))
	{
		sem_wait(&mutex);
		logcmd(buf);
		cmd = parsecmd(buf);
		sem_post(&mutex);

		pthread_create(&t_worker, NULL, (void *)worker, cmd);
		pthread_join(t_worker, NULL);
	}
}

void *worker(struct cmd *cmd)
{
	switch (cmd->type)
	{

	case WRITE:
		write(cmd);
		break;

	case READ:
		read(cmd);
		break;

	case EMPTY:
		empty(cmd);
		break;

	default:
		printf("unsupported command\n");
	}
	sem_destroy(&mutex);
}

void *logcmd(char *buf)
{
	FILE *f_commands;
	f_commands = fopen("commands.txt", "a");
	fprintf(f_commands, "%s\n", buf);
	fclose(f_commands);
}

void *write(struct cmd *cmd)
{
	printf("write(%s, %s)\n", cmd->dir, cmd->str);
	// return;
}

void *read(struct cmd *cmd)
{
	printf("read(%s)\n", cmd->dir);
	// return;
}

void *empty(struct cmd *cmd)
{
	printf("empty(%s)\n", cmd->dir);
	// return;
}