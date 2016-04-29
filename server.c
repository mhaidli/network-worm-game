/* #include<stdio.h> */
/* #include<string.h>    //strlen */
/* #include<stdlib.h>    //strlen */
/* #include<sys/socket.h> */
/* #include<arpa/inet.h> //inet_addr */
/* #include<unistd.h>    //write */
/* #include<pthread.h> //for threading , link with lpthread */

/* #include <errno.h> */
/* #include <unistd.h>   //close */
/* #include <sys/types.h> */
/* #include <netinet/in.h> */
/* #include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros */


/* void *connection_handler(void *); */
/* //void* reply_to_client_func(void* socket_desc); */
/* void *reply_handler(void* socket_desc); */

/* int main(int argc , char *argv[]) */
/* { */
/*     int socket_desc , new_socket , c , *new_sock; */
/*     struct sockaddr_in server , client; */
/*     char *message; */

/*     //For select */
/*     int max_sd; */
/*     fd_set readfds; */
/*     int client_socket[2] = {0}; */
    
    
/*     int yes = 1; */
/*     //Create socket */
/*     socket_desc = socket(AF_INET , SOCK_STREAM , 0); */
/*     if (socket_desc == -1) */
/*     { */
/*         printf("Could not create socket"); */
/*     } */
     
/*     //Prepare the sockaddr_in structure */
/*     server.sin_family = AF_INET; */
/*     server.sin_addr.s_addr = INADDR_ANY; */
/*     server.sin_port = htons( 8888 ); */

/*     //This code is to allow the port to be reused */
/*     if (setsockopt(socket_desc,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) { */
/*       perror("setsockopt error"); */
/*       exit(1); */
/*     }   */
    
/*     //Bind */
/*     if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0) */
/*     { */
/*         puts("bind failed"); */
/*         return 1; */
/*     } */
/*     puts("bind done"); */

/*     //try to specify maximum of 3 pending connections for the master socket */
/*     if (listen(socket_desc, 3) < 0) */
/*     { */
/*         perror("listen"); */
/*         exit(EXIT_FAILURE); */
/*     } */
     
/*     //Accept and incoming connection */
/*     puts("Waiting for incoming connections..."); */
/*     c = sizeof(struct sockaddr_in); */
/*     /\* pthread_t sniffer_thread; *\/ */
/*     /\* pthread_t replying_thread; *\/ */
    
/*     while( (new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) ) */
/*     { */
/*         puts("Connection accepted"); */
         
/*         //Reply to the client */
/*         message = "Hello Client , I have received your connection. And now I will assign a handler for you\n"; */
/*         write(new_socket , message , strlen(message)); */
         
/*         //pthread_t sniffer_thread; */
/*         new_sock = malloc(1); */
/*         *new_sock = new_socket; */
         
/*         /\* if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0) *\/ */
/*         /\* { *\/ */
/*         /\*     perror("could not create thread"); *\/ */
/*         /\*     return 1; *\/ */
/*         /\* } *\/ */
                      
/*         //pthread_t replying_thread; */
/*         /\* if( pthread_create( &replying_thread , NULL ,  reply_handler , (void*) new_sock) < 0) *\/ */
/*         /\*   { *\/ */
/*         /\*     perror("could not create thread"); *\/ */
/*         /\*     return 1; *\/ */
/*         /\*   } *\/ */

         
/*         //Now join the thread , so that we dont terminate before the thread */
/*         //pthread_join( sniffer_thread , NULL); */
/*         puts("Handler assigned"); */
/*     } */

/*     if (new_socket<0) */
/*     { */
/*         perror("accept failed"); */
/*         return 1; */
/*     } */
/*     /\* pthread_join(sniffer_thread, NULL); *\/ */
/*     /\* pthread_join(replying_thread, NULL); *\/ */

/*     char data[1024]; */
/*     recv(socket_desc, data, sizeof(data), 0); */
/*     return 0; */
/* } */





