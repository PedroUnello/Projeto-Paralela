#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <omp.h>
#include <sys/socket.h> //https://www.educative.io/edpresso/how-to-implement-tcp-sockets-in-c
#include <netinet/in.h> //https://www.gta.ufrj.br/ensino/eel878/sockets/sockaddr_inman.html
#include <arpa/inet.h> //https://www.educative.io/edpresso/how-to-implement-tcp-sockets-in-c
#include <gtk/gtk.h> //GTK para construção da interface
#include <glib/gstdio.h> //Bibliotecas de dependências do gtk

#define MAXIMO 20 //Nº MAXIMOimo de conexões (clientes)
#define INCREMENT 1000000000 //Incremento por step (independente do nº clientes)

//Estrutura Faixa, referente a faixa de calculo atribuida a cada cliente, por step
typedef struct {
    unsigned long long inicio;
    unsigned long long fim;
} Faixa;

//Definições em escopo global
pthread_t* thread_controle; //Handle para thread de controle (rege as faixas)
pthread_t* thread_server;  //Handle para thread do servidor (aceita clients)
pthread_t* thread_handles; //Handles para as threads dos clientes (comunicação server-client)
unsigned long long n = 1; //N do calculo
double soma = 0; //Soma da série
int thread_count = 0; //Quantidade de clientes conectados
long thread = 0; //Indice do ultimo cliente conectado (independente de buracos/desconexões)

GtkWidget * conexao[MAXIMO]; //Componentes (revealer), para cada conexão persente
GObject * wSoma; //Barra de status para demonstrar o valor da soma
int iniciado = 0; //Flag de controle que rege o inicio do servidor

//*era pra ser alocação dinâmica, mas...
//Aloca espaços para calculo, no nº máximo de clientes
Faixa toCalc[MAXIMO]; //Cada espaço é uma faixa para calculo, preenchidos até o indice thread
int qntdCalc = 0; //Quantidade de espaços à calcular restantes (por step), começa em thread, decrementa até 0
int prontoCalc = 0; //flag para controle, similar a um semáforo, para dar tempo de distribuir novas faixas sem interrupção


//Decorator para a thread de controle, coloca faixas novas para calculo
void * RegeFaixa(){ 
    while (iniciado) 
    {
        if (qntdCalc <= 0 && thread_count > 0) //Se tiver clients, e estes já tiverem calculado as faixas
        {
            prontoCalc = 0; //Fecha o calculo
            for (int i = 0; i < thread_count; i++){ 
                //Preenche o vetor de faixas, com incrementos dividos pela quantidade de clients
                toCalc[i].inicio = n + i * ((INCREMENT)/thread_count);
                toCalc[i].fim = n + (i+1) * ((INCREMENT)/thread_count);

            }
            //Quantidade de faixas restantes = nº de clients
            qntdCalc = thread_count;
            n+= INCREMENT;
            prontoCalc = 1; //Abre para calculo novamente
        }
    }

    return NULL;
}

