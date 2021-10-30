#include <stdio.h>
#include <time.h>    
#include <omp.h>

#define BILLION  1000000000.0

void tartaruga(int inicio, int fim, double *soma);

int main(int argc, char * argv[])
{
  // Inicio do clock
  struct timespec start, end;
  clock_gettime(CLOCK_REALTIME, &start);


  
    // *****************************************************
    //                    INICIALIZAÇÃO
    // *****************************************************
    double soma;
    int inicio, fim;
    int thread_count;

    thread_count = strtol(argv[1], NULL, 10);
    # pragma omp parallel num_threads(thread_count)
    tartaruga(inicio,fim,&soma);

    // ******************************************************
    

    //Fim do clock
    clock_gettime(CLOCK_REALTIME, &end);
    double time_spent = (end.tv_sec - start.tv_sec) +
                        (end.tv_nsec - start.tv_nsec) / BILLION;
    printf("\n\nTempo de execução %f ", time_spent);
    return 0;
}

void tartaruga(int inicio, int fim, double *soma)
{
  double somaLocal;
  int contador = inicio;

  int idThread = omp_get_thread_num();
  int qtdThreads = omp_get_num_threads();

  while (contador <= fim)
  {
    if (contador%qtdThreads == idThread)
    {
      somaLocal = 1/contador;
    }
    
    contador++;
  }
  
#pragma omp critical
  *soma += somaLocal;
}