/**
   Handle multiple socket connections with select and fd_set on Linux
     
   Silver Moon ( m00n.silv3r@gmail.com)
*/
  
#include <stdio.h>
#include <string.h>   //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>   //close
#include <arpa/inet.h>    //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
  
#define TRUE   1
#define FALSE  0
#define PORT 8888
 
int main(int argc , char *argv[])
{
  int opt = TRUE;
  int master_socket , addrlen , new_socket , client_socket[30] , max_clients = 30 , activity, i , valread , sd;
  int max_sd;
  int sd_last;
  int has_stdin = 0;
  struct sockaddr_in address;
      
  char buffer[1025];  //data buffer of 1K
      
  //set of socket descriptors
  fd_set readfds;
      
  //a message
  char *message = "ECHO Daemon v1.0 \r\n";
  
  //initialise all client_socket[] to 0 so not checked
  for (i = 0; i < max_clients; i++) 
    {
      client_socket[i] = 0;
    }
      
  //create a master socket
  if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0) 
    {
      perror("socket failed");
      exit(EXIT_FAILURE);
    }
  
  //set master socket to allow multiple connections , this is just a good habit, it will work without this
  if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
    {
      perror("setsockopt");
      exit(EXIT_FAILURE);
    }
  
  //type of socket created
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons( PORT );
      
  //bind the socket to localhost port 8888
  if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0) 
    {
      perror("bind failed");
      exit(EXIT_FAILURE);
    }
  printf("Listener on port %d \n", PORT);
     
  //try to specify maximum of 3 pending connections for the master socket
  if (listen(master_socket, 3) < 0)
    {
      perror("listen");
      exit(EXIT_FAILURE);
    }
      
  //accept the incoming connection
  addrlen = sizeof(address);
  puts("Waiting for connections ...");
     
  while(TRUE) 
    {
      //clear the socket set
      FD_ZERO(&readfds);
  
      //add master socket to set
      FD_SET(master_socket, &readfds);
      max_sd = master_socket;

      //Add stdin
      FD_SET(fileno(stdin), &readfds);
         
      //add child sockets to set
      for ( i = 0 ; i < max_clients ; i++) 
        {
          //socket descriptor
          sd = client_socket[i];
             
          //if valid socket descriptor then add to read list
          if(sd > 0)
            FD_SET( sd , &readfds);
             
          //highest file descriptor number, need it for the select function
          if(sd > max_sd)
            max_sd = sd;
        }
  
      //wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely
      activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);
    
      if ((activity < 0) && (errno!=EINTR)) 
        {
          printf("select error");
        }
          
      //If something happened on the master socket , then its an incoming connection
      if (FD_ISSET(master_socket, &readfds)) 
        {
          if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
            {
              perror("accept");
              exit(EXIT_FAILURE);
            }
          
          //inform user of socket number - used in send and receive commands
          printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
        
          //send new connection greeting message
          if( send(new_socket, message, strlen(message), 0) != strlen(message) ) 
            {
              perror("send");
            }
              
          puts("Welcome message sent successfully");
              
          //add new socket to array of sockets
          for (i = 0; i < max_clients; i++) 
            {
              //if position is empty
              if( client_socket[i] == 0 )
                {
                  client_socket[i] = new_socket;
                  printf("Adding to list of sockets as %d\n" , i);
                     
                  break;
                }
            }
        }

      // If its stdin, send message
  /*     if (FD_ISSET(fileno(stdin), &readfds)){ */
 /*        // sd = fileno(stdin); */
 /*        has_stdin = 1; */
 /*        i=0; */

 /*        //Check if it was for closing , and also read the incoming message */
        
 /*        //   valread = read(fileno(stdin), buffer, 1024); */
 /* printf("Here i=%d\n", i); */
 /*        fgets(buffer, 1024, stdin); */

              
 /*        //Echo back the message that came in */
 /*                 //set the string terminating NULL byte on the end of the data read */

 /*        //buffer[valread] = '\0'; */
                  
 /*        //   printf("%s", buffer); */
 /*        for(i=0; i<max_clients; i++){ */
 /*          sd = client_socket[i]; */
 /*          send(sd , buffer , strlen(buffer) , 0 ); */
            
 /*          /\* send(sd , buffer , strlen(buffer) , 0 ); *\/ */
 /*        } */
 /*      } */
          
      //else its some IO operation on some other socket :)
      for (i = 0; i < max_clients; i++) 
        {
          //   if(has_stdin == 1)
            // printf("Here i=%d\n", i);
          ////////////////////////////////////////
          if (i==25)
            sd = fileno(stdin);
          else
          sd = client_socket[i];
          // sd_last = client_socket[i];    
          if (FD_ISSET( sd , &readfds) &&  sd != fileno(stdin) )
            {
              //Check if it was for closing , and also read the incoming message
              if ((valread = read( sd , buffer, 1024)) == 0)
                {
                  //Somebody disconnected , get his details and print
                  getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);
                  printf("Host disconnected , ip %s , port %d \n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
                      
                  //Close the socket and mark as 0 in list for reuse
                  close( sd );
                  client_socket[i] = 0;
                }

              
              //Echo back the message that came in
              else
                {
                  //set the string terminating NULL byte on the end of the data read

                  buffer[valread] = '\0';
                  
                  printf("%s", buffer);
                  //   send(sd , buffer , strlen(buffer) , 0 );
                }
            }
          else if (FD_ISSET( sd , &readfds) && sd == fileno(stdin))
            
            {
              // sd = fileno(stdin);
              has_stdin = 1;
              i=0;

              //Check if it was for closing , and also read the incoming message
        
              //   valread = read(fileno(stdin), buffer, 1024);
              // printf("Here i=%d\n", i);
              fgets(buffer, 1024, stdin);

              
              //Echo back the message that came in
              //set the string terminating NULL byte on the end of the data read

              //buffer[valread] = '\0';
                  
              //   printf("%s", buffer);
              for(i=0; i<max_clients; i++){
                sd = client_socket[i];
                send(sd , buffer , strlen(buffer) , 0 );
            
                /* send(sd , buffer , strlen(buffer) , 0 ); */
              }
            } 
             
           
        }
    }
      
  return 0;
} 













