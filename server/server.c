#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
 
int main(){
 
  char *ip = "10.1.4.198";
  int port = 5566;
 
  int server_sock, client_sock;
  struct sockaddr_in server_addr, client_addr;
  socklen_t addr_size;
  char buffer[1024];
  int n;
 
  server_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (server_sock < 0){
    perror("[-]Socket error");
    exit(1);
  }
  printf("[+]TCP server socket created.\n");
 
  memset(&server_addr, '\0', sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = port;
  server_addr.sin_addr.s_addr = inet_addr(ip);
 
  n = bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
  if (n < 0){
    perror("[-]Bind error");
    exit(1);
  }
  printf("[+]Bind to the port number: %d\n", port);
 
  listen(server_sock, 5);
  printf("Listening...\n");
 
  while(1){
    addr_size = sizeof(client_addr);
    client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_size);
    printf("[+]Client connected.\n");   
 
    bzero(buffer, 1024);
    recv(client_sock, buffer, sizeof(buffer), 0);
    if (strcmp(buffer, "RPIClientConnect") != 0){
        printf("Illigal Client Connected, disconnecting!\n");
        close(client_sock);
    }
    else{
        while(1){

            bzero(buffer, sizeof(buffer));
            printf("Enter a string : ");
            n = 0;
            while ((buffer[n++] = getchar()) != '\n');
            printf("Server: %s\n", buffer);
            send(client_sock, buffer, strlen(buffer), 0);

            if (strcmp(buffer, "exit\n") == 0){
                close(client_sock);
                break;
            }
            else if (strcmp(buffer, "reg\n") == 0){
                bzero(buffer, 1024);
                recv(client_sock, buffer, sizeof(buffer), 0);
                if (strcmp(buffer, "ack") == 0){
                    bzero(buffer, sizeof(buffer));
                    printf("Enter an Index to store finger in: ");
                    n = 0;
                    while ((buffer[n++] = getchar()) != '\n');
                    printf("Server: %s\n", buffer);
                    send(client_sock, buffer, strlen(buffer), 0);
                }
            }
            else if (strcmp(buffer, "delete\n") == 0){
                bzero(buffer, 1024);
                recv(client_sock, buffer, sizeof(buffer), 0);
                if (strcmp(buffer, "ack") == 0){
                    bzero(buffer, sizeof(buffer));
                    printf("Enter an Index to delete finger from: ");
                    n = 0;
                    while ((buffer[n++] = getchar()) != '\n');
                    printf("Server: %s\n", buffer);
                    send(client_sock, buffer, strlen(buffer), 0);
                }
            }

        }

        printf("[+]Client disconnected.\n\n");
    }
  }
  return 0;
}
