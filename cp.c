#include "definition.h"

void copy(char* from, char* to,char currDir,int* success);

char errMsg[SIZE_SECTOR];

int main(){
    char argv[128][128];
    char temp[10];
    char argc;
    int i;
    int result;
    char currDir;

    interrupt(0x21,0xFF<<8|0x04,errMsg,"e.msg",0);

    interrupt(0x21,0x21,&currDir,0,0);
    interrupt(0x21,0x22,&argc,0,0); // ambil argc
    for(i=0;i<argc;i++){ // copy semua isi argv ke temp
        interrupt(0x21,0x23,i,argv[i],0);
    }
    if(argc==2){
        copy(argv[0],argv[1],currDir,&result);
    }else{
        interrupt(0x21,0x00,"Usage cp <source> <dest>\n",0,0);
    }
    interrupt(0x21,0x7,0,0,0); // terminateProgram
    return 0;
}

void copy(char* from, char* to,char currDir,int* success){
    int i;
    int result;
    int nSector;
    int dirIdx;
    int foundIdx;
    char tempBuffer[16*512];
    char directories[SIZE_SECTOR];
    char files[SIZE_SECTOR];
    char dirName[MAX_DIRECTORYNAME];
    char filename[MAX_FILENAME];
    char tempPathFrom[MAX_PATHNAME];
    char tempPathTo[MAX_PATHNAME];

    interrupt(0x21,0x2,directories,LOC_DIR_SECTOR,0); // readSector directori
    interrupt(0x21,0x2,files,LOC_FILE_SECTOR,0); // readSector file

    interrupt(0x21,currDir<<8|0x12,from,&result,0);//panggil isDirectory
    if(result==ERROR_NOT_FOUND){//kalau bukan file ataupun direktori
        interrupt(0x21,0x00,errMsg+EMSG_NOT_FOUND*SIZE_EMSG_ENTRY,0,0);
        *success=ERROR_NOT_FOUND;
    }else if(result==0){ //kalau file
        interrupt(0x21,currDir<<8|0x13,from,&nSector,0);//getFileSize
        interrupt(0x21,currDir<<8|0x04,tempBuffer,from,&result);//readFile
        //mungkin hang karena overuse result
        if(result==0){
            interrupt(0x21,currDir<<8|0x05,tempBuffer,to,&nSector);//writeFile
            if(nSector>0){
                *success = 0;
            } else {
                *success=nSector;//set success
            }
        }else{
            interrupt(0x21,0x00,errMsg+EMSG_IO_ERROR*SIZE_EMSG_ENTRY,0,0);
        }
    }else if(result==1){ //kalau direktori
        interrupt(0x21,currDir<<8|0x08,to,&result,0);//makeDirectory
        //mungkin hang karena overuse result
        if(result==0 || result==ERROR_ALREADY_EXISTS){//berhasil makeDirectory
            interrupt(0x21,currDir<<8|0x10,from,dirName,&dirIdx);//pathparser
            interrupt(0x21,dirIdx<<8|0x11,dirName,directories,&foundIdx);//finder
            if(foundIdx!=ERROR_NOT_FOUND){
                for(i=0;i<MAX_DIRECTORIES;i++){
                    if(directories[i*SIZE_DIR_ENTRY]==foundIdx && directories[i*SIZE_DIR_ENTRY+1]!='\0'){
                        stringCopy(directories,dirName,i*SIZE_DIR_ENTRY+1,MAX_DIRECTORYNAME);
                        //mungkin error di bawah ini
                        stringCopy(from,tempPathFrom,0,MAX_PATHNAME);
                        stringConcat(tempPathFrom,"/",tempPathFrom);
                        stringConcat(tempPathFrom,dirName,tempPathFrom);

                        stringCopy(to,tempPathTo,0,MAX_PATHNAME);
                        stringConcat(tempPathTo,"/",tempPathTo);
                        stringConcat(tempPathTo,dirName,tempPathTo);

                        copy(tempPathFrom,tempPathTo,currDir,&result);
                        if(result!=0){
                            *success=result;
                            return;
                        }
                    }
                }
                for(i=0;i<MAX_FILES;i++){
                    if(files[i*SIZE_DIR_ENTRY]==foundIdx && files[i*SIZE_DIR_ENTRY+1]!='\0'){
                        stringCopy(files,filename,i*SIZE_DIR_ENTRY+1,MAX_FILENAME);
                        //mungkin error di bawah ini
                        stringCopy(from,tempPathFrom,0,MAX_PATHNAME);
                        stringConcat(tempPathFrom,"/",tempPathFrom);
                        stringConcat(tempPathFrom,filename,tempPathFrom);

                        stringCopy(to,tempPathTo,0,MAX_PATHNAME);
                        stringConcat(tempPathTo,"/",tempPathTo);
                        stringConcat(tempPathTo,filename,tempPathTo);

                        copy(tempPathFrom,tempPathTo,currDir,&result);
                        if(result!=0){
                            *success=result;
                            return;
                        }
                    }
                }
                *success=0;
            }else{
                interrupt(0x21,0x00,errMsg+EMSG_NOT_FOUND*SIZE_EMSG_ENTRY,0,0);
                *success=ERROR_NOT_FOUND;
            }
        }else{
            *success=result;
        } 
    }
}