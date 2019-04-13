#include "definition.h"

int charToInt(char* angka);

int main(){
    char temp[128][128];
    char hasil[128];
    char print[2];
    char n;
    int i;
    int j;
    int k;
    int tick;
    
    enableInterrupts();
    interrupt(0x21,0x22,&n,0,0); // ambil argc
    if(n>1){
        for(i=0;i<n;++i){ // copy semua isi argv ke temp
            interrupt(0x21,0x23,i,temp[i],0); 
        }
        tick = charToInt(temp[0]);
        if(tick>=0){
            i=1; 
            j=0;
            k=0;
            while(i<n){ // salin dari temp ke hasil untuk menjadi string panjang
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

            interrupt(0x21,0x40,tick,0,0);
            
            interrupt(0x21,0,hasil,0,0); // cetak hasilnya ke layar
            interrupt(0x21,0,"\n",0,0); // cetak hasilnya ke layar
        } else {
            interrupt(0x21,0x00,"Wrong time inputed\n\r");
        }
    } else {
        interrupt(0x21,0x00,"Usage timer <tick> <argument> ... \n\r",0,0);
    }

    interrupt(0x21,0x7,0,0,0); // terminateProgram
    return 0;
}

int charToInt(char* angka){
  int hasil=0;
  char temp[31];
  int i=0;
  int neg = 1;

  if(angka[i]=='-'){
    i++;
    neg = -1;
  }

  while(angka[i]!='\0'){
    if(((angka[i]-'0')>=0) && ((angka[i]-'0')<=9)){
      hasil=hasil*10+(angka[i]-'0');
      i++;
    }else{
      hasil = -1;
      hasil *= neg;
      break;
    }
  }

  hasil *= neg;

  return hasil;
}