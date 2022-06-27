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
int contatt;        /*elementi in attesa in sala*/  //<<<<---non so se serve
int cap;          /*capacità corrente della sala*/
typedef enum{
    in, out
}dir;

dir direz[2];                 /*2 rappresenta il numero di corridoi*/

int nutenti[2];               /*2 rappresenta il numero di corridoi*/
pthread_mutex_t mutex;
pthread_cond_t attsala;
pthread_cond_t codain[2];     /*2 rappresenta il numero di corridoi*/
pthread_cond_t codaout[2];    /*2 rappresenta il numero di corridoi*/
int attesa_coda_in[2];
int attesa_coda_out[2];

void myInit(){
    for(int i=0;i<2;i++){
        nutenti[i]=0;
        direz[i]=in;
        attesa_coda_in[i]=0;
        attesa_coda_out[i]=0;
        pthread_cond_init(&codain[i], NULL);
        pthread_cond_init(&codaout[i], NULL);
    }
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&attsala, NULL);
    contatt=0;
    cap=0;
}

void INcorrAccesso(int c,int n){
    pthread_mutex_lock(&mutex);
  
    /*controllo se possono entrare se no WAIT corridoio*/
    while(((direz[c]!=in) && (nutenti[c]!=0))||((direz[c]==in)&&((nutenti[c]+n)>MAX))||((direz[c]==in)&&(attesa_coda_out[c]!=0)&&(cap+n>CAPAC))){
        attesa_coda_in[c]++;
        pthread_cond_wait(&codain[c],&mutex);
        attesa_coda_in[c]--;
    }
    cap+=n;
    nutenti[c]+=n;
    direz[c]=in;
    printf("il processo con pid [%lu] entra nel corridoio [%d] e con [%d] componenti del gruppo. NEL CORRIDOIO SONO IN [%d]\n",pthread_self(),c,n,nutenti[c]); 
    pthread_mutex_unlock(&mutex); 
}

void segnalain(int c){
    for(int i =MAX;i!=0;i--){
        int tmp=attesa_coda_in[c];
        while(tmp!=0 && (nutenti[c]+i<=MAX) && attesa_coda_out[c]==0 && (cap+i<=CAPAC)){
            pthread_cond_signal(&codain[c]);
            tmp--;
        }
    }
}

void segnalaout(int c){
    for(int i =MAX;i!=0;i--){
        int tmp=attesa_coda_out[c];
        while(tmp!=0 &&(nutenti[c]+i<=MAX)){
            pthread_cond_signal(&codaout[c]);
            tmp--;
        }
    }
}

void INcorrRilascio(int c, int n){
    pthread_mutex_lock(&mutex);
    nutenti[c]-=n;                /*escano dal corridoio usato per l ingresso*/
    if(nutenti[c]==0)             /*se non ci sono utenti nel corridoio*/
        segnalaout(c);           /*svegliamo quelli in coda per uscire*/
    else
        segnalain(c);            /*se no svegliamo quelli in coda per entrare*/
    pthread_mutex_unlock(&mutex); 
}

void OUTcorrAccesso(int c, int n){
    pthread_mutex_lock(&mutex);
    cap-=n;                       /*svuoto sala del gruppo corrente che esce*/
    //segnala eventualmente all altro corridoio
    for(int i=0;i<contatt;i++){
        pthread_cond_signal(&attsala);          /*sveglio tutti i processi in attesa della sala*/
    }
    while(((direz[c]!=out)&&(nutenti[c]!=0))||((direz[c]==out)&&(nutenti[c]+n>MAX))){ 
        attesa_coda_out[c]++;
        pthread_cond_wait(&codaout[c],&mutex);  /*metto in coda i processi che non trovano il corridoio abbastanza vuoto per attraversarlo*/
        attesa_coda_out[c]--;
    }
    nutenti[c]+=n;                /*faccio entrare il gruppo nel corridoio*/
    direz[c]=out;                 /*imposto la direzione*/
    pthread_mutex_unlock(&mutex); 
}

void OUTcorrRilascio(int c, int n){
    pthread_mutex_lock(&mutex);
    nutenti[c]-=n;                /*faccio uscire dal corridoio*/
    segnalaout(c);
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
    int i=0;
    while(1){
        int n= (rand() % (MAX)) + 1 ;   /*numero utenti nel gruppo*/
        int c=(rand() % (2));           /*numero corridoio scelto*/
        /* entrano*/
        printf("Utente-[Thread%d e identificatore %lu] ENTRIAMO IN N.[%d] NEL CORRIDOIO [%d] (iter. %d)\n", *pi, pthread_self(),n,c,i);
        INcorrAccesso(c,n);
        sleep(1);/*transita*/
        INcorrRilascio(c,n);
        /*aspetto*/
        printf("Utente-[Thread%d e identificatore %lu] ASPETTO                (iter. %d)\n", *pi, pthread_self(), i);
        sleep(3);
        /*escono*/
        printf("Utente-[Thread%d e identificatore %lu] ESCONO   *******       (iter. %d)\n", *pi, pthread_self(), i);
        OUTcorrAccesso(c,n);
        sleep(1);/*transita*/
        OUTcorrRilascio(c,n);
        i++;
        sleep(2);/*tempo prima che l'utente rientri in fila per entrare nella sala*/
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

