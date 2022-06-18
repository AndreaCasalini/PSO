#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
/*variabili globali*/

/* semaforo di mutua esclusione per l'accesso a tutte le variabili condivise 
(simula il semaforo di mutua esclusiome associato ad una istanza di tipo monitor) */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 


#define V 7    /*numero di vani*/
#define N 4  /*numero bagagli per vano*/

int deposito[V]={0};/*array struttura deposito*/

void to_store(){}
void to_retire(){}

void *user(void*id){
     int *pi = (int  *)id;
     int *ptr;
     ptr = (int *)malloc(sizeof(int));
     if (ptr == NULL)
     {
          printf("Problemi con l'allocazione di ptr\n");
          exit(-1);
     }
     srand(time(NULL));/*inizializzo il seme*/
     /*attribuisco un numero casuale di bagagli ad ogni utente compreso tra 1 e N*/
     int bagagli_utente= (rand() % (N-1)) + 1 ; 
     int vano_used=V;/*essendo presenti V vani contati da 0 a (V-1), V sarebbe un valore non ammissibile*/
     //pthread_mutex_lock(&mutex);
     int i=0;
     while(1){
          /*deposito bagaglio*/
          printf("Utente-[Thread%d e identificatore %lu] ENTRO NEL DEPOSITO PER LASCIARE I [%D] BAGAGLI", *pi, pthread_self(),bagagli_utente);
          to_store();
          /*aspetto*/
          printf("Utente-[Thread%d e identificatore %lu] STO ASPETTANDO PER POI RITIRARE (iter. %d)\n", *pi, pthread_self(), i);
          sleep(6);
          /*ritiro bagaglio*/
          printf("Utente-[Thread%d e identificatore %lu] ESCO DAL DEPOSITO (iter. %d)\n", *pi, pthread_self(), i);
          to_retire();
     }
     while(vano_used==V){
          for (int i=0;i<V;i++){
               /*i bagagli dell utente stanno nel vano i-esimo*/
               if (deposito[i]+bagagli_utente<=N){
                    deposito[i]=deposito[i]+bagagli_utente;
                    vano_used=i;
                    break;
               }
          }
          /*non ho trovato un vano libero per i bagagli quindi devo attendere*/
     }

     //pthread_mutex_unlock(&mutex);
    
}

int main (int argc,char **argv)
{
     pthread_t *thread;
     int *taskids;
     int i;
     int *p;
     int NUM_THREADS = 10;

     

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