/*
 * This will handle connection for each client
 * */
/* void *connection_handler(void *socket_desc) */
/* { */
/*     //Get the socket descriptor */
/*     int sock = *(int*)socket_desc; */
/*     int read_size; */
/*     char *message , client_message[2000]; */
     
/*     //Send some messages to the client */
/*     message = "Greetings! I am your connection handler\n"; */
/*     write(sock , message , strlen(message)); */
     
/*     message = "Now type something and i shall repeat what you type \n"; */
/*     write(sock , message , strlen(message)); */

       
/*     while( (read_size = recv(sock , client_message , 2000 , 0)) > 0 ) */
/*       { */
/*         //Send the message back to client */
/*         printf("%s", client_message); */
/*       } */
     
/*     if(read_size == 0) */
/*       { */
/*         puts("Client disconnected"); */
/*         fflush(stdout); */
/*       } */
/*     else if(read_size == -1) */
/*       { */
/*         perror("recv failed"); */
/*       } */
/*     //close(socket_desc); */
/*     //Free the socket pointer */
/*     free(socket_desc); */
     
/*     return 0; */
/* } */


/* void *reply_handler(void* socket_desc){ */
/*   //Get the socket descriptor */
/*   int sock = *(int*)socket_desc; */
/*   int read_size; */
/*   char client_message[2000]; */
/*   while( (read_size = recv(sock , client_message , 2000 , 0)) > 0 ) */
/*     { */
/*       //Send the message back to client */
/*       //printf("%s\n", client_message); */
/*       char message[2000]; */
/*       scanf("%s", message); */
/*       write(sock, message, strlen(message)); */
/*     } */

/*   return 0; */
/* } */
