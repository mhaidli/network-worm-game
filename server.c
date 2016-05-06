
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
#include <curses.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <sys/select.h>

#include "scheduler.h"
#include "util.h"


//Socket definitions
#define TRUE  1
#define FALSE  0
#define PORT 8888


//Global Varibles for board
// Defines used to track the worm direction
#define DIR_NORTH 0
#define DIR_EAST 1
#define DIR_SOUTH 2
#define DIR_WEST 3

// Game parameters
#define INIT_WORM_LENGTH 4
#define WORM_HORIZONTAL_INTERVAL 200
#define WORM_VERTICAL_INTERVAL 300
#define BOARD_WIDTH 50
#define BOARD_HEIGHT 25
#define ARRAY_LEN BOARD_WIDTH*BOARD_HEIGHT

//Game vairables
//board
int board[BOARD_HEIGHT][BOARD_WIDTH];
int board_1d[ARRAY_LEN];

// Worm parameters
int worm_dir = DIR_NORTH;
int worm_length = INIT_WORM_LENGTH;

// Apple parameters
int apple_age = 120;


//Function signatures
int scheduler_init();
void read_input(char key);
void get_keystrokes();

//These technically do not exist here
void send_board();
void init_display();
int screen_col(int col);
int screen_row(int row);


//Global variables for get_keystroes function
fd_set readfds;
int client_socket[5];
int max_clients = 5;
//Main
int main(int argc , char *argv[])
{
  //Variables
  int opt = TRUE;
  int master_socket , addrlen , new_socket;
  int activity, i , valread , sd;
  int max_sd , l;
  int sd_last;
  int has_stdin = 0;
  int client_sock;
  int connections = 0;

  int sourceArray[ARRAY_LEN];
  for (l=0;l<ARRAY_LEN;l++)
            sourceArray[l] = l;
  
  struct sockaddr_in address;
      
  char buffer[1025];  //data buffer of 1K
      
  //set of socket descriptors
  //fd_set readfds;
      
  //a message
  char *message = "WELCOME TO SERVER";
  
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
  
  //set master socket to allow multiple connections
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

  //   fd_set readfds = *readfds_addr;
  while(TRUE) 
    {
      //clear the socket set
      FD_ZERO(&readfds);
  
      //add master socket to set
      FD_SET(master_socket, &readfds);
      max_sd = master_socket;

      //Add stdin
      FD_SET(fileno(stdin), &readfds);
         
      //add sockets to readfds
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
          connections++;
          //RUN SCHEDULER FUNCTION
        
          //send new connection greeting message
          /* if( send(new_socket, message, strlen(message), 0) != strlen(message) )  */
          /*   { */
          /*     perror("send"); */
          /*   } */
          client_sock = new_socket;
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
           if (connections == 1)
             scheduler_init();
             //SCHEDULER
        }
      

      //Send array to client
      
      //int result = send(client_sock, sourceArray, sizeof(int) * ARRAY_LEN, 0);
    }//while
    
      
  return 0;
} 


/**
 * Based on the ncurses tutorials at http://www.paulgriffiths.net/program/c/curses.php
 */


//The below three functions, screen_row, screen_col, and init_display all concern themselves with
//displaying the board, and so are moved to the client.
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
  for(int col=0; col<BOARD_WIDTH; col++) {
    mvaddch(screen_row(-1), screen_col(col), ACS_HLINE);
    mvaddch(screen_row(BOARD_HEIGHT), screen_col(col), ACS_HLINE);
  }
  
  // Print left and right edges
  for(int row=0; row<BOARD_HEIGHT; row++) {
    mvaddch(screen_row(row), screen_col(-1), ACS_VLINE);
    mvaddch(screen_row(row), screen_col(BOARD_WIDTH), ACS_VLINE);
  }
  
  refresh();
}

// Show a game over message, wait for a key press, then stop the game scheduler
void end_game() {
  mvprintw(screen_row(BOARD_HEIGHT/2)-1, screen_col(BOARD_WIDTH/2)-6, "            ");
  mvprintw(screen_row(BOARD_HEIGHT/2),   screen_col(BOARD_WIDTH/2)-6, " Game Over! ");
  mvprintw(screen_row(BOARD_HEIGHT/2)+1, screen_col(BOARD_WIDTH/2)-6, "            ");
  refresh();
  timeout(-1);
  getch();
  stop_scheduler();
}

