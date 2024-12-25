#include <stdio.h>
#include <stdlib.h>
// might be useful
#include <string.h>
#include <time.h>
// fork, pipe, getpid, getppid
#include <unistd.h>
// wait, waitpid
#include <sys/wait.h>
// pid_t, key_t and other types
#include <sys/types.h>
// shmget, shmat, shmdt, shmctl
#include <sys/shm.h>
// semget, semop, semctl
#include <sys/sem.h>
// pthread_create, pthread_join, pthread_self, mutex
#include <pthread.h>
// open idk if needed
#include <fcntl.h>
// signal handling
#include <signal.h>
// error handling
#include <errno.h>
// msget, msgsnd, msgrcv
#include <sys/msg.h>
#include <semaphore.h>

void P2()
{
  printf("\nProcess 2");
  // get data from proc1 -> shm i sem
  char lineFromProc1[1024];
  strcpy(lineFromProc1, "for testing");
  int lineLength = strlen(lineFromProc1);
  printf("\nLine lenght: %i", lineLength);
  // send lineLength to proc3 -> msg
}

int main(int argc, char *argv[])
{
  // wait for all process to finish
  for (int i = 0; i < 3; i++)
  {
    wait(NULL);
  printf("\n");
  return 0;
}
