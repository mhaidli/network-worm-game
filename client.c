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
#define ARRAY_LEN 100*100

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
    int m;

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
        //  valread = read( socket_desc , message, 1024);
        // message[valread] = '\0';   
        // printf("%s", message);
        //printf("Message received!\n");    
        //  printf("%s", message);
          /* int j, k; */
          /* for(j=0; j<50; j++){ */
          /*   for(k=0; k<25;k++){ */
          /*     printf("%d ", client_array[j][k]); */
          /*   } */
          /*   printf("\n"); */
          /* } */
        int targetArray[ARRAY_LEN];

        char *buffer = (char*)targetArray;
        size_t remaining = sizeof(int) * ARRAY_LEN;
        while (remaining) {
          ssize_t recvd = read(socket_desc, buffer, remaining);
          // TODO: check for read errors etc here...
          remaining -= recvd;
          buffer += recvd;
        }
        for (m=0;m<=ARRAY_LEN;m++){
          if (m%100 == 0)
            printf("\n");
          printf(" %d", targetArray[m]);
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
















//Old Crap
/* /\* #include<stdio.h> *\/ */
/* /\* #include<string.h>    //strlen *\/ */
/* /\* #include<sys/socket.h> *\/ */
/* /\* #include<arpa/inet.h> //inet_addr *\/ */
/* /\* #include <errno.h> *\/ */

/* #include <stdio.h> */
/* #include <string.h>   //strlen */
/* #include <stdlib.h> */
/* #include <errno.h> */
/* #include <unistd.h>   //close */
/* #include <arpa/inet.h>    //close */
/* #include <sys/types.h> */
/* #include <sys/socket.h> */
/* #include <netinet/in.h> */
/* #include <sys/time.h>  */
/* //Select and pull to handle simulataneous messaging */

/* #define row 30 */
/* #define col 30 */

/* int main(int argc , char *argv[]) */
/* { */
/*     int socket_desc; */
/*     struct sockaddr_in server; */
/*     char *message_1 , server_reply[2000000]; */
/*     char message[50*100]; */
/*     int max_sd; */
/*     int server_sd, activity, valread; */
/*     int stdin_sd = fileno(stdin); */

/*     //Variables For select */
/*     fd_set readfds; */
    
    
/*     //Create socket */
/*     socket_desc = socket(AF_INET , SOCK_STREAM , 0); */
/*     if (socket_desc == -1) */
/*     { */
/*         printf("Could not create socket"); */
/*     } */

/*     char server_ip[1024]; */
/*     printf("Please enter IP address to connect: "); */
/*     scanf("%s", server_ip); */
    
/*     server.sin_addr.s_addr = inet_addr(server_ip); */
/*     server.sin_family = AF_INET; */
/*     server.sin_port = htons( 8888 ); */
 
/*     //Connect to remote server */
/*     if ((server_sd = connect(socket_desc , (struct sockaddr *)&server , sizeof(server))) < 0) */
/*     { */
/*         puts("connect error"); */
/*         return 1; */
/*     } */
    
/*     puts("Connected\n"); */
     
/*     //Send some data */
/*     //message = "Hello!\n"; */
/*     while(1){ */
  
/*       int client_array[row*col]; */
      
/*       FD_ZERO(&readfds); */
      
/*       //Add stdin */
/*       /\* FD_SET(stdin_sd, &readfds); *\/ */
/*       /\* max_sd = stdin_sd; *\/ */
/*       max_sd = -1; */

/*       FD_SET(socket_desc, &readfds); */
/*       if(socket_desc > max_sd) */
/*         max_sd = socket_desc; */

/*       //wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely */
/*       activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL); */

/*       //error check for select */
/*       if ((activity < 0) && (errno!=EINTR))  */
/*         { */
/*           printf("select error"); */
/*         } */

/*       /\* if (FD_ISSET(stdin_sd, &readfds)){ *\/ */
/*       /\*   //fgets(message, 1024, stdin); *\/ */
/*       /\*   //send(socket_desc , message , strlen(message) , 0); *\/ */
/*       /\*   ; *\/ */
/*       /\* } *\/ */

/*       if (FD_ISSET(socket_desc, &readfds)){ */
/*         //valread = read( socket_desc , client_array, sizeof(int)*row*col); */
/*         //message[valread] = '\0'; */
/*         recv(socket_desc, client_array , sizeof(int)*row*col , 0); */
/*         //printf("Message received!\n");     */
/*         //printf("%s", message); */
/*         int j; */
/*         int k = 0; */
/*         for(j=0; j<(row*col); j++){ */
/*           k++; */
/*           printf("%d ", client_array[j]); */
/*           if(k == row){ */
/*             printf("\n"); */
/*             k = 0; */
/*           } */
/*         } */
/*         //printf("\n\n\nHEYAHEYAHEYA\n\n\n"); */
/*       } */

/*     } */
    
/*     if( send(socket_desc , message , strlen(message) , 0) < 0) */
/*       { */
/*         puts("Send failed"); */
/*         return 1; */
/*     } */
/*     puts("Data Send\n"); */
     
/*     //Receive a reply from the server */
/*     if( recv(socket_desc, server_reply , 2000 , 0) < 0) */
/*     { */
/*         puts("recv failed"); */
/*     } */
/*     puts("Reply received\n"); */
/*     puts(server_reply); */
     
/*     return 0; */
/* } */
