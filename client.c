// working incurses + chatbox
//Select and pull to handle simulataneous messaging
#include <curses.h>
#include <stdint.h>
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
#include <time.h>

#define BOARD_WIDTH 50
#define BOARD_HEIGHT 25
#define ARRAY_LEN BOARD_WIDTH*BOARD_HEIGHT

char read_input(int socket);
void init_display();
int screen_col(int col);
int screen_row(int row);
void draw_board(int *board);

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

        //Keyboard Shenanigans
    WINDOW* mainwin = initscr();
    noecho();               // Don't print keys when pressed
    keypad(mainwin, true);  // Support arrow keys
    timeout(0);
    init_display();
    //Send some data
    //message = "Hello!\n";

    int targetArray[ARRAY_LEN];
    char *buffer = (char*)targetArray;
    size_t remaining = sizeof(int) * ARRAY_LEN;
        
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

      /* if (FD_ISSET(stdin_sd, &readfds)){ */
      /*    fgets(message, 1024, stdin); */
      /*   send(socket_desc , message , strlen(message) , 0); */
   
      /* } */


      

      char keystroke = read_input(socket_desc);
      //if (keystroke == 'q' || 'w' || 'a' || 's' || 'd')
      //  {
      if(keystroke != -50){
        char keystroke_ts[3];
        keystroke_ts[0] = keystroke;            
        keystroke_ts[1] = '\n';
        keystroke_ts[2] = '\0';
        send(socket_desc, keystroke_ts, strlen(keystroke_ts) , 0);
          
        /* printf("String to be sent: %s\n", keystroke_ts); */
          
      }


    

      if (FD_ISSET(socket_desc, &readfds)){

        //Prints out a message
        /* valread = read( socket_desc , message, 1024); */
        /* message[valread] = '\0'; */
        /* printf("%s", message); */
        /* //printf("Message received!\n"); */
        /* printf("%s", message); */
          /* int j, k;
          /* for(j=0; j<50; j++){ */
          /*   for(k=0; k<25;k++){ */
          /*     printf("%d ", client_array[j][k]); */
          /*   } */
          /*   printf("\n"); */
          /* } */

        //For arrays
         int targetArray[ARRAY_LEN]; 

        char *buffer = (char*)targetArray;
        size_t remaining = sizeof(int) * ARRAY_LEN;
        while (remaining) {
          ssize_t recvd = read(socket_desc, buffer, remaining);
          // TODO: check for read errors etc here...
          remaining -= recvd;
          buffer += recvd;
        }
        draw_board(targetArray);
      /*   for (m=0;m<=ARRAY_LEN;m++){ */
      /*     if (m%10 == 0) */
      /*       printf("\n"); */
      /*     printf(" %d", targetArray[m]); */
      /*   } */
      /* } */
      
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




char read_input(int socket) {
  int key = getch();
  char key_ts;
  int is_valid_key = 0;
  if(key == KEY_UP) {
    key_ts = 'w';
    is_valid_key = 1;
  } else if(key == KEY_RIGHT) {
    key_ts = 'd';
    is_valid_key = 1;
  } else if(key == KEY_DOWN) {
    key_ts = 's';
    is_valid_key = 1;
  } else if(key == KEY_LEFT) {
    key_ts = 'a';
    is_valid_key = 1;
  } else if(key == 'q') {
    key_ts = 'q';
    is_valid_key = 1;
  }

  if(is_valid_key == 1){
    return key_ts;
  }
  else
    return -50;
 
}













//Functions from worm.c to represent screen
/**
 * Convert a board row number to a screen position
 * \param   row   The board row number to convert
 * \return        A corresponding row number for the ncurses screen
 */
int screen_row(int row) {
  return 2 + row;
}

/**
 * Convert a board column number to a screen position
 * \param   col   The board column number to convert
 * \return        A corresponding column number for the ncurses screen
 */
int screen_col(int col) {
  return 2 + col;
}

// Initialize the board display by printing the title and edges
void init_display() {
  // Print Title Line
  move(screen_row(-2), screen_col(BOARD_WIDTH/2 - 5));
  addch(ACS_DIAMOND);
  addch(ACS_DIAMOND);
  printw(" Worm! ");
  addch(ACS_DIAMOND);
  addch(ACS_DIAMOND);
  
  // Print corners
  mvaddch(screen_row(-1), screen_col(-1), ACS_ULCORNER);
  mvaddch(screen_row(-1), screen_col(BOARD_WIDTH), ACS_URCORNER);
  mvaddch(screen_row(BOARD_HEIGHT), screen_col(-1), ACS_LLCORNER);
  mvaddch(screen_row(BOARD_HEIGHT), screen_col(BOARD_WIDTH), ACS_LRCORNER);
  
  // Print top and bottom edges
  int row, col;
  for(col=0; col<BOARD_WIDTH; col++) {
    mvaddch(screen_row(-1), screen_col(col), ACS_HLINE);
    mvaddch(screen_row(BOARD_HEIGHT), screen_col(col), ACS_HLINE);
  }
  
  // Print left and right edges
  
  for(row=0; row<BOARD_HEIGHT; row++) {
    mvaddch(screen_row(row), screen_col(-1), ACS_VLINE);
    mvaddch(screen_row(row), screen_col(BOARD_WIDTH), ACS_VLINE);
  }
  
  refresh();
}


void draw_board(int *board) {
  int r, c;
  // Loop over cells of the game board
  for( r=0; r<BOARD_HEIGHT; r++) {
    for( c=0; c<BOARD_WIDTH; c++) {
      if(board[r*BOARD_WIDTH+c] == 0) {  // Draw blank spaces
        mvaddch(screen_row(r), screen_col(c), ' ');
      } else if(board[r*BOARD_WIDTH+c] > 0) {  // Draw worm
        mvaddch(screen_row(r), screen_col(c), ACS_CKBOARD);
      } else {  // Draw apple spinner thing
        char spinner_chars[] = {'|', '/', '-', '\\'};
        mvaddch(screen_row(r), screen_col(c), spinner_chars[abs(board[r*BOARD_WIDTH+c] % 4)]);
      }
    }
  }
  
  // Draw the score
  //mvprintw(screen_row(-2), screen_col(BOARD_WIDTH-9), "Score %03d\r", worm_length-INIT_WORM_LENGTH);
  
  refresh();
}
