#include <pthread.h>
#include <semaphore.h>

/*variabili globali*/

/* semaforo di mutua esclusione per l'accesso a tutte le variabili condivise 
(simula il semaforo di mutua esclusiome associato ad una istanza di tipo monitor) */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 
/*numero di vani*/
int V = 10;
/*numero bagagli per vano*/
int N = 5;

void *funzione(void*id){
    int *pi = (int  *)id;
    
}

int main ()
{
   pthread_t *thread;
   int *taskids;
   int i;
   int *p;
   int NUM_THREADS = 5;

   thread=(pthread_t *) malloc(NUM_THREADS * sizeof(pthread_t));
   if (thread == NULL)
   {
        printf("Problemi con l'allocazione dell'array thread\n");
        exit(3);
   }
   taskids = (int *) malloc(NUM_THREADS * sizeof(int));
   if (taskids == NULL)
   {
        printf("Problemi con l'allocazione dell'array taskids\n");
        exit(4);
   }

   /* creazione dei thread */
   for (i=0; i < NUM_THREADS; i++)
   {
        taskids[i] = i;
   	    printf("Sto per creare il thread %d-esimo\n", taskids[i]);
	    if (pthread_create(&thread[i], NULL, funzione, (void *) (&taskids[i])) != 0)
                printf("SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskids[i]);
	    printf("SONO IL MAIN e ho creato il Pthread %i-esimo con id=%lu\n", i, thread[i]);
   }

   for (i=0; i < NUM_THREADS; i++)
   {
        int ris;
        pthread_join(thread[i], (void**) & p);
        ris= *p;
        printf("Pthread %d-esimo restituisce %d\n", i, ris);
   }

   exit(0);
}