#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER 50
#define INVALID 0
#define READ 2
#define WRITE 4
#define EMPTY 8

// structs
struct cmd
{
	int type;
	char input[BUFFER];
	char dir[BUFFER];
	char str[BUFFER];
};

// global variables
sem_t mutex;
pthread_t t_master;
pthread_t t_worker;

// function declarations
void *master(void);
void *worker(struct cmd *cmd);

void *logcmd(char *buf);

void *readcmd(struct cmd *cmd);
void *writecmd(struct cmd *cmd);
void *emptycmd(struct cmd *cmd);

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

void *master(void)
{
	char buf[BUFFER];
	struct cmd *cmd;

	while (getcmd(buf, sizeof(buf)))
	{
		logcmd(buf);
		cmd = parsecmd(buf);

		pthread_create(&t_worker, NULL, (void *)worker, cmd);
		pthread_join(t_worker, NULL);
	}
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
	FILE *f_commands;
	f_commands = fopen("commands.txt", "a");
	fprintf(f_commands, "%s\n", buf);
	fclose(f_commands);
}

void *writecmd(struct cmd *cmd)
{
	printf("write(%s, %s)\n", cmd->dir, cmd->str);
	FILE *file = fopen(cmd->dir, "a");
	fprintf(file, "%s\n", cmd->str);
	fclose(file);
}

void *readcmd(struct cmd *cmd)
{
	printf("read(%s)\n", cmd->dir);

	FILE *file = fopen(cmd->dir, "r");
	FILE *f_read = fopen("read.txt", "a");
	char cont[BUFFER];
	fprintf(f_read, "%s: ", cmd->input);
	if (file)
		while (fgets(cont, BUFFER, file))
			fprintf(f_read, "%s", cont);
	else
		fprintf(f_read, "FILE DNE\n");
	fclose(file);
	fclose(f_read);
}

void *emptycmd(struct cmd *cmd)
{
	printf("empty(%s)\n", cmd->dir);
	FILE *file = fopen(cmd->dir, "w+");
	FILE *f_empty = fopen("empty.txt", "a");
	char cont[BUFFER];
	fprintf(f_empty, "%s: ", cmd->input);
	if (file)
	{
		sem_wait(&mutex);
		while (fgets(cont, BUFFER, file))
			fprintf(f_empty, "%s", cont);
		sem_post(&mutex);
		fprintf(file, "");
	}
	else
		fprintf(f_empty, "FILE ALREADY EMPTY\n");
	fclose(file);
	fclose(f_empty);
}