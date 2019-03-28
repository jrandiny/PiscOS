#include "definition.h"

int main(){
    char name[MAX_FILENAME];
    char directoris[SECTOR_SIZE];
    char files[SECTOR_SIZE];
    char curDir;
    int i;

    interrupt(0x21,0x21,&curDir,0,0); // ambil directori sekarang
    interrupt(0x21,0x2,directoris,DIRS_SECTOR,0); // readSector directori
    interrupt(0x21,0x2,files,FILES_SECTOR,0); // readSector file
    for(i=0;i<MAX_FILESYSTEM_ITEM;i++){
        if(directoris[i*DIR_ENTRY_LENGTH]==curDir && directoris[i*DIR_ENTRY_LENGTH+1]!='\0'){
            stringCopy(directoris,name,i*DIR_ENTRY_LENGTH+1,MAX_FILENAME);
            interrupt(0x21,0,name,0,0); // cetak namanya ke layar
            interrupt(0x21,0,"\n",0,0);
        }
    }
    for(i=0;i<MAX_FILESYSTEM_ITEM;i++){
        if(files[i*DIR_ENTRY_LENGTH]==curDir && files[i*DIR_ENTRY_LENGTH+1]!='\0'){
            stringCopy(files,name,i*DIR_ENTRY_LENGTH+1,MAX_FILENAME);
            interrupt(0x21,0,name,0,0); // cetak namanya ke layar
            interrupt(0x21,0,"\n",0,0);
        }
    }
    interrupt(0x21,0x7,0,0,0); // terminateProgram
    return 0;
}