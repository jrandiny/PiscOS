#include "definition.h"

int main(){
    char temp[128];
    int res;
    char curDir;
    boolean succ;

    succ = false;
    interrupt(0x21,0x21,&curDir,0,0); // ambil directori sekarang
    interrupt(0x21,0x23,0,temp,0); // ambil argumen pertama

    interrupt(0x21,curDir<<8|0x09,temp,&res,0);

    if(res != NOT_FOUND){
        succ = true;
    }

    interrupt(0x21,curDir<<8|0x0A,temp,&res,0);

    if(res != NOT_FOUND){
        succ = true;
    }

    if(!succ){
        interrupt(0x21,0x00,"Not found",0,0);
    }

    interrupt(0x21,0x7,0,0,0); // terminateProgram
    return 0;
}