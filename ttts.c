//tic tac toe server code
//presentation layer
//establish connection to clients 
//read messages
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <time.h>

#include "message.h"
#include "ttt_game.h" 

#define QUEUE_SIZE 8

//string representing the current state of the board
char *board;

volatile int active = 1;

void handler(int signum)
{
    active = 0;
}

void install_handlers(void)
{
    struct sigaction act;
    act.sa_handler = handler;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);

    sigaction(SIGINT, &act, NULL);
    sigaction(SIGTERM, &act, NULL);
}

int open_listener(char *service, int queue_size)
{
    struct addrinfo hint, *info_list, *info;
    int error, sock;

    // initialize hints
    memset(&hint, 0, sizeof(struct addrinfo));
    hint.ai_family   = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags    = AI_PASSIVE;

    // obtain information for listening socket
    error = getaddrinfo(NULL, service, &hint, &info_list);
    if (error) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
        return -1;
    }

    // attempt to create socket
    for (info = info_list; info != NULL; info = info->ai_next) {
        sock = socket(info->ai_family, info->ai_socktype, info->ai_protocol);

        // if we could not create the socket, try the next method
        if (sock == -1) continue;

        // bind socket to requested port
        error = bind(sock, info->ai_addr, info->ai_addrlen);
        if (error) {
            close(sock);
            continue;
        }

        // enable listening for incoming connection requests
        error = listen(sock, queue_size);
        if (error) {
            close(sock);
            continue;
        }

        // if we got this far, we have opened the socket
        break;
    }

    freeaddrinfo(info_list);

    // info will be NULL if no method succeeded
    if (info == NULL) {
        fprintf(stderr, "Could not bind\n");
        return -1;
    }

    return sock;
}

void reap() {
    int pid;
    //repeatedly wait until all zombies are reaped
    do {
        pid = waitpid(-1, NULL, WNOHANG);
    } while(pid > 0);
}

