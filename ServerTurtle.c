#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <omp.h>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <arpa/inet.h> 

#define MAX 50
#define INCREMENT 1000000000

typedef struct {
    unsigned long long inicio;
    unsigned long long fim;
} Faixa;

unsigned long long n = 1;
double soma = 0;
int thread_count = 0;
long thread = 0;


Faixa toCalc[MAX];
int qntdCalc = 0;
int prontoCalc = 0;

void * RegeFaixa(){
    while (1)
    {
        if (qntdCalc <= 0 && thread_count > 0) 
        {
            prontoCalc = 0;
            for (int i = 0; i < thread_count; i++){
                toCalc[i].inicio = n + i * ((INCREMENT)/thread_count);
                toCalc[i].fim = n + (i+1) * ((INCREMENT)/thread_count);

            }
            qntdCalc = thread_count;
            n+= INCREMENT;
            prontoCalc = 1;
        }
    }

    return NULL;
}

void * ManageCalc(void *conec){

    int *clientArgs =  (int*) conec; 
    int idCliente = *(clientArgs);
    int idCalc = *(clientArgs + 1);
    int fixedId = idCalc;
    unsigned long long msg[2] = { toCalc[idCalc].inicio, toCalc[idCalc].fim };
    double somaFaixa;
    while (1)
    {
        if (prontoCalc == 1)
        {
            if (msg[0] != 0) 
            { 
                send(idCliente, msg, sizeof(msg), 0);
                if (recv(idCliente, &somaFaixa, sizeof(somaFaixa), 0) == 0){
                    shutdown(idCliente, SHUT_RDWR);
                    thread_count--;
                    return NULL;
                }       
                else if (toCalc[idCalc].inicio != 0 && msg[0] == toCalc[idCalc].inicio)
                {
#                   pragma omp critical                    
                    soma += somaFaixa;
                    printf("Soma = %f\n", soma);
                    toCalc[idCalc].inicio = 0;
                    toCalc[idCalc].fim = 0;
#                   pragma omp critical
                    qntdCalc--;
                }
            }

            idCalc++; if (idCalc >= thread) { idCalc = 0; }
            msg[0] = toCalc[idCalc].inicio;
            msg[1] = toCalc[idCalc].fim;
        }
        else { idCalc = fixedId; }
    }
    return NULL;
}


int main(){
    
    //Inicializa o vetor de calculo
    int mC = MAX;
    for (int i = 0; i < mC; i++){
        toCalc[i].inicio = 0;
        toCalc[i].fim = 0;
    }

    //Cria o socket
    int server = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in conn;
    conn.sin_addr.s_addr = inet_addr("192.168.15.5");
    conn.sin_family = AF_INET;
    conn.sin_port = htons(3490);
    bind(server, (struct sockaddr*)&conn, sizeof(conn));
    listen(server, mC);

    //Cria uma thread de controle do vetor de calculo
    pthread_t* thread_controle = malloc(sizeof(pthread_t));
    pthread_create(thread_controle, NULL, RegeFaixa, NULL);

    //Começa a permitir clients, e criar uma thread pra cada
    pthread_t* thread_handles = malloc(sizeof(pthread_t) * mC);
    struct sockaddr_in client;
    int client_id;
    int client_tam;
    int calc[2];
    //THREADS for multiple connections;
    while (1)
    {
        client_tam = sizeof(client);
        client_id = accept(server, (struct sockaddr*)&client, &client_tam);
        printf("Conexao no socket: %s:%i\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port)); 
        calc[0] = client_id;
        calc[1] = thread_count;
        pthread_create(&thread_handles[thread_count], NULL, ManageCalc, calc); 
        thread_count++;
        thread++;
    }

    for (thread = 0; thread < thread_count; thread++){
        pthread_join(thread_handles[thread], NULL);
    }

    free(thread_handles);
    free(thread_controle);

    return 0;
}
