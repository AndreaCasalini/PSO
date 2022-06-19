#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define V 2  /*numero di vani*/
#define N 4  /*numero bagagli per vano*/

/*variabili globali*/
/* semaforo di mutua esclusione per l'accesso a tutte le variabili condivise 
(simula il semaforo di mutua esclusiome associato ad una istanza di tipo monitor) */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t coda;
int counter=0;/*contatore di utenti in coda in attesa di risveglio*/
int deposito[V]={0};/*array struttura deposito*/

int to_store(int bagagli_utente){ /*ritorno il vano in cui mettiamo i bagagli*/
     pthread_mutex_lock(&mutex);
     while(1){
          for(int i=0;i<V;i++){
               if(deposito[i]+bagagli_utente<=N){
                    deposito[i]+=bagagli_utente;
                    printf("INSERISCO BAGAGLIO NEL VANO %d e sono il thread con id %lu\n",i,pthread_self());
                    pthread_mutex_unlock(&mutex);
                    return i;
               }
          }
          /*non ho trovato un vano libero per i bagagli quindi devo attendere*/
          counter++;
          printf("-->sono [%lu] e sono bloccato\n",pthread_self());
          pthread_cond_wait(&coda, &mutex);
          counter--;
     }
}

void to_retire(int vano_utente, int bagagli_utente){
     pthread_mutex_lock(&mutex);

     deposito[vano_utente]-=bagagli_utente;
     for(int i=0;i<counter;i++)
          pthread_cond_signal(&coda);
     
     pthread_mutex_unlock(&mutex);
}

void *user(void*id){
     int *pi = (int  *)id;
     int *ptr;
     ptr = (int *)malloc(sizeof(int));
     if (ptr == NULL)
     {
          printf("Problemi con l'allocazione di ptr\n");
          exit(-1);
     }
     /*attribuisco un numero casuale di bagagli ad ogni utente compreso tra 1 e N*/
     int bagagli_utente= (rand() % (N)) + 1 ; 
     int i=0;
     while(1){
          /*deposito bagaglio*/
          printf("Utente-[Thread%d e identificatore %lu] ENTRO CON N.[%d] BAGAGLI (iter. %d)\n", *pi, pthread_self(),bagagli_utente,i);
          int n_vano = to_store(bagagli_utente);
          /*aspetto*/
          printf("Utente-[Thread%d e identificatore %lu] ASPETTO (iter. %d)\n", *pi, pthread_self(), i);
          sleep(5);
          /*ritiro bagaglio*/
          printf("Utente-[Thread%d e identificatore %lu] ESCO  (iter. %d)\n", *pi, pthread_self(), i);
          to_retire(n_vano,bagagli_utente);
          i++;
          sleep(2);
     }
      
}

int main (int argc,char **argv)
{
     pthread_t *thread;
     int *taskids;
     int i;
     int *p;
     int NUM_THREADS = 10;

     srand(time(NULL));/*inizializzo il seme*/

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
          if (pthread_create(&thread[i], NULL, user, (void *) (&taskids[i])) != 0)
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