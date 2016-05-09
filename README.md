# network-worm-game
A network Worm! game using C (By Abraham Mhaidli & Joel Katticaran)

Our system is a two-player network Worm! Game. Inside, you will find two
folders, A client-folder and a server-folder. The server folder contains
the necessary files and makefile to create the server that will host the
game. To compile the server, run 'make' when in the proper directory in the
terminal window. Once the server has been compiled, it can be executed by
the command ./server. The client folder contains
the necessary file and makefile to connect to the server (when provided
with the correct IP address). To compile the server, run 'make' when in the
proper directory in the terminal window. Once the client has been compiled,
it can be executed by the command ./client.

To Launch the game, first, one would compile and run the server. Before
doing this however, make sure to note down the IP address of the machine
that is hosting the server. This can be found using the 'ip addr show'
command. Once the server has been hosted, two players can connect to the
game by compiling and running the client program. Once the client program
has been run, it will ask for the server IP address. Type in the IP address
of the server machine and hit enter. Once both players have connected, the
game will begin.

Once the game has begun, the client terminal windows will display the board
that displays the worm board. The player can then interact with the worm by
pressing arrow keys to change the worm's direction or 'q' to quit the
game. Once the game is over (When one player wins by getting 10 points or
one of the player dies/quits), the screen displays an appropriate victory
message and then the game quits and returns to the terminal window.

While the game is running, the server will display the game on the terminal
of the server machine. This allows for spectators to watch the game.


Citations:
Inspiration for this game is based on the implementation of the Worm! game
by Dr. Charlie Curtsinger.
http://www.cs.grinnell.edu/~curtsinger/teaching/2016S/CSC213/labs/real-time/

Sockets:
http://www.binarytides.com/multiple-socket-connections-fdset-select-linux/
http://www.binarytides.com/socket-programming-c-linux-tutorial/
http://stackoverflow.com/questions/674525/when-sending-an-array-of-int-over-tcp-why-are-only-the-first-amount-correct

ncurses:
http://www.tldp.org/HOWTO/html_single/NCURSES-Programming-HOWTO/

Scheduler:
Based on past work by Abraham Mhaidli & Daniel Nanetti-Palacios
