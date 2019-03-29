#include "definition.h"

int main(){
    char temp[128];
    char print[2];
    char curDir;
    int suc;

    interrupt(0x21,0x23,0,temp,0); // ambil argumen pertama
    interrupt(0x21,0x21,&curDir,0,0); // ambil directori sekarang
    interrupt(0x21,curDir<<8 | 0x8,temp,&suc,0); // panggil makeDirectory
    if(suc==ERROR_INSUFFICIENT_SECTORS){
        interrupt(0x21,0x00,"Insufficient sectors\n",0,0); // printString
    }else if(suc == ERROR_NOT_FOUND){
        interrupt(0x21,0x00,"Not found\n",0,0);
    }else if(suc == ERROR_ALREADY_EXISTS){
        interrupt(0x21,0x00,"Already exists\n",0,0);
    }else if(suc == ERROR_INSUFFICIENT_ENTRIES){
        interrupt(0x21,0x00,"Insufficient entries\n",0,0);
    }
    interrupt(0x21,0x7,0,0,0); // terminateProgram
    return 0;
}