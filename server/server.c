#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 50

void empty_file(){
  /*FILE *f1 = fopen("instruction", "w");
  fprintf(f1, "");
  fclose(f1);
  FILE *f2 = fopen("index", "w");
  fprintf(f2, "");
  fclose(f2);*/
  fclose(fopen("instruction", "w"));
  fclose(fopen("index", "w"));
}
 
int main(){
  empty_file();

  char *ip = "0.0.0.0";
  int port = 50505;
 
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
  server_addr.sin_port = htons(port);
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
  	char buff[BUFFER_SIZE];
        while(1){
	    bzero(buff, sizeof(buff));
	    FILE *f = fopen("instruction", "r");
  	    fgets(buff, BUFFER_SIZE, f);
	    if (strlen(buff) <= 0){
		//same as before
		sleep(1);
	    }
	    else{
		printf("String read: %s Size: %d\n", buff, strlen(buff));
		fclose(f);
		
            	bzero(buffer, sizeof(buffer));
		strcpy(buffer, buff);

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
                    		bzero(buff, sizeof(buff));
				FILE *f = fopen("index", "r");
  	    			fgets(buff, BUFFER_SIZE, f);
  	    			printf("Index Read: %s\n", buff);
  	    			fclose(f);
				fclose(fopen("index", "w")); 
                    		send(client_sock, buff, strlen(buff), 0);
                	}
            	}
            	else if (strcmp(buffer, "delete\n") == 0){
                	bzero(buffer, 1024);
                	recv(client_sock, buffer, sizeof(buffer), 0);
                	if (strcmp(buffer, "ack") == 0){
				bzero(buff, sizeof(buff));
				FILE *f1 = fopen("index", "r");
  	    			fgets(buff, BUFFER_SIZE, f1);
  	    			printf("Index Read: %s\n", buff);
  	    			fclose(f1);
				fclose(fopen("index", "w")); 
                    		send(client_sock, buff, strlen(buff), 0);
			}
                }
	    sleep(1);
	    empty_file();
            }

        }
        printf("[+]Client disconnected.\n\n");
    }
  }
  return 0;
}
