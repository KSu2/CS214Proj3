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

#define QUEUE_SIZE 8

//string representing the current state of the board
char *board;
// Struct to hold player data
struct player {
    int id;
    int socket;
    char* name; 
};
/*
    Stores game data like: 
    board
    players' id
    mutex (for protection)
    cond (when player makes a move)
*/

struct game_data
{
   char *board;
   int player1_id, player2_id;
   pthread_mutex_t mutex;
   pthread_cond_t cond;
};

struct queue
{
    int players[100];
    int head;
    int tail;
};


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

// Function to add player ID to queue
void enqueue(struct queue* q, int id) {
    if (q->tail == 100) {
        printf("Queue is full\n");
    } else {
        q->players[q->tail] = id;
        q->tail++;
    }
}

// Function to remove player ID from queue
int dequeue(struct queue* q) {
    if (q->head == q->tail) {
        printf("Queue is empty\n");
        return -1;
    } else {
        int id = q->players[q->head];
        q->head++;
        return id;
    }
}
void game_thread(void* args){
    struct game* g = (struct game*)args;
    
    //Insert game code here from main() function

    pthread_exit(NULL);
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

    struct queue* q = malloc(sizeof(struct queue));
    q->head=0;
    q->tail=0;
    

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

    char *player1_name;
    char *player1_len;
    int player1_name_len;

    int draw_suggested = 0;
    int ask_again = 0;

    char *player2_name;
    char *player2_len;
    int player2_name_len;
    int player_id=1;

    int other_player;
    //should do this for every new game
    init_board(board);
    while (active) {
        remote_host_len = sizeof(remote_host);
        //wait for two players to join the current session before starting the game
        //FOR TESTING PURPOSES COMMENTING THIS OUT
        /*
        code for adding players to queue to wait for a second player then dequeue them 
        to start a game thread after player connection
    
        player* p = malloc(sizeof(struct player));
        p->id = player_id;
        p->socket = client_socket 
        enqueue(q,player_id);
        player_id++;

        // Check if there are enough players for a game
        if (q->rear - q->front >= 2) {
            // Dequeue two players to start a game
            int player1_id = dequeue(q);
            int player2_id = dequeue(q);

            // Create game struct and initialize data
            struct game* g = malloc(sizeof(struct game));
            g->player1_id = player1_id;
            g->player2_id = player2_id;
            g->turn = 0;
            g->winner = 0;
            pthread_mutex_init(&g->mutex, NULL);
            pthread_cond_init(&g->cond, NULL);

            // Create game thread
            pthread_t tid;
            pthread_create(&tid, NULL, game_thread, g);
        }        
        
        Next we need to add part of the code below to the method in game thread
        
        while game is still running{
            pthread_mutex_lock(&g->mutex);
            while (g->turn != 0) {
                pthread_cond_wait(&g->cond, &g->mutex);
            }
            // Update game board
        

            // Signal other player
            g->turn = 1;
            pthread_cond_signal(&g->cond);
            pthread_mutex_unlock(&g->mutex);
        }

        We need to wait for the game thread to exit so we use

        pthread_join(tid, NULL);

        */

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
                    display_args(mPtr);
                    player1_name = m.args[2];
                    player1_len = m.args[1];
                    player1_name_len = atoi(m.args[1]);
                    //printf("sock1: %d\n", sock1);
                    //h1.sock = sock1;
                    write(sock1, "WAIT|0|", 8);
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
                    player2_len = m.args[1];
                    player2_name_len = atoi(m.args[1]);
                    //printf("sock1: %d\n", sock1);
                    //h1.sock = sock1;
                    write(sock2, "WAIT|0|", 8);
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
                    
                    printf("str1_to_send: %s\n", str_to_send);
                    write(STDOUT_FILENO, str_to_send, strlen(str_to_send));
                    printf("\n");

                    char str2_to_send[strlen(player1_name) + 12];
                    char snum2[3];
                    sprintf(snum2, "%d", player1_name_len + 2);

                    strcpy(str_to_send, "BEGN|");
                    strcat(str_to_send, snum2);
                    strcat(str_to_send, "|");
                    strcat(str_to_send, "O|");
                    strcat(str_to_send, player1_name);
                    strcat(str_to_send, "|");
                    
                    printf("str2_to_send: %s\n", str_to_send);
                    write(STDOUT_FILENO, str_to_send, strlen(str_to_send));
                    printf("\n");
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
                    
                    printf("str_to_send: %s\n", str_to_send);
                    write(STDOUT_FILENO, str_to_send, strlen(str_to_send));
                    printf("\n");

                    char snum2[3];
                    sprintf(snum2, "%d", player2_name_len + 2);

                    strcpy(str_to_send, "BEGN|");
                    strcat(str_to_send, snum2);
                    strcat(str_to_send, "|");
                    strcat(str_to_send, "O|");
                    strcat(str_to_send, player2_name);
                    strcat(str_to_send, "|");

                    printf("str2_to_send: %s\n", str_to_send);
                    write(STDOUT_FILENO, str_to_send, strlen(str_to_send));
                    printf("\n");
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

        //close listener stop accepting incoming connections
        close(listener);

        //COMMENTING THIS OUT FOR TESTING
        //if a draw was proposed on the last message
        if(!r && !ask_again) { 
            h.fd = sock1;
            other_player = sock2;
            curr_move = 'X';
            r = 1;
            printf("PLAYER 1 TURN\n");
        } else if(r && !ask_again) { 
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
            write(h.fd, m.message, strlen(m.message));
            free(m.message);
            free(h.buf);
            //write(sock2, "INVL|12|BAD MESSAGE|", 20);
            printf("closing connection\n");
            break;
        } 

        //take discrete message from message and split it up into the different fields
        //the start of each field is defined by a '|' char
        parse_message(mPtr);
        printf("DONE PARSING\n");

        //helper method to display the contents of the args list 
        //in the message struct
        display_args(mPtr);

        //printf("message received from sock: %s\n", m.message);
        //printf("message length: %d\n", m.length);
        //printf("buffer received from sock: %s\n", h.buf);
        //printf("buffer length: %d\n", h.length);
        
        //printf("board: %s\n", board);

        //perform_action should return the status of the game 
        //1 means game can continue 
        //0 means game should end

        status = perform_action(m.args, board, h.fd, other_player, draw_suggested);
        printf("status: %d\n", status);
        
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

        printf("draw_suggested: %d\n", draw_suggested);
        printf("ask_again: %d\n", ask_again);
        
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

    puts("Shutting down");
    close(sock1);
    close(sock2);
    close(listener);

    return EXIT_SUCCESS;
}
