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
#include "ttt_game.h"

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
int read_message(handle_t *h, message_t *m)
{
    char buf[BUFSIZE + 1];
    //get the socket from the handler struct
    int sock = h->fd;
    int bytes, error, i;
    int total_bytes = 0;
    int count = 0;
    //the expected number of fields for this specific message code
    int expected;
    char* message = m->message; 
    char* buffer = h->buf;
    int active = 1;

    //read to buffer
    bytes = read(sock, buf, BUFSIZE);
    printf("read %d bytes: %s\n", bytes, buf);

    //bytes = read(sock, buf, BUFSIZE);

    //check the first four characters of the buf this should be the code
    char msg_header[5];
    strncpy(msg_header, buf, 4);
    msg_header[4] = '\0';
    printf("msg_header: %s\n", msg_header);

    if(strcmp(msg_header, "PLAY") == 0) {
        expected = 3;
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

    int len[3];
    int exp_len = 0;
    int places = 0;
    //check the next field to see how many bytes the rest of the message should be
    i = 5;
    
    while(buf[i] != '|') {
        printf("buf[i]: %c\n", buf[i]);
        int curr = buf[i] - '0';
        printf("curr: %d\n", curr);
        //check if the digit is a valid one
        if(curr > 9 || curr < 0) { 
            return -1;
        }    

        len[i - 5] = curr;
        places++;
        i++;
    }

    if(places == 1) { 
        exp_len = len[0];
    } else if(places == 2) { 
        exp_len = (len[0]*10) + len[1];
    } else { 
        exp_len = (len[0]*100) + (len[1]+10) + len[2];
    }

    //check if exp_len <= 255
    if(exp_len > 255) { 
        return -1;
    }
    exp_len += bytes;

    do {
        printf("read %d bytes: %s\n", bytes, buf);
        //return -1;
        //iterate over buffer to find if it has the right number of '|'
        
        /*
        i = size;
        while(count != expected) {
            printf("count: %d\n", count);
            //we've encountered a '|' symbol meaning a field has ended
            if(buf[i] == '|') {
                count++;
            }
            i++; 
            //once count == expected we should stop
            //these bytes should be copied to 
        }
        */
    
        //loop to iterate through the current part of the buffer
        for(i = total_bytes; i < (bytes + total_bytes); i++) {
            if(buf[i] == '|') { 
                count++; 
            }
        }

        if(count == expected) { 
            active = 0;
            strncpy(message, buf, i);
        }
        total_bytes += bytes;
        //copy the first size bytes from buf to message
        //the rest of the string should be copied after this
        //printf("final value of i: %d\n", i);
        //copy the first i bytes to the message from the buffer
        //we continue reading from the buffer to see if there's any extra info sent that should all be stored in the
        //printf("number of bytes: %d\n", bytes);
        //we should only be calling read again in the case that 
        //we have not reached a '|' yet and we're still below the expected number of bytes
    } while(active && ((bytes = read(sock, buf + total_bytes, BUFSIZE - total_bytes)) > 0));

    //while((active) && (total_bytes < exp_len) && ((bytes = read(sock, buf + total_bytes, BUFSIZE - total_bytes)) > 0));
    //int addl_bytes = read(sock, buf + bytes, 1);
    //printf("addl_bytes: %d\n", addl_bytes);

    printf("DONE READING MESSAGE!\n");
    printf("i: %d\n", i);
    printf("total_bytes: %d\n", total_bytes);
    //strncpy(message, buf, i);
    //strncpy(buffer, buf, size);
    strncpy(buffer, buf, BUFSIZE - 1);
    message[i - 1] = '\0';
    buffer[i - 1] = '\0';
    h->length = i;
    m->length = total_bytes;
    m->fields = expected;
    //message[i + 1] = '\0';
    return 1;
}

//argument is the string message received from either client or server
//this should determine what type of message it is and return an array with the arguments
//this message should be a valid one
void parse_message(message_t *m) { 
    char** args = m->args;
    char* message = m->message;
    //placeholder for the current string
    char* curr_string;
    //counter for the size of the current string
    int curr_size = 0;
    int size = 10;
    //arg count
    int count = 0;
    int i = 0;
    args = malloc(sizeof(char *) * m->fields);
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
    m->args = args;
    printf("DONE PARSING MESSAGE\n");
}

void free_args(message_t *m) { 
    for(int i = 0; i < m->fields; i++) {
        free(m->args[i]);
    }
    free(m->args);
}

void display_args(message_t *m) { 
    printf("fields: %d\n", m->fields);
    for(int i = 0; i < m->fields; i++) {
        printf("args[%d]: %s\n", i, m->args[i]);
    }
}


//given the msg_type and args perform the appropriate response
//fd is the fd to write to
//return value: 
//-1: if failed for any eason
//the position of the move if possible
//0 ow
void perform_action(char **args, char* board, int fd) {
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
        if(strcmp(args[0], "PLAY") == 0) {
            //do something
            free(args[0]);
            free(args[1]);
            free(args[2]);
            write(fd, "WAIT|0|", 8);
        } else if(strcmp(args[0], "MOVE") == 0) {
            //do something
            //check if it's a valid move
            //if it is then fill space in the grid to 
            //ow send INVALID|23|THAT SPACE IS OCCUPIED
            int x = args[3][0] - '0';
            int y = args[3][2] - '0';
            int valid = valid_move(board, x, y, args[2][0]);
            if(valid) { 
                //build message
                str_to_send = "MOVD|16|";
                strcat(str_to_send, args[2]);
                strcat(str_to_send, "|");
                strcat(str_to_send, args[3]);
                strcat(str_to_send, "|");
            } else if(!valid){
                str_to_send = "INVALID|23|THAT SPACE IS OCCUPIED|";
            } else {
                str_to_send = "INVALID|15|INVALID COORDS|";
            }
            write(fd, str_to_send, strlen(str_to_send));
        } else if(strcmp(args[0], "RSGN") == 0) {
            //do something
            //send the 
        } else if(strcmp(args[0], "DRAW") == 0) {
            //do something
        }
    }
    free(args);
}