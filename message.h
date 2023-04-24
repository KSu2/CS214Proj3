struct handle {
    char* buf;
    int length;
    int fd;
};

struct message { 
    char* message;
    char** args;
    int length;
    int fields;
};

typedef struct handle handle_t;
typedef struct message message_t;

void parse_message(message_t *m);
int perform_action(char **, char *, int, int);
int read_message(handle_t *h, message_t *m);
void display_args(message_t *m);
void free_args(message_t *m);
