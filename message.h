char** parse_message(char *);
void perform_action(char **, int);
char* read_message(int sock, struct sockaddr *rem, socklen_t rem_len);