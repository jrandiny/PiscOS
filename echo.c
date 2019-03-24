

int main(){
    char temp[128][128];
    char hasil[128];
    char print[2];
    int n;
    int i;
    int j;
    int k;

    interrupt(0x21,0x22,&n,0,0); // ambil argc
    for(i=0;i<n;++i){ // copy semua isi argv ke temp
        interrupt(0x21,0x23,i,temp[i],0);
        
    }
    i=0;
    j=0;
    k=0;
    while(i<n){ // saline dari temp ke hasil untuk menjadi string panjang
        while(temp[i][j]!='\0'){
            hasil[k] = temp[i][j];
            j++;
            k++;
        }
        hasil[k] = ' ';
        j=0;
        i++;
        k++;
    }
    hasil[k]='\0';
    interrupt(0x21,0,hasil,0,0); // cetak hasilnya ke layar
    interrupt(0x21,0x7,0,0,0); // terminateProgram
    return 0;
}