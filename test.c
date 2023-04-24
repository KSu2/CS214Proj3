#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>

int main(int argc, char** argv) { 
    int test = argc > 1 ? atoi(argv[1]): 0;

    switch(test) { 
        default:
            puts("Missing or invalid test number");
            return EXIT_FAILURE;
        case 1:
            pid = fork();
            if (pid < 0) { 
                perror("A fork error has occured");
                exit(EXIT_FAILURE);
            }
            else if (pid == 0) {
                //in the child process
                execlp("./myshell", "myshell", "test1.sh", NULL);
                printf("execlp didn't work :( \n");
            }
            //clean up child process
            else {
                wait(NULL);
            }
            break;
        case 2:
            pid = fork();
            if (pid < 0) { 
                perror("A fork error has occured");
                exit(EXIT_FAILURE);
            }
            else if (pid == 0) {
                //in the child process
                execlp("./myshell", "myshell", "test1.sh", NULL);
                printf("execlp didn't work :( \n");
            }
            //clean up child process
            else {
                wait(NULL);
            }
            break;            
    }
}