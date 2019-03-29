#include "definition.h"

int main(){
    char errMsg[SIZE_SECTOR];
    char argv[128][128];
    char tempFile[512];
    char tempData[512];
    char argc;
    int i;
    int result;
    char currDir;

    interrupt(0x21,0xFF<<8|0x04,errMsg,"e.msg",0);

    interrupt(0x21,0x21,&currDir,0,0);

    interrupt(0x21,0x22,&argc,0,0); // ambil argc
    for(i=0;i<argc;++i){ // copy semua isi argv ke temp
        interrupt(0x21,0x23,i,argv[i],0);
    }

    if(argc>1 && stringCompare(argv[1],"-w",2)){
        result = 1;
        interrupt(0x21,currDir<<8|0x05,"test",argv[0],&result); // writeFile
        if(result==ERROR_INSUFFICIENT_SECTORS){
            interrupt(0x21,0x00,errMsg+EMSG_INSUFFICIENT*SIZE_EMSG_ENTRY,0,0); 
            interrupt(0x21,0x00,errMsg+EMSG_SECTORS*SIZE_EMSG_ENTRY,0,0);
        }else if(result == ERROR_NOT_FOUND){
            interrupt(0x21,0x00,errMsg+EMSG_NOT_FOUND*SIZE_EMSG_ENTRY,0,0);
        }else if(result == ERROR_ALREADY_EXISTS){
            interrupt(0x21,0x00,errMsg+EMSG_ALREADY_EXIST*SIZE_EMSG_ENTRY,0,0);
        }else if(result == ERROR_INSUFFICIENT_ENTRIES){
            interrupt(0x21,0x00,errMsg+EMSG_INSUFFICIENT*SIZE_EMSG_ENTRY,0,0); 
            interrupt(0x21,0x00,errMsg+EMSG_ENTRIES*SIZE_EMSG_ENTRY,0,0);
        }else{
            interrupt(0x21,0x00,"Masukkan teks : ",0,0);
            interrupt(0x21,0x01,tempData,0,0); //readString
            interrupt(0x21,currDir<<8|0x09,argv[0],&result,0); //deleteFile
            result = 1;
            interrupt(0x21,currDir<<8|0x05,tempData,argv[0],&result); //writeFile
        }
    }else if(argc==1){
        interrupt(0x21,currDir<<8 | 0x04,tempFile,argv[0],&result);
        if(result==0){
            interrupt(0x21,0x00,tempFile,0,0);
            interrupt(0x21,0x00,"\n",0,0);
        }else if(result==ERROR_NOT_FOUND){
            interrupt(0x21,0x00,errMsg+EMSG_NOT_FOUND*SIZE_EMSG_ENTRY,0,0); 
        }
    }else{
        interrupt(0x21,0x00,"Usage cat <file> (-w)\n",0,0);
    }

    interrupt(0x21,0x07,0,0,0); // terminateProgram
    return 0;
}