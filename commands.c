#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "structs.c"
#include "time.c"

// Logs the command input
// onto the commands.txt file.
void logcmd(char *buf, char *file)
{
    FILE *log = fopen(file, "a");
    fprintf(log, "[%s] %s\n", gettime(), buf);
    fclose(log);
}

// Error handling in case command from
// input is not supported.
void invalidcmd(CMD *cmd)
{
    printf("Unsupported command!\n");
}

// Reads the contents of a file and
// writes it onto read.txt.
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
        fclose(file);
    }
    else
        fprintf(f_read, "FILE DNE\n");
    fclose(f_read);
}

// Writes a string on the specified
// file and creates the file if it
// doesn't exist.
void writecmd(CMD *cmd)
{
    FILE *file = fopen(cmd->dir, "a");
    int i = 0;
    while (cmd->str[i] != '\0')
    {
        fprintf(file, "%c", cmd->str[i++]);
        sleep(0.025);
    }
    fprintf(file, "\n");
    fclose(file);
}

// Writes a content of a file to empty.txt
// and empties the file.
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
        fclose(file);
        file = fopen(cmd->dir, "w");
        fclose(file);
        randsleep(2);
    }
    else
        fprintf(f_empty, "FILE ALREADY EMPTY\n");
    fclose(f_empty);
}

// Deconstructs the input buffer into
// command, directory, and str which
// will be used to execute certain
// functions.
CMD *parsecmd(char *buf)
{
    CMD *cmd = malloc(sizeof(CMD));
    char *command;
    char *dir;
    char *str;

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
        logcmd(cmd->input, "commands.txt");

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

void emptyfiles()
{
    int i = 5;
    char *files[5] = {"commands.txt", "write.txt", "read.txt", "empty.txt", "done.txt"};
    FILE *file;
    while (i--)
        file = fopen(files[i], "w");
    fclose(file);
}