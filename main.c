#include <stdio.h>
#include <time.h>    

 
#define BILLION  1000000000.0
 

int main()
{
    // Inicio do clock
    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);

  
    // start code
      printf("teste de codigo");
      for(int i = 0; i < 10000; i++){
        printf("");
      }
    // end code


    //Fim do clock
    clock_gettime(CLOCK_REALTIME, &end);
    double time_spent = (end.tv_sec - start.tv_sec) +
                        (end.tv_nsec - start.tv_nsec) / BILLION;
    printf("\n\nTempo de execução %f ", time_spent);
    return 0;
}
