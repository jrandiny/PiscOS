#define INSUFFICIENT_SECTORS -4
#define INSUFFICIENT_ENTRIES -3
#define ALREADY_EXISTS -2
#define NOT_FOUND -1

int main(){
    char argv[128][128];
    char tempFile[512];
    char tempData[512];
    char temp[10];
    int argc;
    int i;
    int result;
    char currDir;

    interrupt(0x21,0x21,&currDir,0,0);

    interrupt(0x21,0x22,&argc,0,0); // ambil argc
    // intToChar(argc,temp);
    // interrupt(0x21,0x00,temp,0,0);
    for(i=0;i<argc;++i){ // copy semua isi argv ke temp
        interrupt(0x21,0x23,i,argv[i],0);
    }

    // cek mode write
    // interrupt(0x21,0x00,argv[0],0,0);
    // interrupt(0x21,0x00,argv[1],0,0);
    // interrupt(0x21,0x00,argv[2],0,0);
    // interrupt(0x21,0x00,argv[3],0,0);
    if(argc>2 && stringCompare(argv[2],"-w",2)){
        result = 1;
        interrupt(0x21,currDir<<8|0x05,"abcefghijklmn",argv[1],&result);
        if(result==INSUFFICIENT_SECTORS){
            interrupt(0x21,0x00,"Insufficient sectors",0,0);
        }else if(result == NOT_FOUND){
            interrupt(0x21,0x00,"Not found",0,0);
        }else if(result == ALREADY_EXISTS){
            interrupt(0x21,0x00,"Already exists",0,0);
        }else if(result == INSUFFICIENT_ENTRIES){
            interrupt(0x21,0x00,"Insufficient entries",0,0);
        }else{
            interrupt(0x21,0x00,"Masukkan teks : ",0,0);
            interrupt(0x21,0x01,tempData,0,0);
            interrupt(0x21,currDir<<8|0x09,argv[1],&result,0);
            result = 1;
            interrupt(0x21,currDir<<8|0x05,tempData,argv[1],&result);
        }
    }else{
        interrupt(0x21,currDir<<8 | 0x04,tempFile,argv[1],&result);
        if(result==0){
            interrupt(0x21,0x00,tempFile,0,0);
        }else if(result==-1){
            interrupt(0x21,0x00,"Not found",0,0);
        }
    }

    interrupt(0x21,0x7,0,0,0); // terminateProgram
    return 0;
}