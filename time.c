#include <time.h>

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
        r = rand() % 4;
        sleep(7 + r);
        break;
    }
}

// Fetches the system time
// and outputs a string
// formatted timestamp.
char *gettime()
{
    time_t *current = (time_t *)malloc(sizeof(time_t));
    time(current);

    char *time = ctime(current);
    time[strlen(time) - 1] = 0;

    return time;
}
