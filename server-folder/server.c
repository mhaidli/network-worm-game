/*Program that runs the server for the game*/

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
#define ARRAY_LEN BOARD_WIDTH*(BOARD_HEIGHT+1)
#define NUMBER_OF_PLAYERS 2
#define THRESHOLD_VALUE 10000

#define SCORE_CAP 10

//Game variables
//board
int board[BOARD_HEIGHT+1][BOARD_WIDTH];
//creating an additional row so that other data
//can be sent (eg: player 1 score, player 2 score etc)

int running = 1;

//Worm variables
int worm_dir0 = DIR_NORTH;
int worm_dir1 = DIR_NORTH;
int worm_length0 = INIT_WORM_LENGTH;
int worm_length1 = INIT_WORM_LENGTH;

// Apple parameters
int apple_age = 120;


//Function signatures

//Initialize scheduler
int scheduler_init();
//Change worm direction based on the input passed in
void read_input(char key, int i);
//Gets keystrokes pressed from client and passes it to read_input to change worm direction
void get_keystrokes();
//Sends an array representation of the board to the clients
void send_board();
//Game Functions
// Pick a random location and generate an apple there
void generate_apple();
// Add to the apple numbers to control their duration and animation
void update_apples();
// Update the age of each segment of the worm and move the worm into its next position
void update_worm();
// Show a game over message, wait for a key press, then stop the game scheduler
void end_game();
//Initializes screen for Worm! game
void init_display();
//Convert a board row number to a screen position
int screen_row(int col);
//Convert a board column number to a screen position
int screen_col(int row);


//Global variables
fd_set readfds;
int max_clients = NUMBER_OF_PLAYERS;
int client_socket[NUMBER_OF_PLAYERS];


//Main
int main(int argc , char *argv[])
{

  //Variables
  int opt = TRUE;
  int master_socket , addrlen , new_socket;
  int activity, i , valread , sd;
  int max_sd ;
  int sd_last;
  int has_stdin = 0;
  int client_sock;
  int connections = 0;

  struct sockaddr_in address;
      
  
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

  while(running) 
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
          printf("Player connected , socket fd is %d , ip is : %s , port : %d \n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
          connections++;

                 
          //add new socket to array of sockets
          for (i = 0; i < max_clients; i++) 
            {
              //if position is empty
              if( client_socket[i] == 0 )
                {
                  client_socket[i] = new_socket;
                  if (connections < NUMBER_OF_PLAYERS)
                  printf("Waiting for Player 2 to connect...\n");
                     
                  break;
                }
            }
          
          //If we have right number of players, initialize game by initializing scheduler
           if (connections == NUMBER_OF_PLAYERS)
             scheduler_init();//start scheduler             
        }

    }//while
  sleep(3);
  //end ncurses
  endwin();
  return 0;
} //main




//The below three functions, screen_row, screen_col, and init_display all concern themselves with
//displaying the board, and so are moved to the client.
//They have been left here so that the screen is also displayed in the server for the purpose
//of spectating the game and to aid in debugging if necessary.
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
  move(screen_row(-2), screen_col(BOARD_WIDTH/2 -5));
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
void end_game(int loser) {
  
  board[BOARD_HEIGHT][3] = loser;
  if(loser == 1){// if player 1 dies
    mvprintw(screen_row(BOARD_HEIGHT/2)-1, screen_col(BOARD_WIDTH/2)-6, "            ");
    mvprintw(screen_row(BOARD_HEIGHT/2),   screen_col(BOARD_WIDTH/2)-6, " Player 2 Wins!");
    mvprintw(screen_row(BOARD_HEIGHT/2)+1, screen_col(BOARD_WIDTH/2)-16, " Player 1 You Should Delete This Game And");
    mvprintw(screen_row(BOARD_HEIGHT/2)+2, screen_col(BOARD_WIDTH/2)-16, "Go Do Something Productive in Your Life");
    mvprintw(screen_row(BOARD_HEIGHT/2)+3, screen_col(BOARD_WIDTH/2)-6, "            ");
  }
  else if (loser == 2){// if player 2 dies
    mvprintw(screen_row(BOARD_HEIGHT/2)-1, screen_col(BOARD_WIDTH/2)-6, "            ");
    mvprintw(screen_row(BOARD_HEIGHT/2),   screen_col(BOARD_WIDTH/2)-6, " Player 1 Wins! ");
    mvprintw(screen_row(BOARD_HEIGHT/2)+1, screen_col(BOARD_WIDTH/2)-6, " Player 2 You Just Got Rekt");
    mvprintw(screen_row(BOARD_HEIGHT/2)+2, screen_col(BOARD_WIDTH/2)-6, "            ");
  }
  else if (loser == 3){//if player 1 quits
    mvprintw(screen_row(BOARD_HEIGHT/2)-1, screen_col(BOARD_WIDTH/2)-6, "            ");
    mvprintw(screen_row(BOARD_HEIGHT/2),   screen_col(BOARD_WIDTH/2)-6, " Player 2 Wins! ");
    mvprintw(screen_row(BOARD_HEIGHT/2)+1, screen_col(BOARD_WIDTH/2)-26, "Cause Player 1 Was Too Much of a Wuss to Continue");
    mvprintw(screen_row(BOARD_HEIGHT/2)+2, screen_col(BOARD_WIDTH/2)-6, "            ");
  }
  else if (loser == 4){//if player 2 quits
    mvprintw(screen_row(BOARD_HEIGHT/2)-1, screen_col(BOARD_WIDTH/2)-6, "            ");
    mvprintw(screen_row(BOARD_HEIGHT/2),   screen_col(BOARD_WIDTH/2)-6, " Player 1 Wins! ");
    mvprintw(screen_row(BOARD_HEIGHT/2)+1, screen_col(BOARD_WIDTH/2)-16, " Player 2 Decided to Join the ");
    mvprintw(screen_row(BOARD_HEIGHT/2)+2, screen_col(BOARD_WIDTH/2)-16, " Gallus Gallus Domesticus Family");
    mvprintw(screen_row(BOARD_HEIGHT/2)+3, screen_col(BOARD_WIDTH/2)-6, "            ");
  }
  refresh();
  send_board();
  timeout(-1);
  stop_scheduler();
}//end_game

