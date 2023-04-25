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
#include <pthread.h>
#include <signal.h>
#include <time.h>

#include "message.h"
#include "ttt_game.h"
#include "ttts.h"

#define QUEUE_SIZE 8

//GLOBAL VARIABLES
//string representing the current state of the board
int active;
char *board;
pthread_mutex_t mutex;
player_list_t players;
player_list_t *listPtr = &players;

void handler(int signum) {
    active = 0;
}

void install_handlers(void) {
    struct sigaction act;
    act.sa_handler = handler;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);

    sigaction(SIGINT, &act, NULL);
    sigaction(SIGTERM, &act, NULL);
}

int open_listener(char *service, int queue_size) {
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

void *game_thread(void* args) {
    game_data_t* g = (game_data_t*)args;
    
    //variable controlling whose turn it currently is
    int r = 0;
    int sock1 = g->fd1;
    int sock2 = g->fd2;
    char curr_move;

    int status = -1;
    int err;

    int other_player;

    //initialize structs
    //represent the current handler
    message_t m;
    //this should switch between h1 and h2 depending on whose turn it is
    handle_t *hPtr;
    message_t *mPtr = &m;

    //handler for player 1
    handle_t h1;
    h1.fd = sock1;
    h1.buf = malloc(264);
    h1.length = 0;
    //handler for player 2
    handle_t h2;
    h2.fd = sock2;
    h2.buf = malloc(264);
    h2.length = 0;

    int draw_suggested = 0;
    int ask_again = 0;

    char *player1_name = g->player1_name;
    char *player2_name = g->player2_name;

    char *board = malloc(sizeof(char) * 10);
    init_board(board);

    active = 1;

    while (active) {
        //COMMENTING THIS OUT FOR TESTING
        //if a draw was proposed on the last message
        if(!r && !ask_again) { 
            hPtr = &h1;
            other_player = sock2;
            curr_move = 'X';
            r = 1;
            //printf("PLAYER 1 TURN\n");
        } else if(r && !ask_again) { 
            hPtr = &h2;
            other_player = sock1;
            curr_move = 'O';
            r = 0;
            //printf("PLAYER 2 TURN\n");
        }

        err = read_message(hPtr, mPtr);

        //check if there was some error in reading the message
        //this could mean that the user didn't provide a correct header 
        //didn't provide a message with the correct number of fields
        //didn't provide a message didn't have the specified number of bits
        if(!err || err == -1){
            //printf("there was an ERROR!\n");
            write(hPtr->fd, m.message, strlen(m.message));
            free(m.message);
            //write(sock2, "INVL|12|BAD MESSAGE|", 20);
            //printf("closing connection\n");
            break;
        } 

        //take discrete message from message and split it up into the different fields
        //the start of each field is defined by a '|' char
        parse_message(mPtr);
        //printf("DONE PARSING\n");

        //helper method to display the contents of the args list 
        //in the message struct
        //display_args(mPtr);

        printf("message received from sock: %s\n", m.message);
        //printf("message length: %d\n", m.length);
        printf("buffer received from sock: %s\n", hPtr->buf);
        //printf("buffer length: %d\n", h.length);
        
        //printf("board: %s\n", board);

        //perform_action should return the status of the game 
        //1 means game can continue 
        //0 means game should end

        status = perform_action(m.args, board, hPtr->fd, other_player, draw_suggested, curr_move);
        //printf("status: %d\n", status);
        
        if(status == -2){
            draw_suggested = 1; 
        } else {
            draw_suggested = 0;
        }

        if(status == -1) {
            ask_again = 1; 
        } else {
            ask_again = 0;
        }

        //printf("draw_suggested: %d\n", draw_suggested);
        //printf("ask_again: %d\n", ask_again);
        
        //free args
        free_args(mPtr);
        //free message
        free(m.message);

        status = checkWin(board);
        //check if the game is over

        //STATUS
        //
        //-2 - DRAW WAS SUGGESTED
        //-1 - Game continues
        //0 - DRAW 
        //1 - PLAYER 1 wins 
        //2 - PLAYER 2 wins
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
    /**
    
    CODE TO REMOVE THE PLAYERS FROM THE LIST OF PLAYERS AS WELL AS FREEING RESOURCES

    */

    pthread_mutex_lock(&mutex); 

    //DEBUG
    //printf("REMOVING player1 : %s\n", player1_name);

    remove_player(listPtr, player1_name);
    free(player1_name);

    //DEBUG
    //printf("REMOVING player2 : %s\n", player2_name);

    remove_player(listPtr, player2_name);
    free(player2_name);

    //free buffers
    free(h1.buf);
    free(h2.buf);
    close(sock1);
    close(sock2);

    pthread_mutex_unlock(&mutex);

    printf("Game Over\n");
    free(board);

    //return EXIT_SUCCESS;
    pthread_exit(NULL);
}

//check if names is in name
int in_names(player_list_t *list, char *name) { 
    if(list->length == 0) { 
        //printf("players list is empty\n");
        return 0;
    } else {
        for(int i = 0; i < list->length; i++) { 
            //printf("list->names[%d]: %s\n", i, list->names[i]);
            //printf("name: %s\n", name);
            if(strcmp(list->names[i], name) == 0) { 
                return 1;
            }
        }
    }
    return 0;
}

void add_player(player_list_t *list, char *name) {
    char *str = malloc(strlen(name)*sizeof(char));
    strcpy(str, name);

    //check if the capacity of the list can fit adding another name
    if(list->max_size < list->length + 1) { 
        list->max_size*=2;
        list->names = realloc(list->names, list->max_size);
    }
    list->names[list->length] = str;
    list->length++;
    // return 0;
}

//NEED TO FINISH
void remove_player(player_list_t *list, char *name) { 
    //DEBUG
    //printf("list length before: %d\n", list->length);

    //iterate through players list until we reach a name == name
    int pos = 0;
    for(int i = 0; i < list->length; i++) { 
        //this is a match
        if(strcmp(list->names[i], name) == 0) {
            //printf("found: %s\n", list->names[i]);
            free(list->names[i]);
            pos = i;
        }
    }

    //shift all elements after the removed name back one position
    for(int i = pos - 1; i < list->length - 1; i++) {
        list->names[i] = list->names[i + 1];
    }

    list->length--;
    //DEBUG
    //printf("list length after: %d\n", list->length);
}

void show_list(player_list_t *list) { 
    for(int i = 0; i < list->length; i++) { 
        printf("list[%d]: %s\n", i, list->names[i]);
    }
}

int main(int argc, char **argv) {
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

    //initialize structs
    handle_t h;
    message_t m;
    handle_t *hPtr = &h;
    message_t *mPtr = &m;

    h.buf = malloc(264);
    h.length = 0;

    //randomly generate a number to decide who goes first 
    // r = 0 means player1 goes first
    // r = 1 means player2 goes first
    r = rand() % 2;

    char *player1_name;
    char *player1_len;
    int player1_name_len;

    int draw_suggested = 0;
    int ask_again = 0;

    char *player2_name;
    char *player2_len;
    int player2_name_len;   

    players.length = 0;
    //init size of names list with size 2 initially
    players.names = malloc(sizeof(char *) * 2); 
    players.max_size = 2;

    while (1) {
        remote_host_len = sizeof(remote_host);
        if(players.length <= 252) { 
            //array storing the fds of the two players 
            //sock_fds[0] - Player going first (X)
            //sock_fds[1] - Player going second (O)
            game_data_t g;
            game_data_t *gPtr = &g;

            player_num = 0;
            //wait for at least two players to successfully connect to the server and send a valid |PLAY| message
            //needs to have length < 255
            //and name needs to not be in the current players list
            while (player_num < 3) {
                if(player_num == 0) {
                    sock1 = accept(listener, (struct sockaddr *)&remote_host, &remote_host_len);
                    puts("player 1 connected");
                    h.fd = sock1;
                    //once the user is connected wait for the PLAY message from client which will determine what player1_name should be
                    err = read_message(hPtr, mPtr);

                    if(err == -1) { 
                        write(sock1, m.message, strlen(m.message));
                        close(sock1);
                        player_num--;
                    } else { 
                        printf("message received from sock1: %s\n", m.message);

                        parse_message(mPtr);
                        //display_args(mPtr);

                        player1_name = m.args[2];
                        //strcpy(player1_name, m.args[2]);
                        //printf("in list: %d\n", in_names(listPtr, player1_name));
                        //check if name is in existing names list
                        if(in_names(listPtr, player1_name)) {
                            player_num--;
                            write(sock1, "INVL|20|NAME ALREADY IN USE|", 28);
                        } else { 
                            player1_len = m.args[1];
                            player1_name_len = atoi(m.args[1]);

                            //add this player to the players list
                            add_player(listPtr, player1_name);

                            write(sock1, "WAIT|0|", 8);
                        }
                    }
                } 
                else if(player_num == 1) {
                    sock2 = accept(listener, (struct sockaddr *)&remote_host, &remote_host_len);
                    puts("player 2 connected");
                    h.fd = sock2;
                    err = read_message(hPtr, mPtr);

                    if(err == -1) { 
                        write(sock1, m.message, strlen(m.message));
                        close(sock1);
                        player_num--;
                    } else { 
                        printf("message received from sock2: %s\n", m.message);

                        parse_message(mPtr);
                        display_args(mPtr);
                        player2_name = m.args[2];
                        //strcpy(player2_name, m.args[2]); 
                        printf("in list: %d\n", in_names(listPtr, player1_name));

                        //check if name is in existing names list
                        if(in_names(listPtr, player2_name)) {
                            player_num--;
                            write(sock2, "INVL|20|NAME ALREADY IN USE|", 28);
                        } else { 
                            player2_len = m.args[1];
                            player2_name_len = atoi(m.args[1]);

                            //add this player to the players list
                            add_player(listPtr, player2_name);

                            write(sock1, "WAIT|0|", 8);
                        }
                    }
                }
                //once both players have connected send BEGN to both players with their role and their opponent's name X move first
                else if(player_num == 2) { 
                    //only for testing delete after
                    // r = 1;
                    if (r == 0) {
                        printf("Player 1 goes first\n");
                        char str_to_send[264];
                        //string representing the total length of the message that needs to be sent
                        //honestly this should just be programmed into the perform action method instead of being like this :(
                        
                        char snum1[3];
                        sprintf(snum1, "%d", player2_name_len + 2);
                        
                        strcat(str_to_send, "BEGN|");
                        strcat(str_to_send, snum1);
                        strcat(str_to_send, "|");
                        strcat(str_to_send, "X|");
                        strcat(str_to_send, player2_name);
                        strcat(str_to_send, "|");
                        
                        //printf("str1_to_send: %s\n", str_to_send);
                        //write(STDOUT_FILENO, str_to_send, strlen(str_to_send));
                        //printf("\n");

                        char str2_to_send[strlen(player1_name) + 12];
                        char snum2[3];
                        sprintf(snum2, "%d", player1_name_len + 2);

                        strcpy(str_to_send, "BEGN|");
                        strcat(str_to_send, snum2);
                        strcat(str_to_send, "|");
                        strcat(str_to_send, "O|");
                        strcat(str_to_send, player1_name);
                        strcat(str_to_send, "|");
                        
                        //printf("str2_to_send: %s\n", str_to_send);
                        //write(STDOUT_FILENO, str_to_send, strlen(str_to_send));
                        //printf("\n");

                        g.fd1 = sock1;
                        g.fd2 = sock2;

                        g.player1_name = malloc(strlen(player1_name));
                        strcpy(g.player1_name, player1_name);
                        g.player2_name = malloc(strlen(player2_name));
                        strcpy(g.player2_name, player2_name);

                    } else {
                        printf("Player 2 goes first\n");
                        char str_to_send[264];
                        //string representing the total length of the message that needs to be sent
                        //honestly this should just be programmed into the perform action method instead of being like this :(
                        
                        char snum1[3];
                        sprintf(snum1, "%d", player1_name_len + 2);

                        strcat(str_to_send, "BEGN|");
                        strcat(str_to_send, snum1);
                        strcat(str_to_send, "|");
                        strcat(str_to_send, "X|");
                        strcat(str_to_send, player1_name);
                        strcat(str_to_send, "|");
                        
                        //printf("str_to_send: %s\n", str_to_send);
                        //write(STDOUT_FILENO, str_to_send, strlen(str_to_send));
                        //printf("\n");

                        char snum2[3];
                        sprintf(snum2, "%d", player2_name_len + 2);

                        strcpy(str_to_send, "BEGN|");
                        strcat(str_to_send, snum2);
                        strcat(str_to_send, "|");
                        strcat(str_to_send, "O|");
                        strcat(str_to_send, player2_name);
                        strcat(str_to_send, "|");

                        //printf("str2_to_send: %s\n", str_to_send);
                        //write(STDOUT_FILENO, str_to_send, strlen(str_to_send));
                        //printf("\n");

                        g.fd1 = sock2;
                        g.fd2 = sock1;

                        g.player1_name = malloc(strlen(player2_name));
                        strcpy(g.player1_name, player2_name);
                        g.player2_name = malloc(strlen(player1_name));
                        strcpy(g.player2_name, player1_name);
                    }
                    //actually I think we just need to free_args everytime not the message 
                    //message can just be a static block of memory sizeof(message) = 264 
                    //we just overwrite the data in the message everytime
                    //free args
                    free_args(mPtr);
                    //free message
                    free(m.message);
                }
                player_num++;
            }
            show_list(listPtr);
            //printf("gPtr->player1_name: %s\n", gPtr->player1_name);
            //printf("gPtr->player2_name: %s\n", gPtr->player2_name);
            pthread_t th;
            //third argument is args to pass to the game_thread subroutine
            //this should be a struct with the names and sock fds of the players
            int result = pthread_create(&th, NULL, game_thread, (void *)gPtr);
            if(result){
                printf("Thread creation failed with err no: %d\n", result);
                exit(EXIT_FAILURE);
            }
            
            //DEBUG statement
            printf("NEW GAME HAS STARTED!\n");
        }
    }


    puts("Shutting down");
    close(sock1);
    close(sock2);
    close(listener);

    return EXIT_SUCCESS;
}
