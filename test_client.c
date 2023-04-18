#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>

#ifndef DEBUG
#define DEBUG 0
#endif

#ifndef BUFSIZE
#define BUFSIZE 512
#endif

char *lineBuffer;
int linePos, lineSize, fd;

void dumpLine(int sock)
{
    assert(lineBuffer[linePos-1] == '\n');
    lineBuffer[linePos] = '\0';

    char *line = malloc(linePos);
    printf("command: \n");
    write(1, lineBuffer, linePos);

    write(sock, lineBuffer, linePos);

    free(line);
    free(lineBuffer);

    // dump output to stdout
    // FIXME should confirm that all bytes were written
}

void append(char *buf, int len)
{
    int newPos = linePos + len;
    
    if (newPos > lineSize) {
        lineSize *= 2;
        if (DEBUG) fprintf(stderr, "expanding line buffer to %d\n", lineSize);
        assert(lineSize >= newPos);
        lineBuffer = realloc(lineBuffer, lineSize);
        if (lineBuffer == NULL) {
            perror("line buffer");
            exit(EXIT_FAILURE);
        }
    }

    memcpy(lineBuffer + linePos, buf, len);
    linePos = newPos;
}

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

//Test client which will connect to client and send predetermined sequence of messages
int main(int argc, char **argv) { 
    int sock;

    //not enough arguments
    if (argc != 4) {
        printf("Specify host, service, and filename\n");
        exit(EXIT_FAILURE);
    }

    //attempt to establish connection with websocket 
    //with given host name and port number
    sock = connect_inet(argv[1], argv[2]);

    //after connection is successful send a PLAY message 
    //to the server with name of player

    fd = open(argv[3], O_RDONLY);

    //if failed to open fd
    if(fd == -1) { 
        perror("test_client: ");
        exit(EXIT_FAILURE);
    }

    //if failed to connect to socket
    if (sock < 0) { 
        printf("Failed to establish connection with Websocket\n");
        exit(EXIT_FAILURE);
    }

    int bytes, pos,lstart;
    char buffer[BUFSIZE];
    lineBuffer = malloc(BUFSIZE + 1);
    lineSize = BUFSIZE;
    linePos = 0;

    // read input
    while ((bytes = read(fd, buffer, BUFSIZE)) > 0) {
        // search for newlines
        lstart = 0;
        for (pos = 0; pos < bytes; pos++) {
            //printf("buffer[pos]: %c\n", buffer[pos]);
            if (buffer[pos] == '\n') {
                int thisLen = pos - lstart + 1;
                if (DEBUG) fprintf(stderr, "finished line %d+%d bytes\n", linePos, thisLen);
                //printf("NEWLINE\n");
                append(buffer + lstart, thisLen);
                //send lineBuffer to the sock
                dumpLine(sock);

                //wait for response from server
                //read_message(sock, (struct sockaddr *)&remote_host, remote_host_len);
                linePos = 0;
                lstart = pos + 1;
            }
        }
        if (lstart < bytes) {
            // partial line at the end of the buffer
            int thisLen = pos - lstart;
            if (DEBUG) fprintf(stderr, "partial line %d+%d bytes\n", linePos, thisLen);
            append(buffer + lstart, thisLen);
        }
    }
    if (linePos > 0) {
        // file ended with partial line
        append("\n", 1);
        dumpLine(sock);
    }
    close(fd);
    close(sock);
    return EXIT_SUCCESS;
}