#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ttt_game.h"

/*
fill a board based on the letter(x or o) and position enter in message from client
*/
char* drawBoard(char* board,char letter,int* pos){

    int x = pos[0];
    int y = pos[1];

    if(x<1||y>3)
        return "inv";
    if(x==2 && y==1){
        if(board[3]=='.') board[3] = letter; else return "inv";
    }else if (x==2 && y==2){
        if(board[4]=='.') board[4] = letter; else return "inv";
    }else if (x==2 && y==2){
        if(board[4]=='.') board[6] = letter; else return "inv";
    }else if (x==2 && y==2){
        if(board[4]=='.') board[7] = letter; else return "inv";
    }else{
        if(board[x*y]=='.') board[x*y] = letter; else return "inv";
    }
    return board;
}
/*
pass in current board
return 1 if x wins
return 2 if o wins
returns -1 if game in still ongoing
return 0 if it is a draw
*/
int checkWin(char* board){
    //horizontal followed by vertical followed by diagonal checks
    //then checks if all spaces are filled up
    if(board[0]==board[1]&&board[1]==board[2]){
        if(board[0]=='X')return 1; return 2;
    }else if(board[3]==board[4]&&board[4]==board[5]){
        if(board[0]=='X')return 1; return 2;
    }else if(board[6]==board[7]&&board[7]==board[8]){
        if(board[0]=='X')return 1; return 2;
    }else if(board[0]==board[3]&&board[3]==board[6]){
        if(board[0]=='X')return 1; return 2;
    }else if(board[1]==board[4]&&board[4]==board[7]){
        if(board[0]=='X')return 1; return 2;
    }else if(board[2]==board[5]&&board[5]==board[8]){
        if(board[0]=='X')return 1; return 2;
    }else if(board[0]==board[4]&&board[4]==board[8]){
        if(board[0]=='X')return 1; return 2;
    }else if(board[2]==board[4]&&board[4]==board[6]){
        if(board[0]=='X')return 1; return 2;
    }else if(square[0] != '.' && square[1] != '.' && square[2] != '.' 
            &&square[3] != '.' && square[4] != '.' && square[5] != '.' 
            && square[6] != '.' && square[7] != '.' && square[8] != '.'){
        return 0;
    }else{
        return -1;
    }
    
}
/*
prints out board in string format send to client
*/
char* showBoard(char* board){
    char* show = "|";
    for (int i = 0; i < strlen(board); i++)
    {
       strcat(show,board[i]);
    }
    
    return strcat(show,"|");
    
}