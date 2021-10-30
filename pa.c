#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define BILLION 1000000000.0

double tartaruga(int fim);

int main(int argc, char *argv[])
{
  // Inicio do clock
  struct timespec start, end;
  clock_gettime(CLOCK_REALTIME, &start);

  // *****************************************************
  //                    INICIALIZAÇÃO
  // *****************************************************
  double soma = 0;
  int inicio = 1, fim = 1000000000;
  int thread_count;

  thread_count = strtol(argv[1], NULL, 10);
  #pragma omp parallel num_threads(thread_count) \
    reduction(+: soma)
  soma += tartaruga( (fim));

  // ******************************************************

  //Fim do clock
  clock_gettime(CLOCK_REALTIME, &end);
  double time_spent = (end.tv_sec - start.tv_sec) +
                      (end.tv_nsec - start.tv_nsec) / BILLION;
  printf("Tempo de execução %f ", time_spent);
  printf("Soma = %lf\n", soma);
  return 0;
}

double tartaruga(int fim)
{


  int idThread = omp_get_thread_num();
  int qtdThreads = omp_get_num_threads();
  double meuCont = 0;
  double contador = 1 + idThread * (fim/qtdThreads);
  int final = (idThread+1) * (fim/qtdThreads);
  while (contador <= final)
  {      
      //printf("ID: %d | C: %d\n",idThread, contador);   
    meuCont += 1/contador;
    contador++;
  }
  return meuCont;
}