// Draw the actual game board. Board state is stored in a single 2D integer array.
void draw_board() {
  // Loop over cells of the game board
  for(int r=0; r<BOARD_HEIGHT; r++) {
    for(int c=0; c<BOARD_WIDTH; c++) {
      if(board[r][c] == 0) {  // Draw blank spaces
        mvaddch(screen_row(r), screen_col(c), ' ');
      } else if(board[r][c] > 0) {  // Draw worm
        if(board[r][c] < THRESHOLD_VALUE){
          //icon for player 1
          mvaddch(screen_row(r), screen_col(c), '@');
        }
        else
          mvaddch(screen_row(r), screen_col(c), 'O'); //icon for player 2
      } else {  // Draw apple spinner thing
        char spinner_chars[] = {'|', '/', '-', '\\'};
        mvaddch(screen_row(r), screen_col(c), spinner_chars[abs(board[r][c] % 4)]);
      }
    }
  }
      
  // Draw the score
  mvprintw(screen_row(-2), screen_col(0), "Player1:@ Player2:O\r");
  mvprintw(screen_row(-2), screen_col(BOARD_WIDTH-14), "Player1: %03d Player2: %03d\r", board[BOARD_HEIGHT][0], board[BOARD_HEIGHT][1]);

  refresh();
}//draw_board



