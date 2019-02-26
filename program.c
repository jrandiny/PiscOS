#define MAX_MEM 20

int charToInt(char* angka);
int mod(int a, int b);
int div(int a, int b);
void intToChar(int angka, char* hasil);

int main(){
  char angka1[MAX_MEM];
  char angka2[MAX_MEM];
  char operator[MAX_MEM];
  char hasil[MAX_MEM];

  interrupt(0x21,0x0,"Masukkan angka 1:",0,0);
  interrupt(0x21,0x1,angka1,0,0);
  interrupt(0x21,0x0,"Masukkan angka 2:",0,0);
  interrupt(0x21,0x1,angka2,0,0);
  interrupt(0x21,0x0,"Masukkan operator:",0,0);
  interrupt(0x21,0x1,operator,0,0);

  switch(operator[0]){
    case '+':
      intToChar(charToInt(angka1)+charToInt(angka2),hasil);
      interrupt(0x21,0x0,hasil,0,0);
      break;
    case '-':
      intToChar(charToInt(angka1)-charToInt(angka2),hasil);
      interrupt(0x21,0x0,hasil,0,0);
      break;
    case '/':
      if(charToInt(angka2)==0){
        interrupt(0x21,0x0,"Error, pembagian 0",0,0);
      }else{
        intToChar(div(charToInt(angka1),charToInt(angka2)),hasil);
        interrupt(0x21,0x0,hasil,0,0);
      }
      break;
    case '*':
      intToChar(charToInt(angka1)*charToInt(angka2),hasil);
      interrupt(0x21,0x0,hasil,0,0);
      break;
    case '%':
      intToChar(mod(charToInt(angka1),charToInt(angka2)),hasil);
      interrupt(0x21,0x0,hasil,0,0);
      break;
  }

  return 0;
}

int charToInt(char* angka){
  int hasil=0;
  char temp[MAX_MEM];
  int i=0;
  int neg = 1;

  if(angka[i]=='-'){
    i++;
    neg = -1;
  }

  while(angka[i]!='\0'){
    hasil=hasil*10+(angka[i]-'0');
    i++;
  }

  hasil *= neg;

  return hasil;
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

void intToChar(int angka, char* hasil){
  char tempHasil[MAX_MEM];
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
  }  
}

