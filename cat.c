#include "definition.h"

int main(){
    char argv[128][128];
    char tempFile[512];
    char tempData[512];
    // char temp[10];
    char argc;
    int i;
    int result;
    char currDir;

    interrupt(0x21,0x21,&currDir,0,0);

    interrupt(0x21,0x22,&argc,0,0); // ambil argc
    for(i=0;i<argc;++i){ // copy semua isi argv ke temp
        interrupt(0x21,0x23,i,argv[i],0);
    }

    if(argc>1 && stringCompare(argv[1],"-w",2)){
        result = 1;
        interrupt(0x21,currDir<<8|0x05,"abcefghijklmn",argv[0],&result); // writeFile
        if(result==INSUFFICIENT_SECTORS){
            interrupt(0x21,0x00,"Insufficient sectors\n",0,0); // printString
        }else if(result == NOT_FOUND){
            interrupt(0x21,0x00,"Not found\n",0,0);
        }else if(result == ALREADY_EXISTS){
            interrupt(0x21,0x00,"Already exists\n",0,0);
        }else if(result == INSUFFICIENT_ENTRIES){
            interrupt(0x21,0x00,"Insufficient entries\n",0,0);
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
        }else if(result==NOT_FOUND){
            interrupt(0x21,0x00,"Not found\n",0,0);
        }
    }else{
        interrupt(0x21,0x00,"Usage cat <file> (-w)\n",0,0);
    }

    interrupt(0x21,0x07,0,0,0); // terminateProgram
    return 0;
}