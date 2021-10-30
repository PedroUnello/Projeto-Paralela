#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define BILLION 1000000000.0


void tartaruga(int contador, int fim, double *soma);

int main(int argc, char *argv[])
{
  // Inicio do clock
  struct timespec start, end;
  clock_gettime(CLOCK_REALTIME, &start);

  // *****************************************************
  //                    INICIALIZAÇÃO
  // *****************************************************
  double soma = 0;
  int fios = 2;
  int inicio = 1, fim = 1000000000;
  int devide = fios/fim; 

  int thread_count;

// thread_count = strtol(argv[1], NULL, 10);
//#pragma omp parallel num_threads(thread_count)

#pragma omp parallel sections 
{ 
        #pragma omp section nowait  
        { 
                tartaruga(inicio, devide, &soma);
        } 
         
        #pragma omp section nowait 
        { 
                tartaruga(devide+1, fim, &soma);
        } 
}

  // ******************************************************

  //Fim do clock
  clock_gettime(CLOCK_REALTIME, &end);
  double time_spent = (end.tv_sec - start.tv_sec) +
                      (end.tv_nsec - start.tv_nsec) / BILLION;
  printf("Tempo de execução %f ", time_spent);

  printf("Soma = %lf\n", soma);
  return 0;
}

void tartaruga(int contador, int fim, double *soma)
{
  double resultado;

  int idThread = omp_get_thread_num();
  int qtdThreads = omp_get_num_threads();

  while (contador <= fim)
  {
    resultado = 1 /(double)contador;
    /*
    printf("ID: %d | QTD: %d\n",idThread,qtdThreads);
    printf("R: %.14e | C: %d\n",resultado,*contador);
    */
    contador++;

#pragma omp critical
    *soma += resultado;
  }
}
