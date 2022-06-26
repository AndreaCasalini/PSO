#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define CAPAC 10    /*capacità stanza*/
#define MAX 3       /*capacità corridoio*/


int corridoio;      /*numero di corridoio 1 o 2*/
int gruppo;         /*numero di persone che compongono il gruppo da 1 a MAX*/

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t attsala;

//int codain;         /*elementi in coda in ingresso*/
//int codaout;        /*elementi in coda in uscita*/
int contatt;        /*elementi in attesa in sala*/  //<<<<---non so se serve
int cap=0;          /*capacità corrente della sala*/
typedef enum{
    in, out
}dir;

dir direz[];
int nutenti[];
pthread_cond_t codain[2];
pthread_cond_t codaout[2];  /*2 rappresenta il numro di corridoi*/
int attesa_coda_in=0;
int attesa_coda_out=0;

void INcorrAccesso(int c,int n){
     pthread_mutex_lock(&mutex);
     while(cap+n>CAPAC){
          contatt++;
          pthread_cond_wait(&attsala, &mutex);
          contatt--;
     }
     cap+=n;
     while(((direz[c]!=in) && (nutenti[c]!=0))||((direz[c]==in)&&((nutenti+n)>MAX))||((direz[c]==in)&&(attesa_coda_out!=0))){
          attesa_coda_in++;
          pthread_cond_wait(&codain[c],&mutex);
          attesa_coda_in--;
     }
     nutenti[c]+=n;
     direz[c]=in;
     pthread_mutex_unlock(&mutex); 
}

void segnalain(int c){
     for(int i =MAX;i>0;i--){
          while(attesa_coda_in!=0 &&(nutenti[c]+1<=MAX)&& attesa_coda_out==0){
               pthread_cond_signal(&codain[c]);
          }
     }
}

void segnalaout(int c){
     for(int i =MAX;i>0;i--){
          while(attesa_coda_out!=0 &&(nutenti[c]+1<=MAX)&& attesa_coda_in==0){
               pthread_cond_signal(&codaout[c]);
          }
     }
}

void INcorrRilascio(int c, int n){
     pthread_mutex_lock(&mutex);
     nutenti[c]-=n;
     if(nutenti[c]==0)
          segnalaout(c);
     else
          segnalain(c);
     pthread_mutex_unlock(&mutex); 
}


void *utente(void*id){
     int *pi = (int  *)id;
     int *ptr;
     ptr = (int *)malloc(sizeof(int));
     if (ptr == NULL)
     {
          printf("Problemi con l'allocazione di ptr\n");
          exit(-1);
     }
     /*attribuisco un numero casuale di bagagli ad ogni utente compreso tra 1 e N*/
     int n= (rand() % (MAX)) + 1 ;  /*numero utenti nel gruppo*/
     int c=0;                       /*corridoio*/
     int i=0;
     while(1){
          /* entrano*/
          printf("Utente-[Thread%d e identificatore %lu] ENTRIAMO IN N.[%d]     (iter. %d)\n", *pi, pthread_self(),n_persone,i);
          INcorAccesso(c,n);
          INcorRilascio(c,n);
          /*aspetto*/
          printf("Utente-[Thread%d e identificatore %lu] ASPETTO                (iter. %d)\n", *pi, pthread_self(), i);
          sleep(5);
          /*escono*/
          printf("Utente-[Thread%d e identificatore %lu] ESCONO                 (iter. %d)\n", *pi, pthread_self(), i);
          OUTcorAccesso(c,n);
          OUTcorRilascio(c,n);
          i++;
          sleep(2);/*tempo prima che l'utente rientri nel deposito per depositare altri bagagli*/
     }
      
}


int main (int argc,char **argv)
{
     pthread_t *thread;
     int *taskids;
     int i;
     int *p;
     if (argc != 2){
          printf("Errore nel numero di parametri (%d)\n", argc - 1);
          exit(1);
     }
     int NUM_THREADS = atoi(argv[1]);
     if (NUM_THREADS <= 0){
          printf("Errore: primo parametro deve essere maggiore di zero\n");
          exit(2);
     }
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
          if (pthread_create(&thread[i], NULL, utente, (void *) (&taskids[i])) != 0)
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

