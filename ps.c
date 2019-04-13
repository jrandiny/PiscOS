#include "proc.h"
#include "definition.h"


void intToChar(int angka, char* hasil);
int mod(int a, int b);
int div(int a, int b);

int main(){
    int i;
    struct PCB pcb;
    char convertedResult[31];
    char files[SIZE_SECTOR];

    enableInterrupts();
    interrupt(0x21,0x2,files,LOC_FILE_SECTOR,0);
    for(i=0;i<MAX_SEGMENTS;i++){
        interrupt(0x21,0x16,i,&pcb,0);
        if(pcb.index != EMPTY_INDEX){
            intToChar(i,convertedResult);
            interrupt(0x21,0x00,"PID=",0,0);
            interrupt(0x21,0x00,convertedResult,0,0);
            intToChar(pcb.state,convertedResult);
            interrupt(0x21,0x00," Status=",0,0);
            interrupt(0x21,0x00,convertedResult,0,0);
            interrupt(0x21,0x00," Name=",0,0);
            interrupt(0x21,0x00,files+pcb.index*SIZE_DIR_ENTRY+1,0,0);
            intToChar(pcb.index,convertedResult);
            interrupt(0x21,0x00," \n",0,0);
        }
    }
    interrupt(0x21,0x7,0,0,0); // terminateProgram

}

void intToChar(int angka, char* hasil){
  char tempHasil[31];
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
    hasil[panjang]='\0';
    while(i>0){
      hasil[panjang-i]=tempHasil[i-1];
      i--;
    }
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