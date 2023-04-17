//application layer 
//

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

#include "messages.h"

enum msg{PLAY, WAIT, BEGN, MOVE, MOVD, INVL, RSGN, DRAW};
enum msg msg_type;
int fd1, fd2;

void init(int sock1, int sock2) {
    fd1 = sock1;
    fd2 = sock2;
}

//argument is the string message received from either client or server
//this should determine what type of message it is and return an array with the arguments
char** parse_message(char *message) { 
    char *args[] = {"Hello"};
    int i = 0;
    while(message[i] != '\0') {
        printf("char at message[%d]: %c\n", i, message[i]);
        i++;
    }
    return args;
}

//given the msg_type and args perform the appropriate response
void perform_action(char **args) {
    //TODO: fill in implementation
    printf("hello world\n");
}