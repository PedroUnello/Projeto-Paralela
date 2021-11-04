# Projeto-Paralela

## Detalhes do ambiente:

* gcc (Ubuntu 10.3.0-1ubuntu1) 10.3.0

## Compilação e execução:

* O parâmetro passado para a execução é a quantidade de threads.

```
gcc Main.c -o Main -fopenmp -lm
./Main 4
```
## Obs

* Os arquivos OneClientTurtle.c e OneServerTurtle.c são parte de uma tentativa de possibilitar a execução da tartaruga em múltiplas máquinas ao mesmo tempo.