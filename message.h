char** parse_message(char *);
void perform_action(char **, int);
char* read_message(int sock, struct sockaddr *rem, socklen_t rem_len);
struct handle {
    char* buf;
    int length;
    int fd;
};

struct message { 
    char* message;
    int length;
};

typedef struct handle handle_t;
typedef struct message message_t;
