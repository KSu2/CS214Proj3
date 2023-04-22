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

#include "message.h"

#define BUFSIZE 264
#define HOSTSIZE 100
#define PORTSIZE 10

//handle stores:
//the entire buffer (with the current message and any fragments of the next message)
//length - the length of the buffer
//sock - the socket the message was read from 

//message stores: 
//the discrete message that needs to be used to parse message
//the length of the message

//method to read message from socket
//write message to message
//return 0 or -1 on failure
//return 1 on success
int receive_message(handle_t handler, message_t message)
{
    char buf[BUFSIZE + 1]
    //get the socket from the handler struct
    int sock = handler.sock;
    int bytes, error, i;
    int size = 0;
    int count = 0;
    //the expected number of fields for this specific message code
    int expected;
    char* message; 
    char* size;

    /**
    //address-to-name translation
    error = getnameinfo(rem, rem_len, host, HOSTSIZE, port, PORTSIZE, NI_NUMERICSERV);
    if (error) {
        fprintf(stderr, "getnameinfo: %s\n", gai_strerror(error));
        strcpy(host, "??");
        strcpy(port, "??");
    }

    printf("Connection from %s:%s\n", host, port);
    */

    //read to buffer
    bytes = read(sock, buf + size, BUFSIZE);

    //check the first four characters of the buf this should be the code
    char msg_header[5];
    strncpy(msg_header, message, 4);
    msg_header[4] = '\0';
    printf("msg_header: %s\n", msg_header);

    if(strcmp(msg_header, "PLAY") == 0) {
        epxected = 3;
    } else if(strcmp(msg_header, "MOVE") == 0) {
        expected = 4;
    } else if(strcmp(msg_header, "RSGN") == 0) {
        expected = 1;
    } else if(strcmp(msg_header, "DRAW") == 0) {
        expected = 3;
    } else {
        //this is an invalid message
        return -1;
    }

    char len_mess[3];
    //check the next field to see how many bytes the rest of the message should be
    i = 5;
    while(buf[i] != '|') {
        len_mess[i - 5] = buf[i]; 
        i++;
    }

    int len = atoi(len_mess);
    printf("expected length of message: %d\n", len);

    message = malloc(BUFSIZE);
    while (((bytes = read(sock, buf + size, BUFSIZE)) > 0)) {
        printf("[%s:%s] read %d bytes |%s|\n", host, port, bytes, buf);
        size += bytes;
        //iterate over buffer to find if it has the right number of '|'
        for(i = size; i < bytes; i++) {
            //we've encountered a '|' symbol meaning a field has ended
            if(count == expected) { 
                //should keep track of where in the buffer we are
                break;
            } else if(buf[i + size] == '|') {
                count++;
            }
            //once count == expected we should stop
            //these bytes should be copied to 
        }
        //copy the first size bytes from buf to message
        //the rest of the string should be copied after this
        strncpy(message, buf, size);
    }

    /**
	if (bytes == 0) {
		printf("[%s:%s] got EOF\n", host, port);
	} else if (bytes == -1) {
		printf("[%s:%s] terminating: %s\n", host, port, strerror(errno));
	} else {
		printf("[%s:%s] terminating\n", host, port);
	}
    */

    return message;
}

/**
//argument is the string message received from either client or server
//this should determine what type of message it is and return an array with the arguments
char** parse_message(char *message) { 
    char msg_header[5];
    char** args;
    //placeholder for the current string
    char* curr_string;
    //counter for the size of the current string
    int curr_size = 0;
    int size = 10;
    //arg count
    int count = 0;
    int i = 0;
    int expected;
    //copy the first 4 bits from message to msg_header if it is a valid message it will be one of the valid msg headers
    strncpy(msg_header, message, 4);
    msg_header[4] = '\0';
    printf("msg_header: %s\n", msg_header);

    if(strcmp(msg_header, "PLAY") == 0) {
        args = malloc(sizeof(char *) * 2);
        expected = 3;
    } else if(strcmp(msg_header, "MOVE") == 0) {
        args = malloc(sizeof(char *) * 4);
    } else if(strcmp(msg_header, "RSGN") == 0) {
        args = malloc(sizeof(char *));
    } else if(strcmp(msg_header, "DRAW") == 0) {
        args = malloc(sizeof(char *) * 2);
    } else {
        //invalid message return INVL
        args = malloc(sizeof(char *));
        char* str = malloc(4);
        strcpy(str, "INVL");
        args[0] = str;
        return args;
    }

    //build args list
    curr_string = malloc(sizeof(char) * size);
    while(message[i] != '\0') {
        printf("char at message[%d]: %c\n", i, message[i]);
        //if we are at the symbol '|' and it is not the first arg allocate a new char* pointer
        if(message[i] == '|') { 
            curr_string[curr_size] = '\0';
            args[count] = curr_string;
            curr_string = malloc(sizeof(char) * 10);
            size = 10;
            curr_size = 0;
            count++;
        } else if(curr_size > (size - 1)){
            //size of curr_string exceeds the 
            size *= 2;
            curr_string = realloc(curr_string, size);
            curr_string[curr_size] = message[i];
        } else {
            curr_string[curr_size] = message[i];
            curr_size++;
        }
        i++;
    }
    
    return args;
}
*/

//given the msg_type and args perform the appropriate response
//fd is the fd to write to
//return value: 
//-1: if failed for any eason
//the position of the move if possible
//0 ow
void perform_action(char **args, int fd) {
    //TODO: fill in implementation 
    char* str_to_send;
    //check if previous message was invalid message first 
    if(strcmp(args[0], "INVL") == 0) {
        //write to client INVL
        printf("INVALID MESSAGE\n");
        free(args[0]);
        str_to_send = "INVL|23|INVALID MESSAGE FORMAT|";
        write(fd, str_to_send, strlen(str_to_send));
    } else if ((atoi(args[1]) > 255) || (atoi(args[1]) < 0)){
        //message length field should be between 0 - 255 
        //ow write the INVL message
        free(args[0]);
        str_to_send = "INVL|13|LEN TOO LONG|";
        write(fd, str_to_send, strlen(str_to_send));
    } else {
        printf("VALID MESSAGE\n");
        printf("args[0]: %s\n", args[0]);
        printf("args[1]: %s\n", args[1]);
        printf("args[2]: %s\n", args[2]);
        if(strcmp(msg_header, "PLAY") == 0) {
            //do something
            free(args[0]);
            free(args[1]);
            free(args[2]);
            write(fd, "WAIT", 5);
        } else if(strcmp(msg_header, "MOVE") == 0) {
            //do something
            //check if it's a valid move
            //if it is then fill space in the grid to 
            //ow send INVALID|23|THAT SPACE IS OCCUPIED
            int valid = 
            if(valid) { 
                str_to_send = "MOVD| | |"
            } else {
                str_to_send = "INVALID|23|THAT SPACE IS OCCUPIED|";
            }
            write(fd, str_to_send, strlen(str_to_send));
        } else if(strcmp(msg_header, "RSGN") == 0) {
            //do something
            //send the 
        } else if(strcmp(msg_header, "DRAW") == 0) {
            //do something
        }
    }
    free(args);
}