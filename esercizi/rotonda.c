#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define CMAX 10 /*numero massimo di auto nella rotonda*/
#define N 4     /*numero rami della rotonda*/
#define NMAX 20 /*numero massimo di auto*/


pthread_cond_t enter[N];  /*coda */
pthread_cond_t daiprec[N];  /*coda */
pthread_mutex_t mutex;
int content[N];     /*sospesi in ogni ramo*/
int cont;           /*numero car in rotonda*/
int sospesidaiprec;

void myInit(){
    pthread_mutex_init(&mutex, NULL);
    for(int i=0;i<N;i++){
        pthread_cond_init(&enter[i], NULL);
        pthread_cond_init(&daiprec[i], NULL);
        content[i]=0;
    }
    cont=0;
    sospesidaiprec=0;
}

void Ingresso(int i){
    pthread_mutex_lock(&mutex);
    if(cont ==NMAX){
        pthread_cond_wait(&enter[i],&mutex);
    }
    cont++;
    content[i]++;
    pthread_mutex_unlock(&mutex);
}

void Ruota(int i,int o){
    pthread_mutex_lock(&mutex);
    content[i]--;
    if(content[i]==0){
        for(int l=0;l<sospesidaiprec;l++){
            pthread_cond_signal(&daiprec);
        }
    }
    for(int j=i;i<o;i++){  //METTERE A POSTO
        while(cont<NMAX && content[i]!=0){
            sospesidaiprec++;
            pthread_cond_wait(&daiprec[i],&mutex);
            sospesidaiprec--;
        }
    }
    pthread_mutex_unlock(&mutex);

}

void Esci(int o){
    
}

void *Auto(void*id){
    int *pi = (int  *)id;
    int *ptr;
    ptr = (int *)malloc(sizeof(int));
    if (ptr == NULL)
    {
        printf("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }
    /*attribuisco un numero casuale di bagagli ad ogni utente compreso tra 1 e N*/
    int i;
    int o; 

    int k=0;
    while(1){
        i= (rand() % (N)) ;
        o= (rand() % (N)) ; 
        /*cap_vano bagaglio*/
        printf("Utente-[Thread%d e identificatore %lu] ENTRO ALL'INGRESSO[%d] E VOGLIO USCIRE ALLA [%d] (iter. %d)\n", *pi, pthread_self(),i,o,k);
        Ingresso(i);
        /*aspetto*/
        printf("Utente-[Thread%d e identificatore %lu] RUOTO (iter. %d)\n", *pi, pthread_self(), k);
        Ruota(i,o);
        sleep(5);
        /*ritiro bagaglio*/
        printf("Utente-[Thread%d e identificatore %lu] ESCO  alla [%d]  (iter. %d)\n", *pi, pthread_self(),o,k);
        Esci(o);
        k++;
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
        if (pthread_create(&thread[i], NULL, Auto, (void *) (&taskids[i])) != 0)
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