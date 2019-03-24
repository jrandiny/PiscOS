// #include "string.c"

#define MAX_ELEMENT 35
#define MAX_STRINGLENGTH 15
#define SECTOR_SIZE 512
#define MAX_DIRECTORY 32
#define MAX_FILES 32
#define MAX_FILESYSTEM_ITEM 32
#define MAX_DIRECTORYNAME 15
#define MAX_FILENAME 15
#define MAX_SECTORS 16
#define DIR_ENTRY_LENGTH 16
#define DIRS_SECTOR 257
#define FILES_SECTOR 258
#define NOT_FOUND -1
#define MAX_PATHNAME 512
#define ROOT 0xff

void printString(char *string);
void stringPlace(char *in,char *out);
void splitStringArray(char *s, char delim, int* length, char result[MAX_ELEMENT][MAX_STRINGLENGTH]);
void stringConcat(char *first, char *second, char *out);
void stringCopy(char* in, char* out, int start, int length);
int stringCompare(char* a, char* b, int length);
void pathParser(char *path, char *fileName, int *dirIndex, char parentIndex);
void finder(char* name,char* dir, int parent,int* idx);
int stringLen(char* in);
void splitString(char *s, char delim, int start, int* end);
void printStringe(char *string);
void findParent(char* dirName,char* directories, int* parentIdx);

void intToChar(int angka, char* hasil);
int mod(int a, int b);
int div(int a, int b);

int main(){
   int tempDir;
   int nextIdx;
   int length;
   int dirIdx;
   int i;
   int result;
   int curDir=ROOT;
   char pathNow[MAX_PATHNAME];
   char argc;
   char *argv[50];
   char dirName[MAX_FILENAME];
   char tempName[MAX_PATHNAME];
   char directories[MAX_SECTORS];
   char input[MAX_PATHNAME];
   char concatedInput[MAX_ELEMENT][MAX_STRINGLENGTH];
   ///////////
   char tempangka[20];
   int found;
   
   pathNow[0]='\0';
   while(1){
      interrupt(0x21,0x02,directories,DIRS_SECTOR,0); // readSector directori
      intToChar(curDir,tempangka);
      printString("curdir=");
      printStringe(tempangka);
                     
      printString(pathNow);
      printString("$ ");
      interrupt(0x21,0x01,input,0,0); // ambil input
      splitStringArray(input,' ',&length,concatedInput);//split input
      if(stringCompare(concatedInput[0],"cd",2)){
         if(length==2){
            if(stringCompare(concatedInput[1],"..",2)){ //balik
               if(curDir!=ROOT){
                  i=stringLen(pathNow)-1;
                  while(pathNow[i]!='/'){
                     i--;
                  }
                  stringCopy(pathNow,tempName,0,i);
                  stringCopy(pathNow,dirName,i+1,MAX_DIRECTORYNAME);
                  stringPlace(tempName,pathNow);
                  printString("   path jadi:");
                  printStringe(pathNow);
                  printString("   yang dicari jadi:");
                  printStringe(dirName);
                  findParent(dirName,directories,&curDir);
               }
            }else{ //masuk
               pathParser(concatedInput[1],dirName,&dirIdx,curDir);
               if(dirIdx!=NOT_FOUND){
                     finder(dirName,directories,dirIdx,&tempDir);
                     if(tempDir!=NOT_FOUND){
                        curDir=tempDir;
                        // intToChar(tempDir,tempangka);
                        // printString("tempDir=");
                        // printStringe(tempangka);
                        
                        // printString("pathnow sebelum concat=");
                        // printStringe(pathNow);
                        // printString("akan diconcat dengan=");
                        // printStringe(concatedInput[1]);

                        stringConcat(pathNow,"/",tempName);
                        stringConcat(tempName,concatedInput[1],pathNow);
                        // printString("pathnow sesudah concat=");
                        // printStringe(pathNow);
                     }else{
                        printString("No such file or directory\n\r");
                     }
               }else{
                  printString("No such file or directory\n\r");
               }
            }
         }else{
            printString("Usage: cd <dir_name>\n\r");
         }
      }else if(stringCompare(concatedInput[0], "./",2)){ // run program dari currDir
         argc=length-1;
         for(i=1;i<=argc;i++){
            argv[i-1]=concatedInput[i];
         }
         length=stringLen(concatedInput[0]);
         for(i=0;i<length-2;i++){
            concatedInput[0][i]=concatedInput[0][i+2];
         }
         concatedInput[0][length-2]='\0';
         interrupt(0x21,0x20,curDir,argc,argv);
         interrupt(0x21,curDir<<8|0x6,concatedInput[0],0x2000,&result);
         if(result!=0){
            printString("No program found\n\r");
         }
      }else{ // run program dari root
         argc=length-1;
         for(i=1;i<=argc;i++){
            argv[i-1]=concatedInput[i];
         }
         interrupt(0x21,0x20,curDir,argc,argv);
         interrupt(0x21,0xFF<<8|0x6,concatedInput[0],0x2000,&result);
         if(result!=0){
            printString("No program found\n\r");
         }
      }
      printString("\n\r");
   }
}

void printString(char *string){//tanpa enter
   int i =0;
   while(string[i]!='\0'){
      interrupt(0x10,0xE00+string[i],0,1,0);
      i++;
   }
}

