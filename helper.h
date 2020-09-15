/******************************************************************
 * Header file for the helper functions. This file includes the
 * required header files, as well as the function signatures and
 * the semaphore values (which are to be changed as needed).
 ******************************************************************/


# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/ipc.h>
# include <sys/shm.h>
# include <sys/sem.h>
# include <sys/time.h>
# include <math.h>
# include <errno.h>
# include <string.h>
# include <pthread.h>
# include <ctype.h>
# include <iostream>
using namespace std;

# define SEM_KEY 0x45 // Change this number as needed

#define NON_POSITIVE_INTEGER		1
#define INCORRECT_NUMBER_OF_PARAMETERS  2
#define NO_ERROR		  	0

union semun {
  int val;               /* used for SETVAL only */
  struct semid_ds *buf;  /* used for IPC_STAT and IPC_SET */
  ushort *array;         /* used for GETALL and SETALL */
};

int check_arg (char *);
int sem_create (key_t, int);
int sem_init (int, int, int);
void sem_wait (int, short unsigned int);
void sem_signal (int, short unsigned int);
int sem_close (int);
int sem_timed_wait (int id, short unsigned int num, int delay);

void semget_err(int err);
void sem_init_err(int err);
void semctl_err(int err);
void pthread_create_err(int err);
void pthread_join_err(int err);
