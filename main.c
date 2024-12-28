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

#define BUFF_S 1024
int sid;
key_t key = 0x283;
key_t key_msg = 0x284;

// struktura danych do kolejek komunikatów
struct msgbuf
{
  long mtype;
  int mdata;
};

void P1(int semid)
{
  int wybor;
  char *tekst = (char *)shmat(sid, NULL, 0);
  if (tekst == (void *)-1)
  {
    perror("Blad shmat");
    exit(EXIT_FAILURE);
  }

  while (1)
  {
    printf("\nWybierz \n1: Wczytywanie elementów z klawiatury, \n2: Wczytywanie z pliku\nWybor: ");
    scanf(" %d", &wybor);

    // Usunięcie znaku nowej linii pozostającego w buforze po scanf
    getchar();

    switch (wybor)
    {
    case 1:
      while (1)
      {
        // czekanie az bedzie mozna wpisac dane:
        struct sembuf waitP1 = {0, -1, 0};
        if (semop(semid, &waitP1, 1) == -1)
        {
          perror("Blad semop - oczekiwanie P1");
          exit(EXIT_FAILURE);
        }
        // wprowadzenie danych przez uzytkownika
        printf("\nP1: Wpisz tekst: ");
        fgets(tekst, BUFF_S, stdin);
        // Usunięcie znaku nowej linii z końca
        tekst[strcspn(tekst, "\n")] = 0;
        if (strncmp(tekst, ".", 1) == 0)
        {
          // zwolnienie semafora P1
          struct sembuf signalP1 = {0, 1, 0};
          if (semop(semid, &signalP1, 1) == -1)
          {
            perror("Blad semop - sygnalizacja P1");
            exit(EXIT_FAILURE);
          }
          break;
        }
        // zwolnienie semafora P2
        struct sembuf signalP2 = {1, 1, 0};
        if (semop(semid, &signalP2, 1) == -1)
        {
          perror("Blad semop - sygnalizacja P2");
          exit(EXIT_FAILURE);
        }
      }
      break;
    case 2:
      char fileName[256];
      printf("Podaj nazwe pliku: ");
      scanf(" %s", fileName);
      fileName[sizeof(fileName) - 1] = '\0';

      FILE *sourceFile = fopen(fileName, "r");
      if (!sourceFile)
      {
        perror("Blad otwierania pliku");
        exit(EXIT_FAILURE);
      }
      char fromFile[BUFF_S];
      while (fgets(fromFile, sizeof(fromFile), sourceFile))
      {
        // czekanie az bedzie mozna wpisac dane:
        struct sembuf waitP1 = {0, -1, 0};
        if (semop(semid, &waitP1, 1) == -1)
        {
          perror("Blad semop - oczekiwanie P1");
          exit(EXIT_FAILURE);
        }
        // przepisanie z fromFile do pamieci wspoldzielonej
        strcpy(tekst, fromFile);
        // usuniecie \n z konca nie wiem czy potrzebne ale tak jest bardziej czytelne bo nie liczby \n jako znaku
        tekst[strcspn(tekst, "\n")] = 0;
        // zwolnienie semafora P2
        struct sembuf signalP2 = {1, 1, 0};
        if (semop(semid, &signalP2, 1) == -1)
        {
          perror("Blad semop - sygnalizacja P2");
          exit(EXIT_FAILURE);
        }
      }
      // czekanie az bedzie mozna wpisac dane:
      // może wydawać się bez sensu bo w sumie tak jest, ale wtedy zaczeka aż zakończy się najpierw P3
      struct sembuf waitP1 = {0, -1, 0};
      if (semop(semid, &waitP1, 1) == -1)
      {
        perror("Blad semop - oczekiwanie P1");
        exit(EXIT_FAILURE);
      }
      struct sembuf signalP1 = {0, 1, 0};
      if (semop(semid, &signalP1, 1) == -1)
      {
        perror("Blad semop - sygnalizacja P1");
        exit(EXIT_FAILURE);
      }

      break;
    default:
      printf("Nieprawidlowy wybor.\n");
      break;
    }
  }

  shmdt(tekst);
}

