#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 

#define BILLION 1000000000.0

double tartaruga(unsigned long long inicio, unsigned long long fim);

int main(int argc, char *argv[])
{
  // *****************************************************
  //                    INICIALIZAÇÃO
  // *****************************************************
  double soma = 0;
  int thread_count;
  thread_count = strtol(argv[1], NULL, 10);
  int server = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in conn;
  conn.sin_addr.s_addr = inet_addr("192.168.15.5");
  conn.sin_family = AF_INET;
  conn.sin_port = htons(3490);
  printf("\nConnected = %d", connect(server, (struct sockaddr*)&conn, sizeof(conn)));
  unsigned long long receive[2];
  unsigned long long inicio, fim;
  double encodeMsg;
  while (1)
  {
    recv(server, receive, sizeof(receive), 0);
    printf("\nFaixa a calcular = %lld ate %lld", receive[0], receive[1]);
    inicio = receive[0];   
    fim = receive[1];
    soma = 0;
    #pragma omp parallel num_threads(thread_count) \
        reduction(+: soma)
    soma += tartaruga( (inicio), (fim));
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
  while (contador < final)
  {
    double numerador = 1.0;        
    minhaSoma += numerador / contador;
    contador++;
  }
  return minhaSoma;
}