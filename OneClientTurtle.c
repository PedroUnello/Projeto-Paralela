#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define BILLION 1000000000.0

long double tartaruga(long double inicio, long double fim);

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
  conn.sin_addr.s_addr = inet_addr("192.168.15.5");
  conn.sin_family = AF_INET;
  conn.sin_port = htons(3490);
  printf("\nConnected = %d", connect(server, (struct sockaddr*)&conn, sizeof(conn)));

  long double receive[2];
  long double inicio, fim;
  long double encodeMsg;
  while (1)
  {
    recv(server, receive, sizeof(receive), 0);
    printf("\nFaixa a calcular = %Lf ate %Lf", receive[0], receive[1]);
    inicio = receive[0];   
    fim = receive[1];

    soma = 0;
    #pragma omp parallel num_threads(thread_count) \
        reduction(+: soma)
    soma += tartaruga( (inicio), (fim));

    //Fim do clock
    clock_gettime(CLOCK_REALTIME, &end);
    double time_spent = (end.tv_sec - start.tv_sec) +
                      (end.tv_nsec - start.tv_nsec) / BILLION;
    printf("\nTempo de execução %f ", time_spent);

    encodeMsg = soma;
    send(server, &encodeMsg, sizeof(encodeMsg), 0);
  }
  // ******************************************************
  return 0;
}

long double tartaruga(long double inicio, long double fim)
{
  int idThread = omp_get_thread_num();
  int qtdThreads = omp_get_num_threads();
  long double minhaSoma = 0;
  long double contador = inicio + idThread * ((fim - inicio)/qtdThreads);
  long double final = inicio + (idThread+1) * ((fim - inicio)/qtdThreads);
  while (contador < final)
  {        
    minhaSoma += 1/contador;
    contador++;
  }
  return minhaSoma;
}