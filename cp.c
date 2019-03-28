#include "definition.h"
// #define INSUFFICIENT_SECTORS -4
// #define INSUFFICIENT_ENTRIES -3
// #define ALREADY_EXISTS -2
// #define NOT_FOUND -1
// #define MAX_PATHNAME 512
// #define SECTOR_SIZE 512

void intToChar(int angka, char* hasil);
int mod(int a, int b);
int div(int a, int b);
void copy(char* from, char* to,char currDir,int* success);

int main(){
    char argv[128][128];
    char tempBuffer[16*512];
    char tempFile[512];
    char tempData[512];
    char temp[10];
    char tempAngka[10];
    int argc;
    int i;
    int result;
    int readResult;
    int nSector;
    char currDir;
///////////////////
    char from[MAX_PATHNAME];
    char to[MAX_PATHNAME];
    char sectors[SECTOR_SIZE];

    int length;

    interrupt(0x21,0x21,&currDir,0,0);

    interrupt(0x21,0x22,&argc,0,0); // ambil argc
    intToChar(argc,temp);
    interrupt(0x21,0x00,"nilai argc di cp= ",0,0);
    interrupt(0x21,0x00,temp,0,0);
    interrupt(0x21,0x00,"\n",0,0);
    for(i=0;i<argc;i++){ // copy semua isi argv ke temp
        interrupt(0x21,0x23,i,argv[i],0);
        interrupt(0x21,0x00,"letak",0,0);
    }
    if(argc==2){
        interrupt(0x21,0x00,"argumen kamu 2 loh\n",0,0);
        copy(argv[0],argv[1],currDir,&result);
    }else{
        interrupt(0x21,0x00,"hey kebanyakan boi\n",0,0);
    }
    interrupt(0x21,0x7,0,0,0); // terminateProgram
    return 0;
}

void intToChar(int angka, char* hasil){
    char tempHasil[200];
    int temp;
    int i=0;
    int panjang;

    if(angka == 0){
        hasil[0] = '0';
        hasil[1] = '\0';
    }else{
        if(angka<0){
        temp=-1*angka;
        }else{
        temp=angka;
        }
        while(temp>0){
        tempHasil[i]=mod(temp,10)+'0';
        temp = div(temp,10);
        i++;
        }
        if(angka<0){
        tempHasil[i]='-';
        i++;
        }

        panjang = i;

        while(i>0){
        hasil[panjang-i]=tempHasil[i-1];
        i--;
        }
        hasil[panjang] = '\0';
    }  
}

int mod(int a, int b) {
    while(a >= b) {
        a = a - b;
    }
    return a;
}

int div(int a, int b) {
    int q = 0;
    while(q*b <= a) {
        q = q+1;
    }
    return q-1;
}

