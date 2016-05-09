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
#define ARRAY_LEN BOARD_WIDTH*(BOARD_HEIGHT+1)
#define NUMBER_OF_PLAYERS 2
#define THRESHOLD_VALUE 10000

int running = 1;
//Function  signatures

//Reads key pressed and returns it as char
char read_input(int socket);

//Initializes screen for Worm! game
void init_display();

//Convert a board row number to a screen position
int screen_row(int col);

//Convert a board column number to a screen position
int screen_col(int row);

//Draws the board on the screen based on the array that was passed in
void draw_board(int *board);

//Ends game
void end_game(int loser);

//main
int main(int argc , char *argv[])
{
  //Variables
  int socket_desc;
  struct sockaddr_in server;
  char message[1024];
  int max_sd;
  int server_sd, activity;
  int stdin_sd = fileno(stdin);
  fd_set readfds;
        
  //Create socket
  socket_desc = socket(AF_INET , SOCK_STREAM , 0);
  if (socket_desc == -1)
    {
      printf("Could not create socket");
    }

  //Read ip and connect to it
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

  //Initialize screen and ncurses
  WINDOW* mainwin = initscr();
  noecho();               // Don't print keys when pressed
  keypad(mainwin, true);  // Support arrow keys
  timeout(0);
  init_display();

        
  while(running){
  
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

    //Check for keystrokes and send to server
    char keystroke = read_input(socket_desc);
    
    if(keystroke != -50){
      char keystroke_ts[3];
      keystroke_ts[0] = keystroke;            
      keystroke_ts[1] = '\n';
      keystroke_ts[2] = '\0';
      send(socket_desc, keystroke_ts, strlen(keystroke_ts) , 0);
    }

    //Check for incoming array information, and draw board
    if (FD_ISSET(socket_desc, &readfds)){

      int targetArray[ARRAY_LEN]; 

      char *buffer = (char*)targetArray;
      size_t remaining = sizeof(int) * ARRAY_LEN;
      while (remaining) {
        ssize_t recvd = read(socket_desc, buffer, remaining);
        if(recvd == -1){
          perror("recvd error");
        }
        remaining -= recvd;
        buffer += recvd;
      }
      draw_board(targetArray);
    }
  }//while loop
  
  endwin();
  return 0;
}//main


//Reads key pressed and returns it as char
//If no key was pressed, or if a key was pressed that wasn't an arrow key, function
//returns -50;
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
}//read_input


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
  mvprintw(screen_row(BOARD_HEIGHT+1), screen_col((BOARD_WIDTH/3) - 12), "First to 10 wins. May the force be with you");
  
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
}//init_display


//Draws the board on the screen based on the array that was passed in
void draw_board(int *board) {
  int r, c;
  // Loop over cells of the game board
  if(board[((BOARD_HEIGHT*BOARD_WIDTH) + 3)] != 0)
    end_game(board[((BOARD_HEIGHT*BOARD_WIDTH) + 3)]);
  else{
    for( r=0; r<BOARD_HEIGHT; r++) {
      for( c=0; c<BOARD_WIDTH; c++) {
        if(board[r*BOARD_WIDTH+c] == 0) {  // Draw blank spaces
          mvaddch(screen_row(r), screen_col(c), ' ');
        } else if(board[r*BOARD_WIDTH+c] > 0) {  // Draw worm
          if(board[r*BOARD_WIDTH+c] >= THRESHOLD_VALUE){
            mvaddch(screen_row(r), screen_col(c), 'O');
          }
          else{
            mvaddch(screen_row(r), screen_col(c), '@');
          }
        } else {  // Draw apple spinner thing
          char spinner_chars[] = {'|', '/', '-', '\\'};
          mvaddch(screen_row(r), screen_col(c), spinner_chars[abs(board[r*BOARD_WIDTH+c] % 4)]);
        }
      }
    }
  }
    
  // Draw the score and other such information
  int player = board[((BOARD_HEIGHT*BOARD_WIDTH) + 2)] + 1;
  char shape;
  
  if(player == 2)
    shape = 'O';
  else
    shape = '@';
  
  mvprintw(screen_row(-2), screen_col(0), "Player %d {%c%c%c}\r", player, shape, shape, shape);
  mvprintw(screen_row(-2), screen_col(BOARD_WIDTH-13), "Player1: %03d Player2: %03d\r", board[((BOARD_HEIGHT*BOARD_WIDTH) + 0)], board[((BOARD_HEIGHT * BOARD_WIDTH) + 1)]);
  
  refresh();
}//draw_board




// Show a game over message, wait for a key press, then stop the game scheduler
void end_game(int loser) {
  if(loser == 1){
    mvprintw(screen_row(BOARD_HEIGHT/2)-1, screen_col(BOARD_WIDTH/2)-6, "            ");
    mvprintw(screen_row(BOARD_HEIGHT/2),   screen_col(BOARD_WIDTH/2)-6, " Player 2 Wins!");
    mvprintw(screen_row(BOARD_HEIGHT/2)+1, screen_col(BOARD_WIDTH/2)-16, " Player 1 You Should Delete This Game And");
    mvprintw(screen_row(BOARD_HEIGHT/2)+2, screen_col(BOARD_WIDTH/2)-16, "Go Do Something Productive in Your Life");
    mvprintw(screen_row(BOARD_HEIGHT/2)+3, screen_col(BOARD_WIDTH/2)-6, "            ");
  }
  else if (loser == 2){
    mvprintw(screen_row(BOARD_HEIGHT/2)-1, screen_col(BOARD_WIDTH/2)-6, "            ");
    mvprintw(screen_row(BOARD_HEIGHT/2),   screen_col(BOARD_WIDTH/2)-6, " Player 1 Wins! ");
    mvprintw(screen_row(BOARD_HEIGHT/2)+1, screen_col(BOARD_WIDTH/2)-6, " Player 2 You Just Got Rekt");
    mvprintw(screen_row(BOARD_HEIGHT/2)+2, screen_col(BOARD_WIDTH/2)-6, "            ");
  }
  else if (loser == 3){
    mvprintw(screen_row(BOARD_HEIGHT/2)-1, screen_col(BOARD_WIDTH/2)-6, "            ");
    mvprintw(screen_row(BOARD_HEIGHT/2),   screen_col(BOARD_WIDTH/2)-6, " Player 2 Wins! ");
    mvprintw(screen_row(BOARD_HEIGHT/2)+1, screen_col(BOARD_WIDTH/2)-28, " Cause Player 1 Was Too Much of a Wuss to Continue ");
    mvprintw(screen_row(BOARD_HEIGHT/2)+2, screen_col(BOARD_WIDTH/2)-6, "            ");
  }
  else if (loser == 4){
    mvprintw(screen_row(BOARD_HEIGHT/2)-1, screen_col(BOARD_WIDTH/2)-6, "            ");
    mvprintw(screen_row(BOARD_HEIGHT/2),   screen_col(BOARD_WIDTH/2)-6, " Player 1 Wins! ");
    mvprintw(screen_row(BOARD_HEIGHT/2)+1, screen_col(BOARD_WIDTH/2)-16, " Player 2 Decided to Join the ");
    mvprintw(screen_row(BOARD_HEIGHT/2)+2, screen_col(BOARD_WIDTH/2)-16, " Gallus Gallus Domesticus Family");
    mvprintw(screen_row(BOARD_HEIGHT/2)+3, screen_col(BOARD_WIDTH/2)-6, "            ");
  }
    
  refresh();
  sleep(3);
  running = 0;
  refresh();
}//end_game

