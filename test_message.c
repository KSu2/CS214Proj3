//test file for message.c

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

#define BUFSIZE 256
int main() {
    int fd = open(STDIN_FILENO, O_RDONLY);
    char buf[BUFSIZE];
    int bytes;
    while((bytes = read(fd, buf, BUFSIZE)) > 0) {
        //read from buffer
        
    }
}