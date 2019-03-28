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
void splitStringArray(char *s, char delim, int* length, char result[MAX_ELEMENT][MAX_STRINGLENGTH]);
void pathParser(char *path, char *fileName, int* dirIndex, char parentIndex);
void finder(char* name,char* dir, char parent,int* idx);
void printStringe(char *string);
void findParent(char* dirName,char* directories, char* parentIdx);
void executeProgram(char concatedInput[MAX_ELEMENT][MAX_STRINGLENGTH], int argc, char* pathNow, int curDir);

void intToChar(int angka, char* hasil);
int mod(int a, int b);
int div(int a, int b);

int main(){
   int tempDir;
   int nextIdx;
   int length;
   char dirIdx;
   int i;
   int j;
   int result;
   char curDir;
   char pathNow[MAX_PATHNAME];
   char argc;
   char *argv[50];
   char dirName[MAX_FILENAME];
   char tempName[MAX_PATHNAME];
   char directories[SECTOR_SIZE];
   char input[MAX_PATHNAME];
   char concatedInput[MAX_ELEMENT][MAX_STRINGLENGTH];
   char tempPath[MAX_ELEMENT][MAX_STRINGLENGTH];
   char tempCurrPath[MAX_ELEMENT][MAX_STRINGLENGTH];
   int pathPartCount;
   int currPathPartCount;
   int tempFindResult;
   char tempCurrDir;
   ///////////
   char tempangka[20];
   int found;
   
   // pathNow[0]='\0';
   interrupt(0x21,0x23,0,pathNow,0); // getArgv
   interrupt(0x21,0x21,&curDir,0,0); // ambil directori sekarang
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
            if(concatedInput[1][0]=='/'){
               printStringe("dbg_root");
               tempCurrDir = ROOT;
               splitStringArray(concatedInput[1]+1,'/',&pathPartCount,&tempPath);
            }else{
               printStringe("dbg_no_root");
               tempCurrDir = curDir;
               
               splitStringArray(concatedInput[1],'/',&pathPartCount,&tempPath);
            }

            splitStringArray(pathNow+1,'/',&currPathPartCount,&tempCurrPath);

            if(currPathPartCount==1){
               printStringe("dbg_count_1");
            }

            if(tempCurrPath[0][0]=='\0'){
               
               printStringe("dbg_curr_root");
            }

            printStringe("s_dbg");
            for(i=0;i<currPathPartCount;i++){
               printStringe("mas");
               printStringe(tempCurrPath[i]);
            }

            printStringe("masuk");

            printStringe("e_dbg");
            
            for(i=0;i<pathPartCount;i++){
               printStringe("s_dbg");
               printStringe(tempPath[i]);
               // printStringe("dbg_curr_dir");
               intToChar(tempCurrDir,dirName);
               // printStringe("out");
               printStringe(dirName);
               // printStringe("dbg_curr_pathnow");
               for(j=0;j<currPathPartCount;j++){
                  printStringe(tempCurrPath[j]);
               }

               printStringe("e_dbg");
               if(stringCompare(tempPath[i],"..",2)){
                  printStringe("dbg_up");
                  // Up directory
                  if(tempCurrPath[currPathPartCount-1][0]!='\0'){
                     currPathPartCount--;
                     printStringe(tempCurrPath[currPathPartCount]);
                     finder(tempCurrPath[currPathPartCount],directories,directories[tempCurrDir*DIR_ENTRY_LENGTH],&tempFindResult);
                     
                     //assert found
                     if(tempFindResult==NOT_FOUND){
                        printStringe("dbg_err_not_found");
                        break;
                     }

                     tempCurrDir = directories[tempFindResult*DIR_ENTRY_LENGTH];

                  }else{
                     // do nothing
                     // assert currdir == root
                     if(curDir!=ROOT){
                        printStringe("dbg_fatal_not_root");
                     }
                  }
               }else{
                  printStringe("dbg_goto");
                  // Go to
                  finder(tempPath[i],directories,tempCurrDir,&tempFindResult);
                  //Assert found
                  if(tempFindResult==NOT_FOUND){
                     printStringe("dbg_err_not_found");
                     break;
                  }
                  currPathPartCount++;
                  stringCopy(tempPath[i],tempCurrPath[currPathPartCount-1],0,MAX_STRINGLENGTH);

                  tempCurrDir = (char)tempFindResult;
               }
            }

         }else{
            printString("Usage: cd <dir_name>\n\r");
         }
      }else if(stringCompare(concatedInput[0], "./",2)){ // run program dari currDir
         length=stringLen(concatedInput[0]);
         for(i=0;i<length-2;i++){
            concatedInput[0][i]=concatedInput[0][i+2];
         }
         concatedInput[0][length-2]='\0';

         executeProgram(concatedInput, length, pathNow, curDir<<8 | curDir);
      }else{ // run program dari root
         executeProgram(concatedInput, length, pathNow, curDir<<8|0xFF);
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

void pathParser(char *path, char *fileName, int* dirIndex, char parentIndex){
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
            if(stringCompare(dirs+(*dirIndex)*DIR_ENTRY_LENGTH+1,dirSearch,MAX_DIRECTORYNAME) && (dirs[(*dirIndex)*DIR_ENTRY_LENGTH]==parentTemp)){
               found = 1;
               parentTemp = (*dirIndex);
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

void finder(char* name,char* dir, char parent,int* idx){
   int found=0;
   char temp[MAX_DIRECTORYNAME];
   char tempangka[20];
   *idx=0;
   while((*idx)<MAX_FILESYSTEM_ITEM && !found){
      if(stringCompare(name,dir+(*idx)*DIR_ENTRY_LENGTH+1,MAX_FILENAME)&&dir[(*idx)*DIR_ENTRY_LENGTH]==parent){
         found = 1;
      }else{
         (*idx)+=1;
      }
   }
   if(!found){
      *idx=NOT_FOUND;
   }
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

void findParent(char* dirName,char* directories, char* parentIdx){
   int found=0;
   int i=0;
   char tempangka[20];
   char temp[MAX_DIRECTORYNAME];
   char tempDir;

   while(!found){
      stringCopy(directories+(i)*DIR_ENTRY_LENGTH+1,temp,0,MAX_DIRECTORYNAME);
      
      if(stringCompare(directories+DIR_ENTRY_LENGTH*i+1,dirName,MAX_DIRECTORYNAME)){
         found=1;
      }else{
         i++;
      }
   }   

   if (found) {
      // printStringe("found");
      (*parentIdx)=directories[i*DIR_ENTRY_LENGTH];
   }else{
      *parentIdx=NOT_FOUND;
   }

}

void executeProgram(char concatedInput[MAX_ELEMENT][MAX_STRINGLENGTH], int argc, char* pathNow, int curDir){
   char* argv[50];
   int result;
   int i;

   argv[0]=pathNow;
   for(i=1;i<argc;i++){
      argv[i]=concatedInput[i];
   }
   interrupt(0x21,0x20,((curDir>>8) & 0xFF),argc,argv); // putArgs
   interrupt(0x21,(curDir & 0xFF)<<8|0x6,concatedInput[0],0x2000,&result); // executeProgram
   if(result!=0){
      printString("No program found\n\r");
   }
}