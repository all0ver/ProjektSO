#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <string.h>
#include <signal.h>

/*
[0]-> miedzy M a 1
[1]->między 1 a 2
[2]->miedzy 2 a 3
M-macierzyst
1-pierwszy proces
2-drugi proces
3-trzeci proces
*/
sigset_t mask, blocker;
pid_t p1, p2, p3;
int pipes[3][2];
int pom = 0;
struct sygnaly
{
    int nr_sygnalu;

} syg;
void obsluga_1_wywolanie(int sig)
{
    switch (syg.nr_sygnalu)
    {
    case SIGTERM:
        raise(SIGTERM);
        break;
    case SIGCONT:
        raise(SIGCONT);
        break;
    case SIGSTOP:
        raise(SIGSTOP);
        break;
    }
}
void obsluga_3_init(int sig)
{

    if (kill(getppid(), SIGUSR2) == -1)
    {
        perror("error");
        exit(9);
    }
    if (kill(getppid(), sig) == -1)
    {
        perror("error");
        exit(9);
    }
}
void obsluga_main_init(int sig)
{
    sigprocmask(SIG_UNBLOCK, &blocker, &mask);
}
void obsluga_main(int sig)
{
    pom = 0;
    syg.nr_sygnalu = sig;
    write(pipes[0][1], &syg, sizeof(syg));
    write(pipes[1][1], &syg, sizeof(syg));
    write(pipes[2][1], &syg, sizeof(syg));
    if (kill(p1, SIGUSR1) == -1)
    {
        perror("error");
        exit(9);
    }
}
void obsluga_1(int sig)
{

    if (kill(p2, SIGUSR1) == -1)
    {
        perror("error");
        exit(9);
    }
    read(pipes[0][0], &syg, sizeof(syg));
    obsluga_1_wywolanie(sig);
}
void obsluga_2(int sig)
{
    read(pipes[1][0], &syg, sizeof(syg));
    if (kill(p3, SIGUSR1) == -1)
    {
        perror("error");
        exit(9);
    }
    obsluga_1_wywolanie(sig);
}
void obsluga_3(int sig)
{
    read(pipes[2][0], &syg, sizeof(syg));
    obsluga_1_wywolanie(sig);
}

void proces1()
{
    sigset_t mask, blocker;
    sigfillset(&blocker);
    sigdelset(&blocker, SIGUSR1);
    sigprocmask(SIG_SETMASK, &blocker, NULL);
}
void proces2()
{
    sigset_t mask, blocker;
    sigfillset(&blocker);
    sigdelset(&blocker, SIGUSR1);
    sigprocmask(SIG_SETMASK, &blocker, NULL);
}
void proces3()
{
    sigset_t blocker, mask;
    sigemptyset(&mask);
    sigfillset(&blocker);
    sigdelset(&blocker, SIGTERM);
    sigdelset(&blocker, SIGTSTP);
    sigdelset(&blocker, SIGCONT);
    sigdelset(&blocker, SIGUSR1);
    sigprocmask(SIG_SETMASK, &blocker, NULL);
    signal(SIGTERM, obsluga_3_init);
    signal(SIGCONT, obsluga_3_init);
    signal(SIGSTOP, obsluga_3_init);
    signal(SIGUSR1, obsluga_3);
}
int main(int argc, char *argv[])
{
    p1 = fork();
    if (p1 == 0)
    {
        // Child process p1
        proces1();
    }
    else
    {
        p2 = fork();
        if (p2 == 0)
        {
            // Child process p2
            proces2();
        }
        else
        {
            p3 = fork();
            if (p3 == 0)
            {
                // Child process p3
                proces3();
            }
            else
            {
                // Parent process
                sigemptyset(&mask);
                sigfillset(&blocker);
                sigdelset(&blocker, SIGUSR2);
                sigprocmask(SIG_BLOCK, &blocker, &mask);
                signal(SIGUSR2, obsluga_main_init);
                signal(SIGTERM, obsluga_main);
                signal(SIGTSTP, obsluga_main);
                signal(SIGCONT, obsluga_main);
            }
        }
    }
    return 0;
}