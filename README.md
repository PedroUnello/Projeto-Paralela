# Projeto-Paralela

## Detalhes do ambiente:

* gcc (Ubuntu 10.3.0-1ubuntu1) 10.3.0

## Compilação e execução:

<ol>
  <li> Main.c 
  * O parâmetro passado para a execução é a quantidade de threads.

  ```
  gcc Main.c -o Main -fopenmp -lm
  ./Main 4
  ```
  ## Obs

  * Os arquivos OneClientTurtle.c e OneServerTurtle.c são parte de uma tentativa de possibilitar a execução da tartaruga em múltiplas máquinas ao mesmo tempo.
  </l1>
  
  <br><li> Estrutura de Rede (computação distribuida) -> ClientTurtle.c e ServerTurtle.c
  
  ```
  gcc {nome do arquivo} -o {nome do executavel} -fopenmp -lpthread -lm
  ./{nome do executavel}
  ```
  ## Obs

  * O comando de compilação deve ser feito para todos os arquivos.c
  * Deve ser feito a troca do ip em ClientTurtle.c e ServidorTurtle.c
    
  </li>
  
  <br><li> Interface -> Pasta GTK (e seus conteúdos)
  
  ```
  gcc $( pkg-config --cflags gtk4 ) -o {nome do executavel} {nome do arquivo} $( pkg-config --libs gtk4 ) -fopenmp -lpthread -lm
  ./{nome do executavel}
  ```
   ## Obs

  * O comando de compilação deve ser feito para todos os arquivos.c
  * Deve ser feito a troca do ip em ServidorGTK.c para o local
  * Para compilar a interface GTK, deve ser feita a instalação do GTK e suas dependências
  * Todos os arquivos .ui e .css são necessários para correto funcionamento da aplicação.
    
  </li>
  
</ol>

## Resultado:

Resultado obtido após 30 minutos utilizando a estrutura de rede sem a interface GTK.

![image](https://user-images.githubusercontent.com/66622436/143435133-4c0653f7-18c3-44cf-abbd-f1bf6be7a45e.png)
