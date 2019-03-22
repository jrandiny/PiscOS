#define SECTOR_SIZE 512
#define DIRS_SECTOR 257
#define FILES_SECTOR 258
#define MAX_FILENAME 15
#define MAX_FILESYSTEM_ITEM 32
#define DIR_ENTRY_LENGTH 16

void stringCopy(char* in, char* out, int start, int length);

int main(){
    char name[MAX_FILENAME];
    char directoris[SECTOR_SIZE];
    char files[SECTOR_SIZE];
    int curDir;
    int i;

    interrupt(0x21,0x21,&curDir,0,0); // ambil directori sekarang
    interrupt(0x21,0x2,directoris,DIRS_SECTOR,0); // readSector directori
    interrupt(0x21,0x2,files,FILES_SECTOR,0); // readSector file
    for(i=0;i<MAX_FILESYSTEM_ITEM;i++){
        if(directoris[i*DIR_ENTRY_LENGTH]==curDir && directoris[i*DIR_ENTRY_LENGTH+1]!='\0'){
            stringCopy(directoris,name,i*DIR_ENTRY_LENGTH+1,MAX_FILENAME);
            interrupt(0x21,0,name,0,0); // cetak namanya ke layar
        }
    }
    for(i=0;i<MAX_FILESYSTEM_ITEM;i++){
        if(files[i*DIR_ENTRY_LENGTH]==curDir && files[i*DIR_ENTRY_LENGTH+1]!='\0'){
            stringCopy(files,name,i*DIR_ENTRY_LENGTH+1,MAX_FILENAME);
            interrupt(0x21,0,name,0,0); // cetak namanya ke layar
        }
    }
    interrupt(0x21,0x7,0,0,0); // terminateProgram
    return 0;
}

void stringCopy(char* in, char* out, int start, int length){
    int index = start;
    int outIndex = 0;

    while(outIndex<length && in[index]!='\0'){
        out[outIndex] = in[index];
        index++;
        outIndex++;
    }
    out[outIndex] = '\0';
}