#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "commands.c"

/**
 * Works as a worker thread that is spawned
 * each time the master thread dispatches
 * on request.
 */
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

/**
 * Acts as the master thread as C implicitly
 * creates a thread that handles this part
 * of the program.
 */
int main()
{
	FileQueue *active = initFQ();
	char buf[2 * BUFFER + 8];
	pthread_t t_worker;

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