void pathParser(char *path, char *fileName, int *dirIndex, char parentIndex){
   char parentTemp;
   char dirs[SECTOR_SIZE];
   char dirSearch[MAX_DIRECTORYNAME];
   int pathLength;
   int filePart;
   int lastEnd;
   int temp;
   int found;

   parentTemp = parentIndex;
   pathLength = stringLen(path);
   filePart = 0;
   lastEnd = 0;

   interrupt(0x21,0x02,dirs,DIRS_SECTOR,0); // readSector directori
   
   *dirIndex = parentIndex;

   filePart = 0;
   lastEnd = 0;
   while(!filePart){
      splitString(path,'/',lastEnd,&temp);
      if(temp+1==pathLength){
         filePart = 1;
      }else{
         stringCopy(path,dirSearch,lastEnd,temp-lastEnd+1);
         found = 0;
         *dirIndex = 0;
         while((*dirIndex)<MAX_DIRECTORY && !found){
            if(stringCompare(dirs+(*dirIndex)*DIR_ENTRY_LENGTH+1,dirSearch,MAX_DIRECTORYNAME)){
               if(dirs[(*dirIndex)*DIR_ENTRY_LENGTH]==parentTemp){
                  found = 1;
                  parentTemp = (*dirIndex);
               }
            }else{
               (*dirIndex)++;
            }
         }   

         if(found){
            lastEnd = temp+2;
         }else{
            *dirIndex = NOT_FOUND;
            return;
         }
      }
   }
   stringCopy(path,fileName,lastEnd,temp+1);
}

void finder(char* name,char* dir, int parent,int* idx){
   int found=0;
   char temp[MAX_DIRECTORYNAME];
   char tempangka[20];
   *idx=0;
   while((*idx)<MAX_FILESYSTEM_ITEM && !found){
      // stringCopy(dir+(*idx)*DIR_ENTRY_LENGTH+1,temp,0,MAX_DIRECTORYNAME);
      // printStringe(temp);
      if(stringCompare(name,dir+(*idx)*DIR_ENTRY_LENGTH+1,MAX_FILENAME)&&dir[(*idx)*DIR_ENTRY_LENGTH]==parent){
         found = 1;
      }else{
         (*idx)+=1;
      }
   }
   if(!found){
      *idx=NOT_FOUND;
   }
   // intToChar(*idx,tempangka);
   // printString("idx ketemu=");
   // printStringe(tempangka);
}

void splitStringArray(char *s, char delim, int* length, char result[MAX_ELEMENT][MAX_STRINGLENGTH]){
    int idxString=0;
    int lastIdx=0;
    int idxResult=0;
    int end = 0;
    while(!end){
        if(s[idxString]==delim){
            stringCopy(s,result[idxResult],lastIdx,idxString-lastIdx);
            lastIdx=idxString+1;
            idxResult++;
        }else if (s[idxString]=='\0'){
            stringCopy(s,result[idxResult],lastIdx,idxString-lastIdx);
            lastIdx=idxString+1;
            idxResult++;
            end=1;
        }
        idxString++;
    }
    *length=idxResult;
}

void stringPlace(char *in,char *out){
    int i;

    i=0;
    while(in[i]!='\0'){
        out[i]=in[i];
        i++;
    }
    out[i]='\0';
}


void stringConcat(char *first, char *second, char *out){
   int i;
   int j;
   
   for(i=0;i<MAX_STRINGLENGTH;i++){
      out[i]='\0';
   }

   i=0;
   j=0;
   while(first[i]!='\0'){
      out[j]=first[i];
      j++;
      i++;
   }
   i=0;
   while(second[i]!='\0'){
      out[j]=second[i];
      j++;
      i++;
   }
   j++; // tambah \0 di akhir
   out[j] ='\0';    
}

void stringCopy(char* in, char* out, int start, int length){
    int index = start;
    int outIndex = 0;

    while(outIndex<length && in[index]!='\0'){
        out[outIndex] = in[index];
        index++;
        outIndex++;
    }
    out[outIndex] = '\0';
}

int stringCompare(char* a, char* b, int length){
    int isSame = 1;
    int index = 0;

    length -= 1;

    while(a[index]!='\0' && b[index]!='\0' && isSame && index<length){
        if(a[index] != b[index]){
            isSame = 0;
        }else{
            index++;
        }
    }

    if(a[index] != b[index]){
        isSame = 0;
    }

    return isSame;
}

void splitString(char *s, char delim, int start, int* end){
    int idxString=start;
    int found =0;
    while(!found){
        if(s[idxString]==delim || s[idxString]=='\0'){
            idxString--;
            found=1;
        }else {
            idxString++;
        }
    }
    *end=idxString;
}

int stringLen(char* in){
    int index = 0;
    while(in[index]!='\0'){
        index++;
    }
    return index;
}

void printStringe(char *string){
   int i =0;
   while(string[i]!='\0'){
      interrupt(0x10,0xE00+string[i],0,1,0);
      i++;
   }
   interrupt(0x10, 0xE00 + '\n', 0, 1, 0) ;
   interrupt(0x10, 0xE00 + '\r', 0, 1, 0) ;
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

void findParent(char* dirName,char* directories, int* parentIdx){
   int found=0;
   int i=0;
   char tempangka[20];
   char temp[MAX_DIRECTORYNAME];

   while(!found){
      intToChar(i,tempangka);
      printString("i compare =");
      printStringe(tempangka);
      printString("comparing ");
      stringCopy(directories+(i)*DIR_ENTRY_LENGTH+1,temp,0,MAX_DIRECTORYNAME);
      printStringe(temp);
      
      if(stringCompare(directories+DIR_ENTRY_LENGTH*i+1,dirName,MAX_DIRECTORYNAME)){
         found=1;
      }else{
         i++;
      }
   }

   intToChar(i,tempangka);
   printString("i found =");
   printStringe(tempangka);

   if (found) {
      // printStringe("found");
      *parentIdx=directories[i*DIR_ENTRY_LENGTH];
      intToChar(*parentIdx,tempangka);
      printString("parentidx jadi =");
      printStringe(tempangka);
   }else{
      *parentIdx=NOT_FOUND;
   }

}