#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <sys/socket.h> //https://www.educative.io/edpresso/how-to-implement-tcp-sockets-in-c
#include <netinet/in.h> //https://www.gta.ufrj.br/ensino/eel878/sockets/sockaddr_inman.html
#include <arpa/inet.h> //https://www.educative.io/edpresso/how-to-implement-tcp-sockets-in-c


#define BILLION 1000000000.0

int flag; //Flag de parada (execução)

double tartaruga(unsigned long long inicio, unsigned long long fim);

void Cancel(){ //Procedimento de cancelamento da execução (para ser chamada pelo client)
    flag = 0; 
}

//Faz o calculo enquanto estiver recebendo mensagens e flag == 1
void Exec(int threads, char * ip) //ip + nº threads
{
  flag = 1;

  // *****************************************************
  //                    INICIALIZAÇÃO
  // *****************************************************
  double soma = 0; //Soma local
  int thread_count = threads;

  //Cria o socket para o servidor (No ip dado)
  int server = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in conn;
  conn.sin_addr.s_addr = inet_addr(ip);
  conn.sin_family = AF_INET;
  conn.sin_port = htons(3490); //https://stackoverflow.com/questions/19207745/htons-function-in-socket-programing
  int status = connect(server, (struct sockaddr*)&conn, sizeof(conn));
  printf("\nConnected = %d", status);
  if (status < 0) { shutdown(server, SHUT_RDWR); return; } //Caso não conecte, simplesmente termina a execução

  unsigned long long receive[2]; //Estrutura para o recebimento
  double encodeMsg;
  while (flag)
  {
    recv(server, receive, sizeof(receive), 0); //Trava a execução até receber algo
    //printf("\nFaixa a calcular = %lld ate %lld", receive[0], receive[1]); Caso necessário

    soma = 0;
    //Utiliza do openMP para fazer o calculo da série, em {thread_count} seções concorrentes
    #pragma omp parallel num_threads(thread_count) \
        reduction(+: soma)
    soma += tartaruga( (receive[0]), (receive[1]));

    encodeMsg = soma; //Prepara a mensagem (double)
    send(server, &encodeMsg, sizeof(encodeMsg), 0); //Trava a execução até confirmação do recebimento pelo servidor
  }
  shutdown(server, SHUT_RDWR); //Quando terminar a execução, fecha o socket criado
  // ******************************************************
}

double tartaruga(unsigned long long inicio, unsigned long long fim)
{
  int idThread = omp_get_thread_num();
  int qtdThreads = omp_get_num_threads();
  double minhaSoma = 0; //Soma local da thread
  //Divide a faixa à calcular em seções referentes a cada thread
  //Contador = inicio (da thread)
  unsigned long long contador = inicio + idThread * ((fim - inicio)/qtdThreads);
  //Final = final da seção distribuida
  unsigned long long final = inicio + (idThread+1) * ((fim - inicio)/qtdThreads);
  while (contador < final)
  {
    double numerador = 1.0; //Necessário para conversão do unsigned long long para double        
    minhaSoma += numerador / contador;
    contador++;
  }
  return minhaSoma;
}