// Draw the actual game board. Board state is stored in a single 2D integer array.
void draw_board() {
  // Loop over cells of the game board
  for(int r=0; r<BOARD_HEIGHT; r++) {
    for(int c=0; c<BOARD_WIDTH; c++) {
      if(board[r][c] == 0) {  // Draw blank spaces
        mvaddch(screen_row(r), screen_col(c), ' ');
      } else if(board[r][c] > 0) {  // Draw worm
        mvaddch(screen_row(r), screen_col(c), ACS_CKBOARD);
      } else {  // Draw apple spinner thing
        char spinner_chars[] = {'|', '/', '-', '\\'};
        mvaddch(screen_row(r), screen_col(c), spinner_chars[abs(board[r][c] % 4)]);
      }
    }
  }
  
  // Draw the score
  mvprintw(screen_row(-2), screen_col(BOARD_WIDTH-9), "Score %03d\r", worm_length-INIT_WORM_LENGTH);
  
  refresh();
}

// Check for keyboard input and respond accordingly
void read_input(char key) {
  // int key;// = getch();
  printf("Keystroke: %c\n", key);
  if(key == 'w' && worm_dir != DIR_SOUTH) {
    worm_dir = DIR_NORTH;
  } else if(key == 'd' && worm_dir != DIR_WEST) {
    worm_dir = DIR_EAST;
  } else if(key == 's' && worm_dir != DIR_NORTH) {
    worm_dir = DIR_SOUTH;
  } else if(key == 'a' && worm_dir != DIR_EAST) {
    worm_dir = DIR_WEST;
  } else if(key == 'q') {
    stop_scheduler();
    return;
  }
}

// Update the age of each segment of the worm and move the worm into its next position
void update_worm() {
  int worm_row;
  int worm_col;
  
  // "Age" each existing segment of the worm
  for(int r=0; r<BOARD_HEIGHT; r++) {
    for(int c=0; c<BOARD_WIDTH; c++) {
      if(board[r][c] == 1) {  // Found the head of the worm. Save position
        worm_row = r;
        worm_col = c;
      }
      
      // Add 1 to the age of the worm segment
      if(board[r][c] > 0) {
        board[r][c]++;
        
        // Remove the worm segment if it is too old
        if(board[r][c] > worm_length) {
          board[r][c] = 0;
        }
      }
    }
  }
  
  // Move the worm into a new space
  if(worm_dir == DIR_NORTH) {
    worm_row--;
  } else if(worm_dir == DIR_SOUTH) {
    worm_row++;
  } else if(worm_dir == DIR_EAST) {
    worm_col++;
  } else if(worm_dir == DIR_WEST) {
    worm_col--;
  }
  
  // Check for edge collisions
  if(worm_row < 0 || worm_row >= BOARD_HEIGHT || worm_col < 0 || worm_col >= BOARD_WIDTH) {
    end_game();
    return;
  }
  
  // Check for worm collisions
  if(board[worm_row][worm_col] > 0) {
    end_game();
    return;
  }
  
  // Check for apple collisions
  if(board[worm_row][worm_col] < 0) {
    // Worm gets longer
    worm_length++;
  }
  
  // Add the worm's new position
  board[worm_row][worm_col] = 1;
  
  // Update the worm movement speed to deal with rectangular cursors
  if(worm_dir == DIR_NORTH || worm_dir == DIR_SOUTH) {
    update_job_interval(WORM_VERTICAL_INTERVAL);
  } else {
    update_job_interval(WORM_HORIZONTAL_INTERVAL);
  }
}

// Add to the apple numbers to control their duration and animation
void update_apples() {
  // "Age" each apple
  for(int r=0; r<BOARD_HEIGHT; r++) {
    for(int c=0; c<BOARD_WIDTH; c++) {
      if(board[r][c] < 0) {  // Add one to each apple cell
        board[r][c]++;
      }
    }
  }
}

