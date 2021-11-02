#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int n = 1;
long double soma;

int main(){
   
    soma = 0;
    int server = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in conn;

    conn.sin_addr.s_addr = inet_addr("192.168.15.5");
    conn.sin_family = AF_INET;
    conn.sin_port = htons(3490);

    bind(server, (struct sockaddr*)&conn, sizeof(conn));

    listen(server, 1);

    struct sockaddr_in client;
    int client_id;
    int client_tam = sizeof(client);
    client_id = accept(server, (struct sockaddr*)&client, &client_tam);

    printf("Client connected at IP: %s and port: %i\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
    
    long double initialize[2];
    long double somaFaixa;
    while (1)
    {
        
        initialize[0] = n;
        n += 10000000;
        initialize[1] = n;
        send(client_id, initialize, sizeof(initialize), 0);
        recv(client_id, &somaFaixa, sizeof(somaFaixa), 0);
        printf("SomaFaixa = %Lf\n", somaFaixa);

    }
    return 0;
}