

int main(){
    char temp[128];
    char print[2];
    int curDir;

    interrupt(0x21,0x23,0,temp,0); // ambil argumen pertama
    interrupt(0x21,0x21,&curDir,0,0); // ambil directori sekarang
    interrupt(0x21,curDir<<8 | 0x8,temp,0,0); // panggil makeDirectory
    interrupt(0x21,0x7,0,0,0); // terminateProgram
    return 0;
}