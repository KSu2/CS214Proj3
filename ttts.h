struct player_list {
    char **names;
    int length;
    int max_size;
};

struct game_data
{
    int fd1, fd2;
    char *player1_name;
    char *player2_name; 

};

typedef struct player_list player_list_t;
typedef struct game_data game_data_t;
int in_names(player_list_t *list, char *name);
void add_player(player_list_t *list, char *name);
void remove_player(player_list_t *list, char *name);
void show_list(player_list_t *list);