// Update the age of each segment of the worm and move the worm into its next position
//player 1's worm
void update_worm_0() {
  int worm_row;
  int worm_col;
  
  int worm_dir = worm_dir0;
  int worm_length = worm_length0;
  
  // "Age" each existing segment of the worm
  for(int r=0; r<BOARD_HEIGHT; r++) {
    for(int c=0; c<BOARD_WIDTH; c++) {
      if(board[r][c] == 1) {  // Found the head of the worm. Save position
        worm_row = r;
        worm_col = c;
      }
      
      // Add 1 to the age of the worm segment. Threshold value used to identify worm.
      if(board[r][c] > 0 && THRESHOLD_VALUE > board[r][c]) {
        board[r][c]+=1;
        
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
    end_game(1);
    return;
  }
  
  // Check for worm collisions
  if(board[worm_row][worm_col] > 0) {
    end_game(1);
    return;
  }
  
  // Check for apple collisions
  if(board[worm_row][worm_col] < 0) {
    // Worm gets longer
    worm_length0++;
  }
  
  // Add the worm's new position
  board[worm_row][worm_col] = 1;

  //Update score
  int score = worm_length0 - INIT_WORM_LENGTH;
  board[BOARD_HEIGHT][0] = score;
  if (score >= SCORE_CAP)
    end_game(2);
  
  // Update the worm movement speed to deal with rectangular cursors
  if(worm_dir == DIR_NORTH || worm_dir == DIR_SOUTH) {
    update_job_interval(WORM_VERTICAL_INTERVAL);
  } else {
    update_job_interval(WORM_HORIZONTAL_INTERVAL);
  }
}//update_worm_0


// Update the age of each segment of the worm and move the worm into its next position
//player 2's worm
void update_worm_1() {
  int worm_row;
  int worm_col;

  int worm_dir = worm_dir1;
  int worm_length = worm_length1;
  
  // "Age" each existing segment of the worm
  for(int r=0; r<BOARD_HEIGHT; r++) {
    for(int c=0; c<BOARD_WIDTH; c++) {
      if(board[r][c] == THRESHOLD_VALUE) {  // Found the head of the worm. Save position
        worm_row = r;
        worm_col = c;
      }
      
      // Add 1 to the age of the worm segment
      if(board[r][c] >= THRESHOLD_VALUE ) {
        board[r][c]+= 1;
        
        // Remove the worm segment if it is too old
        if(board[r][c] >= (worm_length + THRESHOLD_VALUE)) {
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
    end_game(2);
    return;
  }
  
  // Check for worm collisions
  if(board[worm_row][worm_col] > 0) {
    end_game(2);
    return;
  }
  
  // Check for apple collisions
  if(board[worm_row][worm_col] < 0) {
    // Worm gets longer
    worm_length1++;
  }
  
  // Add the worm's new position
  board[worm_row][worm_col] = THRESHOLD_VALUE;

  //Update score
  int score = worm_length1 - INIT_WORM_LENGTH;
  board[BOARD_HEIGHT][1] = score;
  if (score >= SCORE_CAP)
    end_game(1);

  
  // Update the worm movement speed to deal with rectangular cursors
  if(worm_dir == DIR_NORTH || worm_dir == DIR_SOUTH) {
    update_job_interval(WORM_VERTICAL_INTERVAL);
  } else {
    update_job_interval(WORM_HORIZONTAL_INTERVAL);
  }
}//update_worm_1



// Check for keyboard input and respond accordingly
void read_input(char key, int i) {
  
  if (i == 0){//to indicate which player is giving the comand
    if(key == 'w' && worm_dir0 != DIR_SOUTH) {
      worm_dir0 = DIR_NORTH;
    } else if(key == 'd' && worm_dir0 != DIR_WEST) {
      worm_dir0 = DIR_EAST;
    } else if(key == 's' && worm_dir0 != DIR_NORTH) {
      worm_dir0 = DIR_SOUTH;
    } else if(key == 'a' && worm_dir0 != DIR_EAST) {
      worm_dir0 = DIR_WEST;
    } else if(key == 'q') {
      end_game(3);
      stop_scheduler();
      return;
    }
  }
  else{
    if(key == 'w' && worm_dir1 != DIR_SOUTH) {
      worm_dir1 = DIR_NORTH;
    } else if(key == 'd' && worm_dir1 != DIR_WEST) {
      worm_dir1 = DIR_EAST;
    } else if(key == 's' && worm_dir1 != DIR_NORTH) {
      worm_dir1 = DIR_SOUTH;
    } else if(key == 'a' && worm_dir1 != DIR_EAST) {
      worm_dir1 = DIR_WEST;
    } else if(key == 'q') {
      end_game(4);
      stop_scheduler();
      return;
    }
  }
}//read_input



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

  board[BOARD_HEIGHT/2][(BOARD_WIDTH) / (NUMBER_OF_PLAYERS + 1)] = 1;
  board[BOARD_HEIGHT/2][(BOARD_WIDTH * 2) / (NUMBER_OF_PLAYERS + 1)] = THRESHOLD_VALUE;


  //Initialize score
  board[BOARD_HEIGHT+1][0] = 0;
  board[BOARD_HEIGHT+1][1] = 0;

  //Initialize Losing
  board[BOARD_HEIGHT][3] = 0;
  
  // Create a job to update the worm every 200ms
  add_job(update_worm_0, 200);
  
  // Create a job to update the other worm every 200ms
  add_job(update_worm_1, 200);
  
  // Create a job to draw the board every 33ms
  add_job(draw_board, 33);
  
  // Create a job to update apples every 120ms
  add_job(update_apples, 120);
  
  // Create a job to generate new apples every 2 seconds
  add_job(generate_apple, 10000);

  //Create job to check for keystrokes ever 150ms
  add_job(get_keystrokes, 150);

  //send array every 33ms
  add_job(send_board, 33);
  
  // Generate a few apples immediately
  for(int i=0; i<5; i++) {
    generate_apple();
  }
  
  // Run the task queue
  run_scheduler();
  
  // Clean up window
  delwin(mainwin);
  running = 0;

  return 0;
}//scheduler


// Function to check for keystrokes from client
void get_keystrokes(){
  int i;
  char buffer[1025];
  int sd;
  int valread;
  int max_sd = -1;

  struct timeval mytime;
    
  mytime.tv_sec=0;         /* seconds */
  mytime.tv_usec=0;        /* microseconds */
   
  FD_ZERO(&readfds);

  //Add clients in client socket to readfds;
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

  //wait for an activity on one of the sockets , timeout is 0 , so return immediately
  int activity = select( max_sd + 1 , &readfds , NULL , NULL , &mytime);
 
  if ((activity < 0) && (errno!=EINTR)) 
    {
      printf("select error");
    }
  
  //Check activity from sockets
  for (i = 0; i < max_clients; i++) 
    {

      if (i == max_clients)
        sd = fileno(stdin);
      else
        sd = client_socket[i];

      //If curret client socket was active, deal with situ   
      if (FD_ISSET( sd , &readfds) &&  sd != fileno(stdin) )
        {
          //Check if it was for closing
          if ((valread = read( sd , buffer, 1024)) == 0)
            {
              perror("Player Aborted");
            }
          
          //Get key that was pressed and pass to read_input
          else
            {
              read_input(buffer[0], i);
            }
        }
    }
}//get_keystrokes


//Sends an array representation of the board to the clients
void send_board()
{
  int i;
  for(i=0; i<max_clients;i++){
    board[BOARD_HEIGHT][2] = i;
    int result = send(client_socket[i], board, sizeof(int) * ARRAY_LEN, 0);
    if(result == -1)
      perror("send failed");
  }
}//send_board

