#define SECTOR_SIZE 512
#define DIRS_SECTOR 257
#define FILES_SECTOR 258
#define MAX_FILENAME 15
#define MAX_FILESYSTEM_ITEM 32
#define DIR_ENTRY_LENGTH 16
#define NOT_FOUND -1

int main(){
    char temp[128];
    char name[MAX_FILENAME];
    int res;
    char curDir;
    int succ;
    int i;

    succ = 0;
    interrupt(0x21,0x21,&curDir,0,0); // ambil directori sekarang
    interrupt(0x21,0x23,1,temp,0); // ambil argumen pertama

    interrupt(0x21,curDir<<8|0x09,temp,&res,0);

    if(res != NOT_FOUND){
        succ = 1;
    }

    interrupt(0x21,curDir<<8|0x0A,temp,&res,0);

    if(res != NOT_FOUND){
        succ = 1;
    }

    if(succ!=1){
        interrupt(0x21,0x00,"Not found",0,0);
    }

    interrupt(0x21,0x7,0,0,0); // terminateProgram
    return 0;
}