#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/times.h>

enum{
    ARG_CHILDREN = 1,
    ARG_SECONDS = 2,
    ARG_LOWER_PRIORITIES = 3,
    ARGC_EXPECTED = 4,

    MIN_CHILDREN = 1,
    MIN_SECONDS = 0,
    LOWER_PRIORITIES_NO = 0,
    LOWER_PRIORITIES_YES = 1,

    PARSE_OK = 0,
    PARSE_ERR = -1,
    STRTOL_BASE_10 = 10,
};

struct forkprio_config
{
    int children;
    int seconds;
    int lower_priorities;
};

static int
parse_int_arg(const char *str, int min, int max, int *out_val)
{
    char *end = NULL;
    long value = 0;

    errno = 0;
    value = strtol(str, &end, STRTOL_BASE_10);

    if (errno != 0 || end == str || *end != '\0' || value < min || value > max)
    {
        return PARSE_ERR;
    }

    *out_val = (int)value;
    return PARSE_OK;
}

static int
parse_args(int argc, char *argv[], struct forkprio_config *cfg)
{
    if (argc != ARGC_EXPECTED)
    {
        fprintf(stderr,
                "Usage: %s <children> <seconds> <lower_priorities: 0|1>\n",
                argv[0]);
        return PARSE_ERR;
    }

    if (parse_int_arg(argv[ARG_CHILDREN], MIN_CHILDREN, INT_MAX,
                      &cfg->children) == PARSE_ERR)
    {
        fprintf(stderr, "Invalid children: %s\n", argv[ARG_CHILDREN]);
        return PARSE_ERR;
    }

    if (parse_int_arg(argv[ARG_SECONDS], MIN_SECONDS, INT_MAX,
                      &cfg->seconds) == PARSE_ERR)
    {
        fprintf(stderr, "Invalid seconds: %s\n", argv[ARG_SECONDS]);
        return PARSE_ERR;
    }

    if (parse_int_arg(argv[ARG_LOWER_PRIORITIES], LOWER_PRIORITIES_NO,
                      LOWER_PRIORITIES_YES, &cfg->lower_priorities) == PARSE_ERR)
    {
        fprintf(stderr, "Invalid lower_priorities (must be 0 or 1): %s\n",
                argv[ARG_LOWER_PRIORITIES]);
        return PARSE_ERR;
    }

    return PARSE_OK;
}

static void sigterm_handler(int signum)
{
    struct rusage usage;
    int prio;
    long total_time;

    (void)signum;

    errno = 0;
    prio = getpriority(PRIO_PROCESS, 0);

    if (getrusage(RUSAGE_SELF, &usage) == 0)
    {
        total_time = usage.ru_utime.tv_sec + usage.ru_stime.tv_sec;
    }
    else
    {
        total_time = 0;
    }

    printf("Child %d (nice %2d):\t%3li\n", getpid(), prio, total_time);

    exit(EXIT_SUCCESS);
}

int busywork(void)
{
    struct tms buf;
    for (;;)
    {
        times(&buf);
    }
}

int main(int argc, char *argv[])
{
    struct forkprio_config cfg;
    pid_t *pids;
    int i;

    if (parse_args(argc, argv, &cfg) < 0)
    {
        exit(EXIT_FAILURE);
    }

    pids = calloc((size_t)cfg.children, sizeof(pid_t));
    if (!pids)
    {
        perror("calloc");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < cfg.children; i++)
    {
        pid_t pid = fork();

        if (pid < 0)
        {
            perror("fork");

            int j;
            for (j = 0; j < i; j++)
                kill(pids[j], SIGTERM);
            for (j = 0; j < i; j++)
                wait(NULL);

            free(pids);
            exit(EXIT_FAILURE);
        }

        if (pid == 0)
        {
            struct sigaction sa;

            sa.sa_handler = sigterm_handler;
            sigemptyset(&sa.sa_mask);
            sa.sa_flags = 0;

            if (sigaction(SIGTERM, &sa, NULL) == -1)
            {
                perror("sigaction");
                _exit(EXIT_FAILURE);
            }

            if (cfg.lower_priorities == LOWER_PRIORITIES_YES)
            {
                if (setpriority(PRIO_PROCESS, 0, i) == -1)
                {
                    perror("setpriority");
                }
            }

            busywork();

            exit(EXIT_SUCCESS);
        }
        else
        {
            pids[i] = pid;
        }
    }

    if (cfg.seconds > 0)
    {
        sleep((unsigned int)cfg.seconds);
    }
    else
    {
        while (1)
            pause();
    }

    for (i = 0; i < cfg.children; i++)
    {
        kill(pids[i], SIGTERM);
    }

    for (i = 0; i < cfg.children; i++)
    {
        wait(NULL);
    }

    free(pids);
    exit(EXIT_SUCCESS);
}