void P2(int semid, int msgid)
{
  // wskaznik na pamiec wspoldzelona
  char *tekst = (char *)shmat(sid, NULL, 0);
  while (1)
  {
    // otrzymanie danych przez samafor
    struct sembuf waitP2 = {1, -1, 0};
    if (semop(semid, &waitP2, 1) == -1)
    {
      perror("Blad semop - waitP2");
      exit(EXIT_FAILURE);
    }
    // docelowe obczlienie długości
    printf("\nP2: %s", tekst);
    fflush(stdout);
    int lineLength = strlen(tekst);
    // przeslanie danych uzywajac kolejki komunikatow
    struct msgbuf fromP2;
    fromP2.mdata = lineLength;
    fromP2.mtype = 1;
    // data - dane do przeslania
    // type - 1 -> zostanie dodany do kolejki
    // przeslanie do P3
    if (msgsnd(msgid, &fromP2, sizeof(fromP2.mdata), 0) == -1)
    {
      perror("Blad msgsnd");
      exit(EXIT_FAILURE);
    }
  }
  shmdt(tekst);
}

void P3(int msgid, int semid)
{
  // odczytanie danych z kolejki komunikatow
  while (1)
  {
    struct msgbuf fromP2;
    if (msgrcv(msgid, &fromP2, sizeof(fromP2.mdata), 1, 0) == -1)
    {
      perror("Blad msgrcv");
      exit(EXIT_FAILURE);
    }
    printf("\nP3: Dlugosc: %i", fromP2.mdata);
    // fflush - wymusza natychmiastowy wydruk na ekranie?
    fflush(stdout);
    // przekazanie informacji do p1, że może pracować dalej
    // !! ważne -> na razie tutaj, ponieważ jeżeli dam to od razu po przesłaniu przez msg w P2 to i tak najpierw wykona się to, przez co jeszcze przed wypisaniem tego zdąży wykonać się kod z P1, przez co dziwne rzeczy się dzieją?, ale zgodznie z zadaniem nie jest napisane że tak nie można, a ja nie mam innego pomysłu
    struct sembuf signalP1 = {0, 1, 1};
    if (semop(semid, &signalP1, 1) == -1)
    {
      perror("Blad semop - sygnalizacja P1");
      exit(EXIT_FAILURE);
    }
  }
}

int main(int argc, char *argv[])
{
  // potrzebne do -semaforow
  //
  sid = shmget(key, BUFF_S, IPC_CREAT | 0666);
  if (sid == -1)
  {
    perror("Blad shmget");
    exit(EXIT_FAILURE);
  }
  // utworzenie 2 semaforow
  int semid = semget(key, 2, IPC_CREAT | 0666);
  if (semid == -1)
  {
    perror("Blad semget");
    exit(EXIT_FAILURE);
  }
  // semafor dla p1 - z ustawiana wartoscia 1 - od razu może zacząć pracować
  if (semctl(semid, 0, SETVAL, 1) == -1)
  {
    perror("Blad semctl ustawienie semafora P1");
    exit(EXIT_FAILURE);
  }
  // semafor dla p2 - z ustawiona wartoscia 0 - musi poczekać aż zmieni się jego wartość na > 0
  if (semctl(semid, 1, SETVAL, 0) == -1)
  {
    perror("Blad semctl ustawienie semafora P2");
    exit(EXIT_FAILURE);
  }
  //
  // potrzebne do komunikatow kolejek
  //
  // utworzenie kolejki komunikatów
  int msgid = msgget(key_msg, 0666 | IPC_CREAT);
  if (msgid == -1)
  {
    perror("Blad msgget");
    exit(EXIT_FAILURE);
  }
  // tworzenie procesów
  pid_t processes[3];

  processes[0] = fork();
  switch (processes[0])
  {
  case -1:
    printf("\nfork() error->proc1");
    return 1;
  case 0:
    P1(semid);
    exit(0);
  default:
    break;
  }

  processes[1] = fork();
  switch (processes[1])
  {
  case -1:
    printf("\nfork() error->proc2");
    return 1;
  case 0:
    P2(semid, msgid);
    exit(0);
  default:
    break;
  }

  processes[2] = fork();
  switch (processes[2])
  {
  case -1:
    printf("\nfork() error->proc3");
    return 1;
  case 0:
    P3(msgid, semid);
    exit(0);
  default:
    break;
  }

  // waitint for all process to finish
  for (int i = 0; i < 3; i++)
  {
    wait(NULL);
  }

  // Clean up resources
  shmctl(sid, IPC_RMID, NULL);
  semctl(semid, 0, IPC_RMID, NULL);
  // usuwanie kolejki
  msgctl(msgid, IPC_RMID, NULL);

  return 0;
}
