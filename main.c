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

void procFunctions() {

}

int main(int argc, char *argv[])
{
  // assuming that every process is a child process
  pid_t processes[3];

  // creating processes
  // first process
  processes[0] = fork();

  switch (processes[0])
  {
  case -1:
    printf("\nfork() error->proc1");
    return 1;
    break;
  case 0:
    // process 1 -> Kamil
    printf("\nProcess 1");
    exit(0);
    break;

  default:
    // main process
    processes[1] = fork();
    switch (processes[1])
    {
    case -1:
      printf("\nfork() error->proc2");
      return 1;
      break;
    case 0:
      // process 2 -> Mateusz
      printf("\nProcess 2");
      // get data from proc1 -> shm i sem
      char lineFromProc1[1024];
      strcpy(lineFromProc1, "for testing");
      int lineLength = strlen(lineFromProc1);
      printf("\nLine lenght: %i", lineLength);
      // send lineLength to proc3 -> msg?
      exit(0);
      break;
    default:
      // main process
      // creating third process
      processes[2] = fork();
      switch (processes[2])
      {
      case -1:
        printf("\nfork() error->proc3");
        return 1;
        break;
      case 0:
        // process 3 -> Paweł
        printf("\nProcess 3");
        exit(0);
        break;
      default:
        break;
      }
      break;
    }
    break;
  }

  // waitint for all process to finish
  for (int i = 0; i < 3; i++)
  {
    wait(NULL);
  }

  // main menu
  char select;
  while (1)
  {
    printf("\nSelect: ");
    scanf(" %c", &select);
    switch (select)
    {
    case '1':
      printf("\nData from file");
      break;
    case '2':
      printf("\nData from stdin");
      break;
    default:
      printf("\nInvalid choice");
      break;
    }
  }

  printf("\n");
  return 0;
}
