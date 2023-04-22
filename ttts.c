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

//array to store the grid of the game
//
static char grid[9] = {'.','.','.','.','.','.','.','.','.'};

volatile int active = 1;

//given row, col and grid check if the cell is free
int valid_move(int row, int col) {
    int free = 1;
    if((grid[((row - 1) + (col - 1))] == 'X') || (grid[((row - 1) + (col - 1))] == 'O')) free = 0;
    return free;
}

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

    char* message;
    char** args;
    //if there's an argument use it for the port number 
    //otherwise default to using port 15000
    char *service = argc == 2 ? argv[1] : "15000";

	install_handlers();
	
    //listen for connections QUEUE_SIZE is 
    int listener = open_listener(service, QUEUE_SIZE);
    if (listener < 0) exit(EXIT_FAILURE);
    
    puts("Listening for incoming connections");
    r = rand() % 2;
    if (r == 0) {
        printf("Player 1 goes first\n");
    } else {
        printf("Player 2 goes first\n");
    }
    while (active && status == -1) {
        remote_host_len = sizeof(remote_host);
        //wait for two players to join the current session before starting the game
        while (player_num < 3) {
            if(player_num == 0) {
                sock1 = accept(listener, (struct sockaddr *)&remote_host, &remote_host_len);
                puts("player 1 connected");
                write(sock1, "WAIT", 5);
            }
            if(player_num == 1) {
                sock2 = accept(listener, (struct sockaddr *)&remote_host, &remote_host_len);
                puts("player 2 connected");
                write(sock1, "WAIT", 5);
            }
            player_num++;
        }
        //init(sock1, sock2);
        //randomly generate a number to decide who goes first 
        // r = 0 represents player 1 turn 
        // r = 1 represents player 2 turn
        close(listener);
        if(!r) { 
            message = read_message(sock1, (struct sockaddr *)&remote_host, remote_host_len);
            args = parse_message(message);
            perform_action(args, sock1);
            status = checkBoard(grid);
            r = 1;
        } else { 
            message = read_message(sock2, (struct sockaddr *)&remote_host, remote_host_len);
            args = parse_message(message);
            perform_action(args, sock2);
            status = checkBoard(grid);
            r = 0;
        }
        printf("message received from sock: %s\n", message);
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


    free(message);
    puts("Shutting down");
    close(sock1);
    close(sock2);
    close(listener);

    return EXIT_SUCCESS;
}