//Decorator para as threads de comunicação com os clients
void * ManageCalc(void *conec){
    //Recebe um array de 2 inteiros 
    int *clientArgs =  (int*) conec; 
    int idCliente = *(clientArgs); //Id/descriptor do socket deste client
    int idCalc = *(clientArgs + 1); //nº/identificador desse client 
    int fixedId = idCalc; //Fixa esse identificador (assim a cada step, o client começa no seu ponto original)
    unsigned long long msg[2] = { toCalc[idCalc].inicio, toCalc[idCalc].fim }; //Prepara a mensagem à enviar pelo socket
    double somaFaixa; //somaLocal (para receber no socket)

    int box; //Indice de um espaço vazio (caixa com faixa atual)
    GtkWidget *caixa = gtk_statusbar_new(); //Cria tal caixa
    unsigned char faixa[200]; //Prepara para receber a faixa
    for (box = 0; box < thread; box++){
        if (gtk_revealer_get_child(GTK_REVEALER(conexao[box])) == NULL){ //Se estiver vazio (revealer)
            gtk_revealer_set_child(GTK_REVEALER(conexao[box]), caixa); //Coloca a caixa dentro
            gtk_revealer_set_reveal_child(GTK_REVEALER(conexao[box]),true); //Mostra a caixa e a barra de status
            break;
        }
    }

    while (iniciado) //Se o client não desconectar, fica em loop
    {
        if (prontoCalc == 1) //Se a flag estiver permitindo calculo
        {
            //Já que 0 não aparece na serie de taylor
            if (msg[0] != 0) //Verifica se não está calculando uma faixa vázia/já calculada
            {
                sprintf(faixa, "%lld", msg[0]); //Coloca a faixa (msg[0]), no char *
                gtk_statusbar_push(GTK_STATUSBAR(caixa), 0, faixa); //Coloca no stack da barra
                send(idCliente, msg, sizeof(msg), 0); //Trava no envio da faixa ao cliente
                //Trava no recebimento do resultado pelo cliente
                if (recv(idCliente, &somaFaixa, sizeof(somaFaixa), 0) == 0){//retorna o nº de bytes lidos, caso sejam 0
                    //O cliente desconectou, logo...
                    shutdown(idCliente, SHUT_RDWR); //Fecha o socket criado
                    gtk_revealer_set_child(GTK_REVEALER(conexao[box]), NULL); //Tira a caixa da faixa
                    gtk_revealer_set_reveal_child(GTK_REVEALER(conexao[box]),false); //Esconde o componente
                    thread_count--; //Diminui a contagem de threads/clients
                    return NULL;
                }
                //Caso tenha recebido algo, verifica se ainda é preciso aquele resultado (ou seja, se já não foi calculado por outro cliente)
                //Além de verificar se a faixa requerida (no vetor), é a mesma que foi calculada pelo client
                else if (toCalc[idCalc].inicio != 0 && msg[0] == toCalc[idCalc].inicio)
                {
#                   pragma omp critical                    
                    soma += somaFaixa; //Soma o calculo local no total
                    char wResultado[200]; //Mesmo que a faixa, só que para a barra de resultado (escopo global)
                    sprintf(wResultado, "%f", soma);
                    gtk_statusbar_push(GTK_STATUSBAR(wSoma), 0, wResultado);
                    //printf("Soma: %f\n", soma); CASO NECESSÁRIO.
                    toCalc[idCalc].inicio = 0; //Anula a faixa (confirmando que já foi calculada)
                    toCalc[idCalc].fim = 0;
#                   pragma omp critical
                    qntdCalc--; //Diminui a quantidade de faixas requeridas
                }
            }
            //Incrementa a posição atual da thread, porém trabalha como um vetor circular, não passando do id do ultimo client
            idCalc++; if (idCalc >= thread) { idCalc = 0; }
            msg[0] = toCalc[idCalc].inicio; //Coloca a mensagem local como a faixa no indice atual
            msg[1] = toCalc[idCalc].fim;
        }
        else { idCalc = fixedId; } //Se a flag não permitir calculo, volta para sua posição inicial (diminuir overlap de clientes)
    }
    return NULL;
}

