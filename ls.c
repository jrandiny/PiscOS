#include "definition.h"

int main(){
    char name[MAX_FILENAME];
    char directoris[SIZE_SECTOR];
    char files[SIZE_SECTOR];
    char curDir;
    char argc;
    int i;

    enableInterrupts();
    interrupt(0x21,0x22,&argc,0,0); // ambil argc
    if(argc==0){
        interrupt(0x21,0x21,&curDir,0,0); // ambil directori sekarang
        interrupt(0x21,0x2,directoris,LOC_DIR_SECTOR,0); // readSector directori
        interrupt(0x21,0x2,files,LOC_FILE_SECTOR,0); // readSector file
        for(i=0;i<MAX_FILESYSTEM_ITEM_COUNT;i++){
            if(directoris[i*SIZE_DIR_ENTRY]==curDir && directoris[i*SIZE_DIR_ENTRY+1]!='\0'){
                stringCopy(directoris,name,i*SIZE_DIR_ENTRY+1,MAX_FILENAME);
                interrupt(0x21,0,name,0,0); // cetak namanya ke layar
                interrupt(0x21,0,"\n",0,0);
            }
        }
        for(i=0;i<MAX_FILESYSTEM_ITEM_COUNT;i++){
            if(files[i*SIZE_DIR_ENTRY]==curDir && files[i*SIZE_DIR_ENTRY+1]!='\0'){
                stringCopy(files,name,i*SIZE_DIR_ENTRY+1,MAX_FILENAME);
                interrupt(0x21,0,name,0,0); // cetak namanya ke layar
                interrupt(0x21,0,"\n",0,0);
            }
        }
    } else {
        interrupt(0x21,0x00,"Usage ls\n\r",0,0);
    }
    interrupt(0x21,0x7,0,0,0); // terminateProgram
    return 0;
}