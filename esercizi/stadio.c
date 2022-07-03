#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#define MAX_C 100   //capienza stadio
#define MAX_G 45    //capienza corridoio
typedef enum{
    italiani, stranieri
}nazionalità;
typedef enum{
    nord, sud
}cancello;
int tifosi_in_stadio;   //numero di tifosi nello stadio
pthread_mutex_t mutex;  //semaforo binario per mutua esclusione
pthread_cond_t coda_in[2][2][MAX_G];    //struttura dati di condition variable per l'ingresso [corridoio][nazionalita][numeroPersone]
pthread_cond_t coda_out[2][2][MAX_G];   //struttura dati di condition variable per l'uscita [corridoio][nazionalita][numeroPersone]
int n_corridoio[2][2][2];             /*numero di persone in ogni corridoio*/
int sospesi_coda_in[2][2][MAX_G];       //struttura dati per contatore sospesi in ingresso [corridoio][nazionalita][numeroPersone]
int sospesi_coda_out[2][2][MAX_G];      //struttura dati per contatore sospesi in uscita [corridoio][nazionalita][numeroPersone]

void myInit(){
    pthread_mutex_init(&mutex, NULL);
    for(int i=0;i<2;i++){
        for(int k=0;k<2;k++){
            for(int s=0;s<2;s++){
                n_corridoio[i][k][s]=0;            //n_corridoio[corridoio][nazionalità][direzione]
            }
        }
    }
    for(int i=0;i<2;i++){
        for(int k=0;k<2;k++){
            for(int s=0;s<MAX_G;s++){
                sospesi_coda_in[i][k][s]=0;
                sospesi_coda_out[i][k][s]=0;
                pthread_cond_init(&coda_in[i][k][s], NULL);     //coda_in[corridoio][nazionalità][num]
                pthread_cond_init(&coda_out[i][k][s], NULL);
            }
        }              
    }
    tifosi_in_stadio=0;
}

int otherN(int tipo){
    if(tipo==italiani)
        return stranieri;
    else
        return italiani;
}

int otherC(int c){
    if (c==nord)
        return sud;
    else
        return nord;
}

bool cecoda_out(int c, int tipo){
    bool flag=false;
    for (int i=MAX_G;i>=0;i--){
        if (sospesi_coda_out[c][tipo][i]>0){
            flag=true;
        }
    }
    return flag;
}
void segnalain(int c, int tipo){
    for(int i=MAX_G;i>0;i--){ 
        pthread_cond_signal(&coda_in[c][tipo][i]);
    }
}
void segnalaout(int c, int tipo){
    for(int i=MAX_G;i>0;i--){ 
        pthread_cond_signal(&coda_out[c][tipo][i]);  
    }
}


void acq_in(int c,int num,int tipo){
    pthread_mutex_lock(&mutex);

    /*se non ci stanno nello stadio,se ce gia un gruppo in uscita di altra nazionalità, 
    se stanno uscendo da stesso corridoio ma di altra nazionalità*/
    while((tifosi_in_stadio+num>MAX_C)|| (n_corridoio[c][otherN(tipo)][1]>0) || cecoda_out(c,otherN(tipo))){
        sospesi_coda_in[c][tipo][num]++;
        pthread_cond_wait(&coda_in[c][tipo][num],&mutex);
        sospesi_coda_in[c][tipo][num]--;
    }
    n_corridoio[c][tipo][0]+=num;
    tifosi_in_stadio+=num;
    pthread_cond_signal(&coda_in[c][tipo][num]);    /*sveglia gli omologhi per numero di componenti*/
    printf("Sono    %lu     sono nell'ingresso  per entrare %d  e sono di nazionalita   %d\n",pthread_self(),c,tipo);

    pthread_mutex_unlock(&mutex);
}

void ril_in(int c,int num,int tipo){
    pthread_mutex_lock(&mutex);

    n_corridoio[c][tipo][0]-=num;
    segnalaout(c,otherN(tipo));
    printf("Sono    %lu   entro in stadio   \n",pthread_self());

    pthread_mutex_unlock(&mutex);
}

void acq_out(int c,int num,int tipo){
    pthread_mutex_lock(&mutex);
    
    //se ci sono gruppi di tipo opposto in direzione opposta 
    while((n_corridoio[c][otherN(tipo)][0]>0)){ 
        //printf("SONO %lu IN ACQOUT WHILE VALE %d\n",pthread_self(),n_corridoio[c][otherN(tipo)][0]);
        sospesi_coda_out[c][tipo][num]++;
        pthread_cond_wait(&coda_out[c][tipo][num],&mutex);
        sospesi_coda_out[c][tipo][num]--;
    }
    n_corridoio[c][tipo][1]+=num;
    tifosi_in_stadio-=num;
    segnalain(c,tipo);
    segnalain(otherC(c),stranieri);
    segnalain(otherC(c),italiani);
    pthread_cond_signal(&coda_out[c][tipo][num]);
    printf("Sono    %lu     sono nell'ingresso per uscire %d  e sono di nazionalita   %d\n",pthread_self(),c,tipo);

    pthread_mutex_unlock(&mutex);
}

void ril_out(int c,int num,int tipo){
    pthread_mutex_lock(&mutex);
    n_corridoio[c][tipo][1]-=num;
    segnalain(c,otherN(tipo));
    printf("Sono    %lu   esco in stadio   \n",pthread_self());

    pthread_mutex_unlock(&mutex);
}

void *Tifosi(void*id){
    int *pi = (int  *)id;
    int *ptr;
    ptr = (int *)malloc(sizeof(int));
    if (ptr == NULL)
    {
        printf("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }
    /*attribuisco un numero casuale di bagagli ad ogni utente compreso tra 1 e N*/
    int c_in;
    int c_out; 
    int num;
    int tipo;

    int k=0;
    while(1){
        /*decisione cancello di ingresso*/
        c_in= (rand() % (2)) ;
        /*decisione cancello di uscita*/
        c_out= (rand() % (2)) ; 
        /*componenti gruppo random*/
        num=(rand()%(MAX_G-1)+1);
        /*decisione random del tipo di gruppo se italiano o straniero*/
        tipo=(rand() % (2)) ; 
        /*arrivo all ingresso dello stadio*/
        printf("Utente-[Thread%d e identificatore %lu] ENTRO ALL'INGRESSO[%d] E SONO DI TIPO [%d] IN [%d]   (iter. %d)\n", *pi, pthread_self(),c_in,tipo,num,k);
        acq_in(c_in,num,tipo);
        sleep(2);
        ril_in(c_in,num,tipo);
        /*entro in stadio*/
        printf("Utente-[Thread%d e identificatore %lu] RIMANE NELLO STADIO                                  (iter. %d)\n", *pi, pthread_self(), k);
        sleep(5);
        /*esco dalla rotatoria*/
        printf("Utente-[Thread%d e identificatore %lu] ESCO  DALLO STADIO DALL USCITA [%d]                  (iter. %d)\n", *pi, pthread_self(),c_out,k);
        acq_out(c_out,num,tipo);
        sleep(2);
        ril_out(c_out,num,tipo);
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
        if (pthread_create(&thread[i], NULL, Tifosi, (void *) (&taskids[i])) != 0)
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