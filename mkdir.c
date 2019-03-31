#include "definition.h"

int main(){
    char errMsg[SIZE_SECTOR];
    char temp[128];
    char print[2];
    char curDir;
    char argc;
    int suc;

    interrupt(0x21,0xFF<<8|0x04,errMsg,"e.msg",0);
    interrupt(0x21,0x22,&argc,0,0); // ambil argc
    if(argc==1){ // ada argumen
        interrupt(0x21,0x23,0,temp,0); // ambil argumen pertama
        interrupt(0x21,0x21,&curDir,0,0); // ambil directori sekarang
        interrupt(0x21,curDir<<8 | 0x8,temp,&suc,0); // panggil makeDirectory
        if(suc == ERROR_NOT_FOUND){
            interrupt(0x21,0x00,errMsg+EMSG_NOT_FOUND*SIZE_EMSG_ENTRY,0,0);
        }else if(suc == ERROR_ALREADY_EXISTS){
            interrupt(0x21,0x00,errMsg+EMSG_ALREADY_EXIST*SIZE_EMSG_ENTRY,0,0);
        }else if(suc == ERROR_INSUFFICIENT_ENTRIES){
            interrupt(0x21,0x00,errMsg+EMSG_INSUFFICIENT*SIZE_EMSG_ENTRY,0,0);
            interrupt(0x21,0x00,errMsg+EMSG_ENTRIES*SIZE_EMSG_ENTRY,0,0);
        }
    } else {
        interrupt(0x21,0x00,"Usage makdir <directory>\n\r",0,0);
    }
    interrupt(0x21,0x7,0,0,0); // terminateProgram
    return 0;
}