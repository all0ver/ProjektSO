#include <stdio.h>
#include <stdlib.h>
// might be useful
#include <string.h>
#include <time.h>
// fork, pipe, getpid, getppid
#include <unistd.h>
// wait, waitpid
#include <sys/wait.h>
//pid_t, key_t and other types
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
// extra idk if needed
// messages queues -> lab10
/*#include <sys/msg.h>*/

int main(int argc, char *argv[]) {
  printf("%s", argv[1]);

  printf("\n");
  return 0;
}
