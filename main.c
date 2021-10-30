#include <stdio.h>
#include <time.h>    
#include <omp.h>
#define BILLION  1000000000.0

void tartaruga(double soma, int inicio, int fim);

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
    tartaruga(soma,inicio,fim);

    return 0;
    // ******************************************************
    

    //Fim do clock
    clock_gettime(CLOCK_REALTIME, &end);
    double time_spent = (end.tv_sec - start.tv_sec) +
                        (end.tv_nsec - start.tv_nsec) / BILLION;
    printf("\n\nTempo de execução %f ", time_spent);
    return 0;
}
