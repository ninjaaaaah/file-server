#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

#include "queue.c"

/**
 * Sleeps for simulating file access.
 */
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
        r = rand() % 4;
        sleep(7 + r);
        break;
    }
}

/**
 * Fetches the system time
 * and outputs a string
 * formatted timestamp.
 */
char *gettime()
{
    time_t *current = (time_t *)malloc(sizeof(time_t));
    time(current);

    char *time = ctime(current);
    time[strlen(time) - 1] = 0;

    return time;
}

/**
 * Logs the command input onto
 * the file name provided at file.
 */
void logcmd(char *buf, char *file)
{
    FILE *log = fopen(file, "a");
    fprintf(log, "[%s]: %s\n", gettime(), buf);
    fclose(log);
}

/**
 * Error handling in case command from
 * input is not supported.
 */
void invalidcmd(CMD *cmd)
{
    printf("Unsupported command!\n");
}

/**
 * Writes a string on the specified
 * file and creates the file if it
 * doesn't exist.
 */
void writecmd(CMD *cmd)
{
    FILE *file = fopen(cmd->dir, "a");
    int i = 0;
    while (cmd->str[i] != '\0')
    {
        fprintf(file, "%c", cmd->str[i++]);
        sleep(0.025);
    }
    fclose(file);
}

/**
 * Reads the contents of a file and
 * writes it onto read.txt.
 */
void readcmd(CMD *cmd)
{
    FILE *file = fopen(cmd->dir, "r");
    FILE *f_read = fopen("read.txt", "a");
    char c;
    fprintf(f_read, "%s:\t", cmd->input);
    if (file)
    {
        while ((c = fgetc(file)) != EOF)
            fprintf(f_read, "%c", c);
        fprintf(f_read, "\n");
        fclose(file);
    }
    else
        fprintf(f_read, "FILE DNE\n");
    fclose(f_read);
}

/*
 * Writes a content of a file to empty.txt
 * and empties the file.
 */
void emptycmd(CMD *cmd)
{
    FILE *file = fopen(cmd->dir, "r");
    FILE *f_empty = fopen("empty.txt", "a");
    char c;

    fprintf(f_empty, "%s:\t", cmd->input);
    if (file)
    {
        while ((c = fgetc(file)) != EOF)
            fprintf(f_empty, "%c", c);
        fprintf(f_empty, "\n");
        file = fopen(cmd->dir, "w");
        fclose(file);
        randsleep(2);
    }
    else
        fprintf(f_empty, "FILE ALREADY EMPTY\n");
    fclose(f_empty);
}

/**
 * Deconstructs the input buffer into
 * command, directory, and str which
 * will be used to execute certain
 * functions.
 */
CMD *parsecmd(char *buf)
{
    CMD *cmd = malloc(sizeof(CMD));
    cmd->type = INVALID;
    strcpy(cmd->input, buf);

    char *command = strtok(buf, " ");
    if (!command)
        return cmd;
    char *dir;
    char *str;

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
        logcmd(cmd->input, "commands.txt");

    return cmd;
}

/**
 * Resets the buf memory address and
 * places the result of fgets then
 * truncates the endline character.
 */
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

/**
 * Empties all the log files used
 * in this program.
 */
void emptyfiles()
{
    int i = 4;
    char *files[] = {"commands.txt", "read.txt", "empty.txt", "dump.txt"};
    FILE *file;
    while (i--)
        file = fopen(files[i], "w");
    fclose(file);
}

/**
 * Array of function pointers that
 * corresponds to the function that
 * is being executed.
 */
void (*execute[])(CMD *) = {invalidcmd, writecmd, readcmd, emptycmd};