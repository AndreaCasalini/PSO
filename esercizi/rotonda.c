#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#define N 4     /*numero rami della rotonda*/
#define NMAX 5 /*numero massimo di auto*/

pthread_cond_t enter[N];        /*condition variable per processi che entrano in rotonda*/
pthread_cond_t daiprec[N];      /*condition variable per processi gia in rotonda che danno la precedenza*/
pthread_mutex_t mutex;          /*semaforo binario per la mutua esclusione*/
int content[N];                 /*sospesi in ogni ramo*/
int cont;                       /*numero car in rotonda*/
int sospesidaiprec[N];          /*sospesi per dare la precedenza a chi entra in rotonda*/
int sospesienter[N];            /*sospesi per entrare in rotonda*/

void myInit(){
    pthread_mutex_init(&mutex, NULL);
    for(int i=0;i<N;i++){
        pthread_cond_init(&enter[i], NULL);
        pthread_cond_init(&daiprec[i], NULL);
        content[i]=0;
        sospesidaiprec[i]=0;
        sospesienter[i]=0;
    }
    cont=0;
}

void ingresso(int i){
    pthread_mutex_lock(&mutex);
    if(cont ==NMAX){
        sospesienter[i]++;
        pthread_cond_wait(&enter[i],&mutex);
        sospesienter[i]--;
    }
    cont++;
    content[i]++;
    pthread_mutex_unlock(&mutex);
}

void ruota(int i,int o){
    pthread_mutex_lock(&mutex);
    content[i]--;
    /*vengono segnalti tutti i processi che si sono sospesi e hanno dato la precedenza ai veicoli in transito*/
    if(content[i]==0){
        for(int l=0;l<sospesidaiprec[i];l++){
            pthread_cond_signal(&daiprec[l]);
        }
    }
    int uscitaprima;
    int ingressodopo;
    if (o==0)
        uscitaprima=N-1;
    else
        uscitaprima=o-1;
    if (i==N-1)
        ingressodopo=0;
    else
        ingressodopo=i+1;
    for(int j=ingressodopo;(j%(N-1))!=(uscitaprima%(N-1));){ 
        while(cont<NMAX && content[j]!=0){
            sospesidaiprec[j]++;
            pthread_cond_wait(&daiprec[j],&mutex);
            sospesidaiprec[j]--;
        };
        j++;
        j=j%(N-1);
    }
    pthread_mutex_unlock(&mutex);

}

bool cequalcunoincoda(){
    for (int j=0;j<N;j++){
        if(sospesienter[j]!=0)
            return true;
    }
    return false;
}

void esci(int o){
    pthread_mutex_lock(&mutex);
    cont--;
    if (cont<NMAX && cequalcunoincoda()){
        for(int j=0;j<N;j++){
            int tmp=0;
            //al posto di j mettere (j+o+1)%(N-1) per evitare starvation
            while(cont<NMAX &&tmp<sospesienter[(j+o+1)%(N-1)]){
                tmp++;
                pthread_cond_signal(&enter[(j+o+1)%(N-1)]);
            }
        }
    }
    pthread_mutex_unlock(&mutex);
}

void *Auto(void*id){
    int *pi = (int  *)id;
    int *ptr;
    ptr = (int *)malloc(sizeof(int));
    if (ptr == NULL){
        printf("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }
    int i;
    int o; 
    int k=0;
    while(1){
        /*decido random quale è l'ingresso e quale è l'uscita*/
        i= (rand() % (N)) ;
        o= (rand() % (N)) ; 
        /*arrivo all ingresso della rotatoria*/
        printf("Utente-[Thread%d e identificatore %lu] ENTRO ALL'INGRESSO[%d] E VOGLIO USCIRE ALLA [%d] (iter. %d)\n", *pi, pthread_self(),i,o,k);
        ingresso(i);
        /*entro in rotatoria*/
        printf("Utente-[Thread%d e identificatore %lu] RUOTO (iter. %d)\n", *pi, pthread_self(), k);
        ruota(i,o);
        sleep(5);
        /*esco dalla rotatoria*/
        printf("Utente-[Thread%d e identificatore %lu] ESCO  alla [%d]  (iter. %d)\n", *pi, pthread_self(),o,k);
        esci(o);
        k++;
        sleep(2);/*tempo prima che l'utente rientri in rotatoria*/
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
    myInit();
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