int main(int argc, char **argv)
{
    signal(SIGPIPE, SIG_IGN);
    srand(time(NULL));

    struct sockaddr_storage remote_host;
    socklen_t remote_host_len;
    int player_num = 0;
    int sock1;
    int sock2;
    int r;
    int status = -1;
    int err;
    //char* message;
    char** args;
    char curr_move;
    //if there's an argument use it for the port number 
    //otherwise default to using port 15000
    char *service = argc == 2 ? argv[1] : "15000";
    board = malloc(sizeof(char) * 10);

	install_handlers();
	
    //listen for connections QUEUE_SIZE is 
    int listener = open_listener(service, QUEUE_SIZE);
    if (listener < 0) exit(EXIT_FAILURE);
    
    puts("Listening for incoming connections");

    
    //randomly generate a number to decide who goes first 
    // r = 0 represents player 1 turn 
    // r = 1 represents player 2 turn
    r = rand() % 2;

    //handler for the current message being read

    //initialize structs
    handle_t h;
    message_t m;
    handle_t *hPtr = &h;
    message_t *mPtr = &m;

    m.message = malloc(264);
    h.buf = malloc(264);

    char *player1_name;
    char *player1_len;
    int player1_name_len;



    char *player2_name;
    char *player2_len;
    int player2_name_len;

    int other_player;
    //should do this for every new game
    init_board(board);
    while (active) {
        remote_host_len = sizeof(remote_host);
        //wait for two players to join the current session before starting the game
        //FOR TESTING PURPOSES COMMENTING THIS OUT 
        while (player_num < 3) {
            if(player_num == 0) {
                sock1 = accept(listener, (struct sockaddr *)&remote_host, &remote_host_len);
                puts("player 1 connected");
                h.fd = sock1;
                //once the user is connected wait for the PLAY message from client which will determine what player1_name should be
                err = read_message(hPtr, mPtr);
                parse_message(mPtr);
                display_args(mPtr);
                //need to check if the name is too long if it is then send INVL|16|NAME IS TOO LONG|
                player1_len = m.args[1];
                player1_name_len = atoi(m.args[1]);
                if(player1_name_len > 255) { 
                    write(sock1, "INVL|14|NAME TOO LONG|", 22);
                    //disconnect from sock2
                    player_num--;
                } else {
                    player1_name = m.args[2];
                    //printf("sock1: %d\n", sock1);
                    //h1.sock = sock1;
                    write(sock1, "WAIT|0|", 8);
                }
            }
            if(player_num == 1) {
                sock2 = accept(listener, (struct sockaddr *)&remote_host, &remote_host_len);
                puts("player 2 connected");
                h.fd = sock2;
                err = read_message(hPtr, mPtr);
                parse_message(mPtr);
                display_args(mPtr);
                //need to check if the name is too long if it is then send INVL|16|NAME IS TOO LONG|
                player2_len = m.args[1];
                player2_name_len = atoi(m.args[1]);

                if(player2_name_len > 255) { 
                    write(sock2, "INVL|14|NAME TOO LONG|", 22);
                    //disconnect from sock2
                    player_num--;
                } else {
                    player2_name = m.args[2];
                    //printf("sock1: %d\n", sock1);
                    //h1.sock = sock1;
                    write(sock2, "WAIT|0|", 8);
                }
            }
            //once both players have connected send BEGN to both players with their role and their opponent's name X move first
            if(player_num == 2) { 
                if (r == 0) {
                    printf("Player 1 goes first\n");
                    char str1_to_send[strlen(player2_name) + 9];
                    //string representing the total length of the message that needs to be sent
                    //honestly this should just be programmed into the perform action method instead of being like this :(
                    char snum1[3];
                    //itoa(player1_name_len + 2, snum, 10);
                    //turn the length of the name into a string
                    sprintf(snum1, "%d", player2_name_len + 2);
                    
                    strcat(str1_to_send, "BEGN|");
                    strcat(str1_to_send, snum1);
                    strcat(str1_to_send, "|");
                    strcat(str1_to_send, "X|");
                    strcat(str1_to_send, player2_name);
                    strcat(str1_to_send, "|");
                    
                    printf("str1_to_send: %s\n", str1_to_send);
                    write(sock1, str1_to_send, strlen(str1_to_send));

                    char str2_to_send[strlen(player1_name) + 9];
                    char snum2[3];
                    sprintf(snum2, "%d", player1_name_len + 2);

                    strcat(str2_to_send, "BEGN|");
                    strcat(str2_to_send, snum2);
                    strcat(str2_to_send, "|");
                    strcat(str2_to_send, "O|");
                    strcat(str2_to_send, player1_name);
                    strcat(str2_to_send, "|");
                    
                    printf("str2_to_send: %s\n", str2_to_send);
                    write(sock2, str2_to_send, strlen(str2_to_send));
                } else {
                    printf("Player 2 goes first\n");
                    char str1_to_send[strlen(player1_name) + 9];
                    //string representing the total length of the message that needs to be sent
                    //honestly this should just be programmed into the perform action method instead of being like this :(
                    char snum1[3];
                    
                    sprintf(snum1, "%d", player1_name_len + 2);

                    strcat(str1_to_send, "BEGN|");
                    strcat(str1_to_send, snum1);
                    strcat(str1_to_send, "|");
                    strcat(str1_to_send, "X|");
                    strcat(str1_to_send, player2_name);
                    strcat(str1_to_send, "|");
                    
                    printf("str1_to_send: %s\n", str1_to_send);
                    write(sock1, str1_to_send, strlen(str1_to_send));

                    char str2_to_send[strlen(player2_name) + 9];
                    char snum2[3];
                    sprintf(snum2, "%d", player2_name_len + 2);

                    strcat(str2_to_send, "BEGN|");
                    strcat(str2_to_send, snum2);
                    strcat(str2_to_send, "|");
                    strcat(str2_to_send, "O|");
                    strcat(str2_to_send, player1_name);
                    strcat(str2_to_send, "|");
                    
                    printf("str2_to_send: %s\n", str2_to_send);
                    write(sock2, str2_to_send, strlen(str2_to_send));
                }
            }
            player_num++;
        }

        //close listener stop accepting incoming connections
        close(listener);

        //COMMENTING THIS OUT FOR TESTING
        //if a draw was proposed on the last message
        if(!r) { 
            h.fd = sock1;
            other_player = sock2;
            curr_move = 'X';
            r = 1;
            printf("PLAYER 1 TURN\n");
        } else if(r) { 
            h.fd = sock2;
            other_player = sock1;
            curr_move = 'Y';
            r = 0;
            printf("PLAYER 2 TURN\n");
        }

        err = read_message(hPtr, mPtr);

        //check if there was some error in reading the message
        //this could mean that the user didn't provide a correct header 
        //didn't provide a message with the correct number of fields
        //didn't provide a message didn't have the specified number of bits
        if(!err || err == -1){
            printf("there was an ERROR!\n");
            write(sock1, "INVL|12|BAD MESSAGE|", 20);
            //write(sock2, "INVL|12|BAD MESSAGE|", 20);
            printf("closing connection\n");
            break;
        } 

        //take discrete message from message and split it up into the different fields
        //the start of each field is defined by a '|' char
        parse_message(mPtr);

        //helper method to display the contents of the args list 
        //in the message struct
        display_args(mPtr);

        printf("message received from sock: %s\n", m.message);
        printf("message length: %d\n", m.length);
        printf("buffer received from sock: %s\n", h.buf);
        printf("buffer length: %d\n", h.length);
        
        printf("board: %s\n", board);

        //perform_action should return the status of the game 
        //1 means game can continue 
        //0 means game should end
        status = perform_action(m.args, board, h.fd, other_player);
        
        //free args
        free_args(mPtr);
        //free message
        free(m.message);
        //status = checkWin(grid);
        //check if the game is over
        if((status == 0) || (status == 1) || (status == 2) || (status == 3)){
            active = 0;
        }
    }
    switch (status)
    {
        case 0:
            puts("Draw");
            break;
        case 1:
            puts("X wins");
            break;
        case 2:
            puts("O wins");
            break;
        case 3:
            puts("Someone resigned");
            
        default:
            break;
    }

    //no need to fre this anymore since we are using struct to store the message
    //free(message);
    //free(m.message);
    puts("Shutting down");
    close(sock1);
    close(sock2);
    close(listener);

    return EXIT_SUCCESS;
}