//Decorator para a thread do servidor, prepara o descriptor, criar sockets quando clients conectam, etc...
void * Servidor(){

    for (int i = 0; i < MAXIMO; i++){//Prepara o vetor de faixas (lida com lixo de memória)
        toCalc[i].inicio = 0;
        toCalc[i].fim = 0;
    }

    //Cria o socket
    int server = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in conn;
    conn.sin_addr.s_addr = inet_addr("192.168.15.5");
    conn.sin_family = AF_INET;
    conn.sin_port = htons(3490); //https://stackoverflow.com/questions/19207745/htons-function-in-socket-programing
    bind(server, (struct sockaddr*)&conn, sizeof(conn));
    listen(server, MAXIMO);

    //Começa a permitir clients, e criar uma thread pra cada
    thread_handles = malloc(sizeof(pthread_t) * MAXIMO);
    struct sockaddr_in client;
    int client_id; //Id do socket criado
    int client_tam;
    int calc[2]; //para envio à thread.
    while (iniciado)
    {
        client_tam = sizeof(client);
        client_id = accept(server, (struct sockaddr*)&client, &client_tam); //Trava até ter uma nova conexão
        printf("Conexao no socket: %s:%i\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port)); 
        calc[0] = client_id;
        calc[1] = thread;
        //Cria uma nova thread para o client
        pthread_create(&thread_handles[thread], NULL, ManageCalc, calc); 
        thread_count++;
        thread++;
    }
    return NULL;
}

//Pula (finalR - startR) linhas em um grid dado
static void GridSpaceRow(GtkWidget *grid, int startR, int finalR){
  int i = startR;
  GtkWidget *space;
  for (i; i < finalR; i++){
    space = gtk_label_new(""); //label é um texto, neste caso, vazio
    gtk_grid_attach(GTK_GRID(grid),space,0,i,1,1); //Coloca um texto vazio no grid.
  }
}

//Pula (finalR - startR) colunas em um grid dado
static void GridSpaceColumn(GtkWidget *grid, int startC, int finalC){
  int i = startC;
  GtkWidget *space;
  for (i; i < finalC; i++){
    space = gtk_label_new("");
    gtk_grid_attach(GTK_GRID(grid),space,i,0,1,1);
  }
}

static void Iniciar(GtkWidget *widget, gpointer data){
    if (iniciado == 0) //Caso não tenha iniciado
    {
        iniciado = 1;
        //Cria uma thread para controle do servidor + aceitar clients
        thread_server = malloc(sizeof(pthread_t));
        pthread_create(thread_server, NULL, Servidor, NULL);
        //Cria uma thread de controle do vetor de calculo
        thread_controle = malloc(sizeof(pthread_t));
        pthread_create(thread_controle, NULL, RegeFaixa, NULL);
    }
}

//Na ativação da janela do servidor
static void activate (GtkApplication *app, gpointer user_data)
{

    //Define o  builder para o arq .ui, objetos (retirados do .ui)
    GtkBuilder *builder;
    GObject *window, *grid, *opcoes;
    GtkWidget *botao;
    

    //CSS
    GtkCssProvider *css = gtk_css_provider_new();
    gtk_css_provider_load_from_path(css, "ServerCSS.css");
    gtk_style_context_add_provider_for_display(gdk_display_get_default(), GTK_STYLE_PROVIDER(css), GTK_STYLE_PROVIDER_PRIORITY_USER);

    //Pega o arquivo do builder (pré-define todos os componentes da widget e permite o uso do css)
    builder = gtk_builder_new ();
    gtk_builder_add_from_file (builder, "ServerStyle.ui", NULL);

    //Seta a janela (pela definição em builder), e altera tamanho
    window = gtk_builder_get_object (builder, "window");
    gtk_window_set_application (GTK_WINDOW (window), app);
    
    //Seta um grid de conexões (pela defini....)
    grid = gtk_builder_get_object (builder, "Grid");

    //Seta uma caixa de opções
    GridSpaceRow(GTK_WIDGET(grid),0,2);
    opcoes = gtk_builder_get_object(builder, "Opcoes");
    gtk_box_set_baseline_position(GTK_BOX(opcoes), GTK_BASELINE_POSITION_TOP);
    //Coloca os botões na caixa
    botao = gtk_button_new_with_label("Iniciar");
    g_signal_connect (botao, "clicked", G_CALLBACK (Iniciar), NULL);
    gtk_box_append (GTK_BOX (opcoes), botao);
    botao = gtk_button_new_with_label("Quit");
    g_signal_connect_swapped (botao, "clicked", G_CALLBACK(gtk_window_destroy), window);
    gtk_box_append (GTK_BOX (opcoes), botao);

    //Seta uma barra de status para soma
    GridSpaceRow(GTK_WIDGET(grid),3,4);
    wSoma = gtk_builder_get_object(builder, "Soma");

    //Coloca espaços Revealer para as conexoes (default: invisiveis)
    GridSpaceRow(GTK_WIDGET(grid),6,8);    
    for (int i = 0; i < MAXIMO; i++){
        conexao[i] = gtk_revealer_new(); //Preenche com um widget revealer
        gtk_grid_attach(GTK_GRID(grid),conexao[i],(i%5), 2*(i%4) + 8,1,2); //Coloca tal no grid
    }

    //Mstra todos os componentes anteriormente falados.
    gtk_widget_show (GTK_WIDGET (window));
}

int main(){
    
    #ifdef GTK_SRCDIR
        g_chdir (GTK_SRCDIR);
    #endif

    GtkApplication *interface;
    interface = gtk_application_new ("comp.server.interface", G_APPLICATION_FLAGS_NONE); //Inicializa a applicação no ponteiro de interface
    g_signal_connect (interface, "activate", G_CALLBACK (activate), NULL); //Conecta o "activate" do interface, com um callback para a função activate
    int status = g_application_run (G_APPLICATION (interface), 0, 0); //Inicializa, e fica em loop até fechar
    g_object_unref (interface); //Liberar memoria
    //Liberar mais memória...
    for (long j = 0; j < thread; j++){
        pthread_join(thread_handles[j], NULL);
    }

    free(thread_handles);
    free(thread_controle);
    free(thread_server);

    return status;
}
