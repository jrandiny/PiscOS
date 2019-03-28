

int main(){
    char temp[128];
    char print[2];
    int curDir;
    int suc;

    interrupt(0x21,0x23,0,temp,0); // ambil argumen pertama
    interrupt(0x21,0x21,&curDir,0,0); // ambil directori sekarang
    interrupt(0x21,curDir<<8 | 0x8,temp,&suc,0); // panggil makeDirectory
    if(suc!=0) 
        interrupt(0x21,0x00,"mkdir: cannot create directory '': No such file or directory\n",0,0);
    interrupt(0x21,0x7,0,0,0); // terminateProgram
    return 0;
}