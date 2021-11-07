#include <gtk/gtk.h> //GTK para construção da interface
#include <glib/gstdio.h> //Bibliotecas de dependências do gtk
#include "Tartarura.h" //Header com a função da tartaruga
#include <pthread.h> //Usado para externalizar o funcionamento da tartaruga da interface, não travando botões

const char * ip = ""; //ip de conexão
int thread = 0; //qnt de threads
pthread_t * tt; //handle da thread de conexão
int rodando = 0; //flag para impedir multiplas conexões
GtkWidget *status; //referência para a barra de status (da interface), definida em activate.

//Decorator da thread (para não travar o botão), roda a tartaruga no ip dado em entry, com a quantidade de threads dada em entry
void * RodaTartaruga(){
  char * localIp = (char*) ip;
  gtk_statusbar_push(GTK_STATUSBAR(status),0,"Conectado");
  Exec(thread, localIp); //Chama a função da tartaruga, travando a thread até terminar
  gtk_statusbar_push(GTK_STATUSBAR(status),0,"Esperando conexão...");
  rodando = 0; //Permite mais conexões
  free(tt); //Previne memory leak
}

//procedimento de callback do botao conectar, só cria uma nova thread se não existir uma
static void Conectar(GtkWidget *widget, gpointer data)
{
  if (rodando == 0)
  {
    tt = malloc( sizeof(pthread_t) );
    pthread_create(tt, NULL, RodaTartaruga, NULL); //Cria a thread para tartaruga e trava conexões
    rodando = 1;
  } 
}

//Dsconecta a tartaruga, sem fechar o servidor
static void Desconectar(GtkWidget *widget, gpointer data)
{
  Cancel(); //Chama a função de cancelamento, alterando a var flag, no escopo do header.
}
/*****************************************************/

//Seta o ip (var de escopo global)
static void SetIP(GtkWidget *widget, gpointer data)
{
  //Recebe o buffer no callback do botão
  ip = gtk_entry_buffer_get_text(data); //Retira o texto de tal buffer
  g_print("IP: %s\n", ip);
}

//seta o thread (var de escopo global), com o int convertido de atoi()
static void SetThread(GtkWidget *widget, gpointer data)
{
  //Recebe o buffer no callback do botão
  const char * thr = gtk_entry_buffer_get_text(data); //Retira o texto de tal buffer
  thread = atoi(thr); //Retira o número de threads da cadeia de char (0 se for letra, etc...)
  g_print("Thread: %d\n", thread);
}
/*****************************************************/

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
/*****************************************************/

//Na ativação da janela do client
static void activate (GtkApplication *app, gpointer user_data)
{
  //Define o  builder para o arq .ui, objetos (retirados do .ui), 
  //e um widget para definição de um label/texto
  GtkBuilder *builder;
  GObject *window, *grid, *ip, *thread, *botao, *botaoCaixa;
  GtkWidget *label;

  //CSS
  GtkCssProvider *css = gtk_css_provider_new();
  gtk_css_provider_load_from_path(css, "ClientCSS.css");
  gtk_style_context_add_provider_for_display(gdk_display_get_default(), GTK_STYLE_PROVIDER(css), GTK_STYLE_PROVIDER_PRIORITY_USER);

  //Pega o arquivo do builder (pré-define todos os componentes da widget e permite o uso do css)
  builder = gtk_builder_new ();
  gtk_builder_add_from_file (builder, "ClientStyle.ui", NULL);

  //Seta a janela (pela definição em builder), e altera tamanho
  window = gtk_builder_get_object (builder, "window");
  gtk_window_set_application (GTK_WINDOW (window), app);


  //Seta o grid (pela definição em builder)
  grid = gtk_builder_get_object (builder, "grid");

  //Seta a barra de texto p/ IP (pela definição em builder)
  ip = gtk_builder_get_object (builder, "Ip");
  //Seta o botão para confirmar o IP (pela definição em builder)
  botao = gtk_builder_get_object (builder, "SetIP");
  GtkEntryBuffer *buf = gtk_entry_get_buffer (GTK_ENTRY (ip)); //Pega o buffer da entrada de texto
  g_signal_connect (botao, "clicked", G_CALLBACK (SetIP), buf); //Ao clicar, manda no callback este buffer

  //Mesmo do acima, só que para o nº de threads
  GridSpaceRow(GTK_WIDGET(grid), 1, 2);
  thread = gtk_builder_get_object (builder, "Thread");
  botao = gtk_builder_get_object (builder, "SetThread");
  buf = gtk_entry_get_buffer (GTK_ENTRY (thread));
  g_signal_connect (botao, "clicked", G_CALLBACK (SetThread), buf);

  //Cria um botão para conectar (também pela definiçã em builder)
  GridSpaceRow(GTK_WIDGET(grid), 3, 8);
  botao = gtk_builder_get_object (builder, "Conectar");
  //Coloca o callback da função Conectar, associado ao clique do botão
  g_signal_connect (botao, "clicked", G_CALLBACK (Conectar), NULL);
  //Mesmo do acima, mas para o botão Desconectar
  botao = gtk_builder_get_object (builder, "Desconectar");
  g_signal_connect (botao, "clicked", G_CALLBACK (Desconectar), NULL);
  //Mesmo do acima, mas para o botão Quit
  GridSpaceRow(GTK_WIDGET(grid), 9, 10);
  botao = gtk_builder_get_object (builder, "Quit");
  //Swapped, pois a chamada do callback será dada para window, já que o callback é de destroy.
  g_signal_connect_swapped (botao, "clicked", G_CALLBACK(gtk_window_destroy), window);

  //Cria um label/texto "Status: ", e coloca no grid
  GridSpaceRow(GTK_WIDGET(grid), 10, 12);
  label = gtk_label_new("Status: ");
  gtk_grid_attach(GTK_GRID(grid), label, 0, 12, 1, 1);

  //Pega a definição da caixa, em builder, e da attach de uma barra de status (escopo global)
  botaoCaixa = gtk_builder_get_object (builder, "Status");
  status = gtk_statusbar_new();
  gtk_statusbar_push(GTK_STATUSBAR(status),0,"Esperando conexão...");
  gtk_box_append (GTK_BOX(botaoCaixa), status);

  //Mstra todos os componentes anteriormente falados.
  gtk_widget_show (GTK_WIDGET (window));
}
/*****************************************************/

int main ()
{

  #ifdef GTK_SRCDIR
    g_chdir (GTK_SRCDIR);
  #endif

  GtkApplication *interface;
  interface = gtk_application_new ("comp.tartaruga.interface", G_APPLICATION_FLAGS_NONE); //Inicializa a applicação no ponteiro de interface
  g_signal_connect (interface, "activate", G_CALLBACK (activate), NULL); //Conecta o "activate" do interface, com um callback para a função activate
  int status = g_application_run (G_APPLICATION (interface), 0, 0); //Inicializa, e fica em loop até fechar
  g_object_unref (interface); //Liberar memoria
  return status;

}
