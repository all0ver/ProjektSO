#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/shm.h>
#define BUFF_S 1024

FILE *nowy;
pid_t Proces1;
sem_t *sem1;
sem_t *sem2;
key_t key=0x283;
void P1(char *argv[]) { 
    int wybor;
    //tworzymy segment pamieci dzielonej
    int sid = shmget(key, BUFF_S, IPC_CREAT | 0666);
    if (sid == -1) {
        perror("Blad shmget");
        exit(EXIT_FAILURE);
    }
    //dolaczamy przestrzen dzielona do pamieci adresowej
    char *tekst = (char*) shmat(sid, NULL, 0);
    if (tekst == (void*) -1) {
        perror("Blad shmat");
        exit(EXIT_FAILURE);
    }
    printf("Wybierz 1: Wczytywanie elementów z klawiatury, 2: Wczytywanie z pliku\n");
    scanf("%d", &wybor);
    getchar(); 

    switch (wybor) {
      //wczytujemy z klawiatury
        case 1:
            while (1) {
                printf("Wpisz tekst: ");
                //pobieramy z stdin znaki
                fgets(tekst, BUFF_S, stdin);
                if (strncmp(tekst, "exit", 4) == 0) {
                    
                    sem_post(sem2);
                    sem_wait(sem1); 
                    break;
                }
                //dajemy znać procesowi 2 że może działać
                sem_post(sem2); 

                sem_wait(sem1); 
            }
            break;

        case 2: {
            char nazwa_pliku[256];
            strncpy(nazwa_pliku, argv[1], sizeof(nazwa_pliku) - 1);
            nazwa_pliku[sizeof(nazwa_pliku) - 1] = '\0';
            
            FILE *plik = fopen(nazwa_pliku, "r");
            //obsluga błędóœ
            if (!plik) {
                perror("Blad otwierania pliku");
                exit(EXIT_FAILURE);
            }
            while (fgets(tekst, BUFF_S, plik)) {
               //dajemy znać procesowi 2 że może działać
                sem_post(sem2);  
                sem_wait(sem1); 
            }
            fclose(plik);

            sem_post(sem2); 
            break;
        }

        default:
            printf("Nieprawidlowy wybor.\n");
            break;
    }

    //odlaczenie i skasowanie pamieci dzielonej
    shmdt(tekst); 
    shmctl(sid, IPC_RMID, 0); 
}


int main(int argc, char * argv[]) {
  //musimy tu zadeklarować sepaphory 3 na początku jeszcze
    return 0;
}
