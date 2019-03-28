#define INSUFFICIENT_SECTORS -4
#define INSUFFICIENT_ENTRIES -3
#define ALREADY_EXISTS -2
#define NOT_FOUND -1
#define MAX_PATHNAME 512
#define SECTOR_SIZE 512

void intToChar(int angka, char* hasil);
int mod(int a, int b);
int div(int a, int b);

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
    // intToChar(argc,temp);
    // interrupt(0x21,0x00,temp,0,0);
    for(i=0;i<argc,i<3;++i){ // copy semua isi argv ke temp
        interrupt(0x21,0x23,i,argv[i],0);
    }
    if(argc==3){
        interrupt(0x21,currDir<<8|0x12,argv[1],&result,0);
        if(result==NOT_FOUND){
            interrupt(0x21,0x00,"Hehe ga ketemu! lol",0,0);
        }else if(result==0){ //kalau file
            interrupt(0x21,currDir<<8|0x13,argv[1],&length,0);

            nSector = length;
            interrupt(0x21,currDir<<8|0x04,tempBuffer,argv[1],&readResult);

            interrupt(0x21,currDir<<8|0x05,tempBuffer,argv[2],&nSector,0);
        }else if(result==1){ //kalau direktori

        }
    }else{
        interrupt(0x21,0x00,"hey kebanyakan boi",0,0);
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