void copy(char* from, char* to,char currDir,int* success){
    int i;
    int result;
    int nSector;
    int dirIdx;
    int foundIdx;
    char tempBuffer[16*512];
    char directories[SECTOR_SIZE];
    char files[SECTOR_SIZE];
    char dirName[MAX_DIRECTORYNAME];
    char filename[MAX_FILENAME];
    char tempPathFrom[MAX_PATHNAME];
    char tempPathTo[MAX_PATHNAME];

    interrupt(0x21,0x00,"masuk cp loh!\n",0,0);
    interrupt(0x21,0x2,directories,DIRS_SECTOR,0); // readSector directori
    interrupt(0x21,0x2,files,FILES_SECTOR,0); // readSector file
    
    interrupt(0x21,0x00,"masuk cp loh!\n",0,0);
    interrupt(0x21,0x00,from,0,0);
    interrupt(0x21,0x00,"\n",0,0);

    interrupt(0x21,currDir<<8|0x12,from,&result,0);//panggil isDirectory
    if(result==NOT_FOUND){//kalau bukan file ataupun direktori
        interrupt(0x21,0x00,"Hehe ga ketemu! lol\n",0,0);
        *success=NOT_FOUND;
    }else if(result==0){ //kalau file
        interrupt(0x21,0x00,"a\n",0,0);
        interrupt(0x21,currDir<<8|0x13,from,&nSector,0);//getFileSize
        interrupt(0x21,0x00,"b\n",0,0);
        interrupt(0x21,currDir<<8|0x04,tempBuffer,from,&result);//readFile
        interrupt(0x21,0x00,"c\n",0,0);
        //mungkin hang karena overuse result
        if(result==0){
            interrupt(0x21,currDir<<8|0x05,tempBuffer,to,&nSector);//writeFile
            *success=nSector;//set success
        }else{
            interrupt(0x21,0x00,"gagal read\n",0,0);
        }
    }else if(result==1){ //kalau direktori
        interrupt(0x21,currDir<<8|0x08,to,&result,0);//makeDirectory
        //mungkin hang karena overuse result
        interrupt(0x21,0x00,"hey",0,0);
        if(result==0 || result==ALREADY_EXISTS){//berhasil makeDirectory
            interrupt(0x21,currDir<<8|0x10,from,dirName,&dirIdx);//pathparser
            interrupt(0x21,dirIdx<<8|0x11,dirName,directories,&foundIdx);//finder
            if(foundIdx!=NOT_FOUND){
                for(i=0;i<MAX_DIRECTORY;i++){
                    if(directories[i*DIR_ENTRY_LENGTH]==foundIdx && directories[i*DIR_ENTRY_LENGTH+1]!='\0'){
                        stringCopy(directories,dirName,i*DIR_ENTRY_LENGTH+1,MAX_DIRECTORYNAME);
                        //mungkin error di bawah ini
                        stringCopy(from,tempPathFrom,0,MAX_PATHNAME);
                        stringConcat(tempPathFrom,"/",tempPathFrom);
                        stringConcat(tempPathFrom,dirName,tempPathFrom);
                        //di sini ada print
                        interrupt(0x21,0x00,tempPathFrom,0,0);
                        interrupt(0x21,0x00,"\n",0,0);

                        stringCopy(to,tempPathTo,0,MAX_PATHNAME);
                        stringConcat(tempPathTo,"/",tempPathTo);
                        stringConcat(tempPathTo,dirName,tempPathTo);
                        //di sini ada print
                        interrupt(0x21,0x00,tempPathTo,0,0);
                        interrupt(0x21,0x00,"\n",0,0);
                        copy(tempPathFrom,tempPathTo,currDir,&result);
                        if(result!=0){
                            *success=result;
                            return;
                        }
                    }
                }
                interrupt(0x21,0x00,"cari files\n",0,0);
                for(i=0;i<MAX_FILES;i++){
                    if(files[i*DIR_ENTRY_LENGTH]==foundIdx && files[i*DIR_ENTRY_LENGTH+1]!='\0'){
                        stringCopy(files,filename,i*DIR_ENTRY_LENGTH+1,MAX_FILENAME);
                        //mungkin error di bawah ini
                        stringCopy(from,tempPathFrom,0,MAX_PATHNAME);
                        stringConcat(tempPathFrom,"/",tempPathFrom);
                        stringConcat(tempPathFrom,filename,tempPathFrom);
                        //di sini ada print
                        interrupt(0x21,0x00,tempPathFrom,0,0);
                        interrupt(0x21,0x00,"\n",0,0);

                        stringCopy(to,tempPathTo,0,MAX_PATHNAME);
                        stringConcat(tempPathTo,"/",tempPathTo);
                        stringConcat(tempPathTo,filename,tempPathTo);
                        //di sini ada print
                        interrupt(0x21,0x00,tempPathTo,0,0);
                        interrupt(0x21,0x00,"\n",0,0);
                        copy(tempPathFrom,tempPathTo,currDir,&result);
                        if(result!=0){
                            *success=result;
                            return;
                        }
                    }
                }
                *success=0;
            }else{
                interrupt(0x21,0x00,"gagal foundidx\n",0,0);
                *success=NOT_FOUND;
            }
        }else{
            *success=result;
        } 
    }


}