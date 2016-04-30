/* #include<stdio.h> */
/* #include<string.h>    //strlen */
/* #include<sys/socket.h> */
/* #include<arpa/inet.h> //inet_addr */
/* #include <errno.h> */

#include <stdio.h>
#include <string.h>   //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>   //close
#include <arpa/inet.h>    //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> 
//Select and pull to handle simulataneous messaging


int main(int argc , char *argv[])
{
    int socket_desc;
    struct sockaddr_in server;
    char *message_1 , server_reply[2000];
    char message[1024];
    int client_array[50][25];
    int max_sd;
    int server_sd, activity, valread;
    int stdin_sd = fileno(stdin);

    //Variables For select
    fd_set readfds;
    
    
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }

    char server_ip[1024];
    printf("Please enter IP address to connect: ");
    scanf("%s", server_ip);
    
    server.sin_addr.s_addr = inet_addr(server_ip);
    server.sin_family = AF_INET;
    server.sin_port = htons( 8888 );
 
    //Connect to remote server
    if ((server_sd = connect(socket_desc , (struct sockaddr *)&server , sizeof(server))) < 0)
    {
        puts("connect error");
        return 1;
    }
    
    puts("Connected\n");
     
    //Send some data
    //message = "Hello!\n";
    while(1){
  
      FD_ZERO(&readfds);
      
      //Add stdin
      FD_SET(stdin_sd, &readfds);
      max_sd = stdin_sd;

      FD_SET(socket_desc, &readfds);
      if(socket_desc > max_sd)
        max_sd = socket_desc;

      //wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely
      activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);

      //error check for select
      if ((activity < 0) && (errno!=EINTR)) 
        {
          printf("select error");
        }

      if (FD_ISSET(stdin_sd, &readfds)){
        fgets(message, 1024, stdin);
        send(socket_desc , message , strlen(message) , 0);
      }

      if (FD_ISSET(socket_desc, &readfds)){
        valread = read( socket_desc , client_array, sizeof(int)*50*25);
        //message[valread] = '\0';
        printf("Message received!\n");    
        //printf("%s", message);
          int j, k;
          for(j=0; j<50; j++){
            for(k=0; k<25;k++){
              printf("%d ", client_array[j][k]);
            }
            printf("/n");
          }
      }

    }
    
    if( send(socket_desc , message , strlen(message) , 0) < 0)
    {
        puts("Send failed");
        return 1;
    }
    puts("Data Send\n");
     
    //Receive a reply from the server
    if( recv(socket_desc, server_reply , 2000 , 0) < 0)
    {
        puts("recv failed");
    }
    puts("Reply received\n");
    puts(server_reply);
     
    return 0;
}
