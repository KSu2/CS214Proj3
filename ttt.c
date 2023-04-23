//tic tac toe client code

//display current state of the board
//report moves of the other player

/**

Messages sent by client:
- PLAY name : sent once a connection is established. The third field gives the name of the player. 
                The expected response is WAIT. The server will respond INVL if the name cannot be used
- MOVE : Indicates a move made by a player. THe third field is the player's role and the fourth field is the grid cell they are claiming
            The server will respond with MOVD if the move is accepted or INVL if the move is not allowed
- RSGN role position : Indicates that the player has resigned
                        The server will respond with OVER
- DRAW message : Depending on the message, this indicates that the player is suggesting a draw (S), or is accepting (A) or rejecting (R) a draw proposed by their opponent. 
                    NOTE that DRAW A or DRAW R can only be sent in response to receiving a DRAW S from the server

*/

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>

#define BUFLEN 256

//connect to hostname with port number
int connect_inet(char *host, char *service)
{
    struct addrinfo hints, *info_list, *info;
    int sock, error;

    // look up remote host
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;  // in practice, this means give us IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // indicate we want a streaming socket

    error = getaddrinfo(host, service, &hints, &info_list);
    if (error) {
        fprintf(stderr, "error looking up %s:%s: %s\n", host, service, gai_strerror(error));
        return -1;
    }

    for (info = info_list; info != NULL; info = info->ai_next) {
        sock = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
        if (sock < 0) continue;

        error = connect(sock, info->ai_addr, info->ai_addrlen);
        if (error) {
            close(sock);
            continue;
        }

        break;
    }
    freeaddrinfo(info_list);

    if (info == NULL) {
        fprintf(stderr, "Unable to connect to %s:%s\n", host, service);
        return -1;
    }

    return sock;
}

#define BUFSIZE 256
#define HOSTSIZE 100
#define PORTSIZE 10
void read_data(int sock, struct sockaddr *rem, socklen_t rem_len)
{
    char buf[BUFSIZE + 1], host[HOSTSIZE], port[PORTSIZE];
    int bytes, error;

    error = getnameinfo(rem, rem_len, host, HOSTSIZE, port, PORTSIZE, NI_NUMERICSERV);
    if (error) {
        fprintf(stderr, "getnameinfo: %s\n", gai_strerror(error));
        strcpy(host, "??");
        strcpy(port, "??");
    }

    printf("Connection from %s:%s\n", host, port);

    while (((bytes = read(sock, buf, BUFSIZE)) > 0)) {
        buf[bytes] = '\0';
        printf("[%s:%s] read %d bytes |%s|\n", host, port, bytes, buf);
        //once we reach new line we're done reading
        if(buf[bytes - 1] == '\n') {
            break;
        }
    }

	if (bytes == 0) {
		printf("[%s:%s] got EOF\n", host, port);
	} else if (bytes == -1) {
		printf("[%s:%s] terminating: %s\n", host, port, strerror(errno));
	} else {
		printf("[%s:%s] terminating\n", host, port);
	}

    close(sock);
}

int main(int argc, char **argv) { 
    int sock, bytes, active = 1;
    
    struct sockaddr_storage remote_host;
    socklen_t remote_host_len;
    
    char buf[BUFLEN];

    //not enough arguments
    if (argc != 3) {
        printf("Specify host and service\n");
        exit(EXIT_FAILURE);
    }

    //attempt to establish connection with websocket 
    //with given host name and port number
    sock = connect_inet(argv[1], argv[2]);

    //after connection is successful send a PLAY message 
    //to the server with name of player

    //if failed to connect to socket
    if (sock < 0) { 
        printf("Failed to establish connection with Websocket\n");
        exit(EXIT_FAILURE);
    }

    //after connection start loop to read from the stdin
    while(active){
        remote_host_len = sizeof(remote_host);
        //get next line from the 
        while ((bytes = read(STDIN_FILENO, buf, BUFLEN)) > 0) {
            if(buf[bytes - 1] == '\n') {
                buf[bytes] = '\0';
                break;
            }
        }
        //write to serv
        write(sock, buf, bytes);
    
        /**
        printf("waiting for serv message...\n");
        read_message(sock, (struct sockaddr *)&remote_host, remote_host_len);
        */
    }

    close(sock);
    return EXIT_SUCCESS;
}