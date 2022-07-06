#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define P 4         /*numero pompe*/
#define L 1000      /*numero litri disponibili dal benzinaio*/

int benzdisp;       /*benzina disponibile*/
int pompedisp;      /*pompe disponibili*/
int sospesi;        /*numero automobili sospese*/
int sospesiU;        /*numero automobili sospese urgenti con rischio starvation*/

pthread_cond_t codaAM;  /*coda automobili*/
pthread_cond_t codaAMU;  /*coda automobili urgenti*/

pthread_cond_t codaAB;  /*coda autobotte*/
pthread_mutex_t mutex;
int sospesaAB;      /*sospensione autobotte*/

void myInit(){
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&codaAM, NULL);
    pthread_cond_init(&codaAMU, NULL);
    pthread_cond_init(&codaAB, NULL);
    sospesaAB=0;
    sospesi=0;
    sospesiU=0;
    benzdisp=L;
    pompedisp=P;
}

void Richiedi(int l){
    pthread_mutex_lock(&mutex);
    if(l>((L/100)*80)){
        while(pompedisp==0 || benzdisp<l || sospesaAB !=0 ){
            sospesiU++;
            pthread_cond_wait(&codaAMU,&mutex);
            sospesiU--;
        }
    }
    else{
        while(pompedisp==0 || benzdisp<l || sospesaAB !=0 ){
            sospesi++;
            pthread_cond_wait(&codaAM,&mutex);
            sospesi--;
        }
    }
    /*acquisizione delle risorse*/
    pompedisp--;
    benzdisp-=l;
    //printf("Capacità attuale %d\n",benzdisp);
    pthread_mutex_unlock(&mutex); 
}

void Rilascia(){
    pthread_mutex_lock(&mutex);
    /*rilascio delle risorse*/
    pompedisp++;
    if (pompedisp==P){
        /*risveglio autobotte*/
        pthread_cond_signal(&codaAB);
    }
    int su=sospesiU;
    for(int i=0;i<su;i++)
        pthread_cond_signal(&codaAMU);
    int s=sospesi;
    for(int i=0;i<s;i++)
        pthread_cond_signal(&codaAM);

    pthread_mutex_unlock(&mutex); 
}

void Rifornisci(){
    pthread_mutex_lock(&mutex);
    if(pompedisp<P){
        /*ci sono automobili che stanno facendo benzina*/
        sospesaAB=1;
        pthread_cond_wait(&codaAB,&mutex); /*prima sospendo*/
        sospesaAB=0;
        /*quandovengo rilasciato*/
    }
    benzdisp=L;
    //printf("AUTOBOTTE HA RIFORNITO\n");
    /*risveglio automobili in coda*/
    if (sospesiU!=0){
        pthread_cond_signal(&codaAMU);
    }
    else{
        int s=sospesi;
        for(int i=0;i<s;i++)
            pthread_cond_signal(&codaAM); 
    }

    pthread_mutex_unlock(&mutex); 
}

void *autobotte(void*id){
    int *pi = (int  *)id;
    int *ptr;
    ptr = (int *)malloc(sizeof(int));
    if (ptr == NULL){
        printf("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }
    int i=0;
    while(1){
        printf("AUTOBOTTE-[Thread%d e identificatore %lu] ARRIVA (iter. %d)\n", *pi, pthread_self(),i);
        Rifornisci();
        printf("AUTOBOTTE-[Thread%d e identificatore %lu] HA RIFORNITO (iter. %d)\n", *pi, pthread_self(),i);
        i++;
        sleep(10);
    }
}

void *automobile(void*id){
    int *pi = (int  *)id;
    int *ptr;
    ptr = (int *)malloc(sizeof(int));
    if (ptr == NULL){
        printf("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }
    /*attribuisco un numero casuale di bagagli ad ogni utente compreso tra 1 e N*/
    int i=0;
    while(1){
        int l= (rand() % (1000)) + 1 ;   /*numero litri richiesti dal veicolo*/
        /* entrano*/
        printf("Automobile-[Thread%d e identificatore %lu] RICHIEDO [%d] LITRI (iter. %d)\n", *pi, pthread_self(),l,i);
        Richiedi(l);
        /*aspetto*/
        printf("Automobile-[Thread%d e identificatore %lu] ASPETTO              (iter. %d)\n", *pi, pthread_self(), i);
        sleep(3);
        /*escono*/
        printf("Automobile-[Thread%d e identificatore %lu] RILASCIO             (iter. %d)\n", *pi, pthread_self(), i);
        Rilascia(l);
        /*tempo prima che l'utente rientri in fila per entrare dalbenzinaio*/
        i++;
        sleep(2);
    }
}


int main (int argc,char **argv)
{
    pthread_t *thread1;
    pthread_t *thread2;
    int *taskids1;
    int *taskids2;
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

    thread1=(pthread_t *) malloc(NUM_THREADS * sizeof(pthread_t));
    thread2=(pthread_t *) malloc(1 * sizeof(pthread_t));            /*un solo processo per l'autobotte*/

    if (thread1 == NULL || thread2 == NULL)
    {
        printf("Problemi con l'allocazione degli array thread\n");
        exit(3);
    }
    taskids1 = (int *) malloc(NUM_THREADS * sizeof(int));
    taskids2 = (int *) malloc(1 * sizeof(int));

    if (taskids1 == NULL || taskids2 ==NULL)
    {
        printf("Problemi con l'allocazione degli array taskids\n");
        exit(4);
    }
    myInit();
    /* creazione dei thread */
    taskids2[0]=0;
    printf("Sto per creare il thread_autobotte %d-esimo\n", taskids2[0]);
    if(pthread_create(&thread2[0], NULL, autobotte, (void *) (&taskids2[0])) != 0){
                printf("SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskids2[0]);
    }
    printf("SONO IL MAIN e ho creato il Pthread-autobotte %i-esimo con id=%lu\n", 0, thread2[0]);


    for (i=0; i < NUM_THREADS; i++)
    {
        taskids1[i] = i;
        printf("Sto per creare il thread_automobile %d-esimo\n", taskids1[i]);
        if (pthread_create(&thread1[i], NULL, automobile, (void *) (&taskids1[i])) != 0)
                printf("SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskids1[i]);
        printf("SONO IL MAIN e ho creato il Pthread %i-esimo con id=%lu\n", i, thread1[i]);
    }

    for (i=0; i < NUM_THREADS; i++)
    {
        int ris;
        pthread_join(thread1[i], (void**) & p);
        ris= *p;
        printf("Pthread_automobile %d-esimo restituisce %d\n", i, ris);
    }
    pthread_join(thread2[0], (void**) & p);
    int ris= *p;
    printf("Pthread_autobotte %d-esimo restituisce %d\n", i, ris);
    exit(0);
}