// Pick a random location and generate an apple there
void generate_apple() {
  // Repeatedly try to insert an apple at a random empty cell
  while(1) {
    int r = rand() % BOARD_HEIGHT;
    int c = rand() % BOARD_WIDTH;
    
    // If the cell is empty, add an apple
    if(board[r][c] == 0) {
      // Pick a random age between apple_age/2 and apple_age*1.5
      // Negative numbers represent apples, so negate the whole value
      board[r][c] = -((rand() % apple_age) + apple_age / 2);
      return;
    }
  }
}

// Entry point: Set up the game, create jobs, then run the scheduler
int scheduler_init() {
  WINDOW* mainwin = initscr();
  if(mainwin == NULL) {
    fprintf(stderr, "Error initializing ncurses.\n");
    exit(2);
  }
  
  // Seed random number generator with the time in milliseconds
  srand(time_ms());
  
  noecho();               // Don't print keys when pressed
  keypad(mainwin, true);  // Support arrow keys
  timeout(0);             // Non-blocking keyboard access
  
  init_display();
  
  // Zero out the board contents
  memset(board, 0, BOARD_WIDTH*BOARD_HEIGHT*sizeof(int));
  
  // Put the worm at the middle of the board
  board[BOARD_HEIGHT/2][BOARD_WIDTH/2] = 1;
  
  // Create a job to update the worm every 200ms
  add_job(update_worm, 200);
  
  // Create a job to draw the board every 33ms
  add_job(draw_board, 33);
  
  // Create a job to read user input every 150ms
  // add_job(read_input, 150);
  
  // Create a job to update apples every 120ms
  add_job(update_apples, 120);
  
  // Create a job to generate new apples every 2 seconds
  add_job(generate_apple, 2000);

  //Create job to check for keystrokes ever 150ms
  add_job(get_keystrokes, 150);

  //sending array
  add_job(send_board, 33);
  
  // Generate a few apples immediately
  for(int i=0; i<5; i++) {
    generate_apple();
  }
  
  // Run the task queue
  run_scheduler();
  
  // Clean up window
  delwin(mainwin);
  endwin();

  return 0;
}

int counter = 0;

// Function to check for keystrokes from client
//CHECKING FOR STUFF
void get_keystrokes(/*int* maxclients, fd_set* readfds*/){
  int i;
  char buffer[1025];
  int sd;
  int max_clients = 1;
  int valread;
  int max_sd = -1;

  struct timeval mytime;
    
  mytime.tv_sec=0;         /* seconds */
  mytime.tv_usec=0;        /* microseconds */
           
  
  FD_ZERO(&readfds);

  counter++;

  
  
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

  
     int activity = select( max_sd + 1 , &readfds , NULL , NULL , &mytime);
   
  
      if ((activity < 0) && (errno!=EINTR)) 
        {
          printf("select error");
        }
   
  //else its some IO operation on some other socket :)
  for (i = 0; i < max_clients; i++) 
    {

      if (i == max_clients)
        sd = fileno(stdin);
      else
        sd = client_socket[i];

      //If curret client socket was active, deal with situ   
      if (FD_ISSET( sd , &readfds) &&  sd != fileno(stdin) )
        {
          //Check if it was for closing , and also read the incoming message
          if ((valread = read( sd , buffer, 1024)) == 0)
            {
              /* //Somebody disconnected , get his details and print */
              /* getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen); */
              /* printf("Host disconnected , ip %s , port %d \n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port)); */
                      
              /* //Close the socket and mark as 0 in list for reuse */
              /* close( sd ); */
              /* client_socket[i] = 0; */
            }

              
          //Echo back the message that came in
          else
            {
              //set the string terminating NULL byte on the end of the data read
              read_input(buffer[0]);
              //buffer[valread] = '\0';
              //printf("%s", buffer);
            
              

              //  int result = send(client_sock, sourceArray, sizeof(int) * ARRAY_LEN, 0);
              //   send(sd , buffer , strlen(buffer) , 0 );
            }
        }

      //Else, check to see if stdin was activated
       
    }
    
}


void send_board()
{
  for(int i=0;i<max_clients;i++){
  int result = send(client_socket[i], board, sizeof(int) * ARRAY_LEN, 0);
  }
}
