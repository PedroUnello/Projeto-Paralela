#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <sys/socket.h> //https://www.educative.io/edpresso/how-to-implement-tcp-sockets-in-c
#include <netinet/in.h> //https://www.gta.ufrj.br/ensino/eel878/sockets/sockaddr_inman.html
#include <arpa/inet.h> //https://www.educative.io/edpresso/how-to-implement-tcp-sockets-in-c


#define BILLION 1000000000.0

double tartaruga(unsigned long long inicio, unsigned long long fim);

int main(int argc, char *argv[])
{
  // Inicio do clock
  struct timespec start, end;
  clock_gettime(CLOCK_REALTIME, &start);

  // *****************************************************
  //                    INICIALIZAÇÃO
  // *****************************************************
  double soma = 0;
  int thread_count;
  thread_count = strtol(argv[1], NULL, 10);


  int server = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in conn;
  conn.sin_addr.s_addr = inet_addr("192.168.15.12");
  conn.sin_family = AF_INET;
  conn.sin_port = htons(3490); //https://stackoverflow.com/questions/19207745/htons-function-in-socket-programing
  printf("\nConnected = %d", connect(server, (struct sockaddr*)&conn, sizeof(conn)));

    //int msg[2] = {nFaixa.inicio, nFaixa.fim};
    //char receive[2000];
    //memset(receive, '\0', sizeof(receive));
    //int encodeMsg = htonl(msg);
    //printf("%ld", recv(server, receive, sizeof(receive), 0));
    //printf("%s",receive);

  unsigned long long receive[2];
  unsigned long long inicio, fim;
  double encodeMsg;
  while (1)
  {
    recv(server, receive, sizeof(receive), 0);
    printf("\nFaixa a calcular = %lld ate %lld", receive[0], receive[1]);
    inicio = receive[0];   
    fim = receive[1];

    /*
    //char server_message[512];
    //memset(server_message, '\0', sizeof(server_message));
    //recv(server, server_message, sizeof(server_message), 0);
    int receive[2];
    recv(server, receive, sizeof(receive), 0);
    int inicio = receive[0];
    int fim = receive[1];
    */

    soma = 0;
    #pragma omp parallel num_threads(thread_count) \
        reduction(+: soma)
    soma += tartaruga( (inicio), (fim));

    //Fim do clock
    clock_gettime(CLOCK_REALTIME, &end);
    double time_spent = (end.tv_sec - start.tv_sec) +
                      (end.tv_nsec - start.tv_nsec) / BILLION;
    //printf("\nTempo de execução %f ", time_spent);
    //printf("soma: %Lf", soma);
    encodeMsg = soma;
    send(server, &encodeMsg, sizeof(encodeMsg), 0);
  }
  // ******************************************************
  return 0;
}

double tartaruga(unsigned long long inicio, unsigned long long fim)
{
  int idThread = omp_get_thread_num();
  int qtdThreads = omp_get_num_threads();
  double minhaSoma = 0;
  unsigned long long contador = inicio + idThread * ((fim - inicio)/qtdThreads);
  unsigned long long final = inicio + (idThread+1) * ((fim - inicio)/qtdThreads);
  printf("\nContador %d inicia em: %lld e termina em: %lld", idThread,contador, final);
  while (contador < final)
  {
    double numerador = 1.0;        
    minhaSoma += numerador / contador;
    //printf("\nsoma: %f", numerador/ contador);
    contador++;
  }
  return minhaSoma;
}