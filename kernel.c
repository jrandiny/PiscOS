#include "definition.h"

void handleInterrupt21 (int AX, int BX, int CX, int DX);
// String syscall
void printString(char *string);
void readString(char *string);
// Program syscall
void executeProgram(char *path, int segment, int *result, char parentIndex);
void terminateProgram (int *result);
// Argument syscall
void putArgs (char curdir, char argc, char **argv);
void getArgc (char *argc);
void getArgv (char index, char *argv);
// Sector IO syscall
void readSector(char *buffer, int sector);
void writeSector(char *buffer, int sector);
// File IO syscall
void getCurdir (char *curdir);
void readFile(char *buffer, char *path, int *result, char parentIndex);
void writeFile(char *buffer, char *path, int *sectors, char parentIndex);
void deleteDirectory(char *path, int *success, char parentIndex);
void deleteFile(char *path, int *result, char parentIndex);
void makeDirectory(char *path, int *result, char parentIndex);
void isDirectory(char * path,int * result, char parentIdx);
void getFileSize(char *path, int *result, char parentIndex);

// Helper function
void clear(char *buffer, int length);
void printTxt(char * filename,int x, int y, int color);
void clearScreen(int height);
void pathParser(char *path, char *fileName, int *dirIndex, char parentIndex);
void finder(char* name,char* dir, char parent,int* idx);
int mod(int a, int b);
int div(int a, int b);

int main() {
   char tempFile[SECTOR_SIZE*MAX_SECTORS];
   char *argv[2];
   int suc = 0;
   int i;

   makeInterrupt21();
   clearScreen(30);
   printTxt("title.txt",20,0,0xF);
   printTxt("logo.txt",25,5,0x6);
   printString("Press any key to continue...");
   interrupt(0x16, 0, 0, 0, 0);
   interrupt(0x10,0x3,0,0,0);
   interrupt(0x10,0xE00+'\n',0,0,0);

   // printString("belum while");
   while (1){
      
      makeDirectory("abc",&suc,ROOT);
      // makeDirectory("bbb",&suc,ROOT);

      // makeDirectory("ccc",&suc,ROOT);
      makeDirectory("abc/jjj1",&suc,ROOT);
      // makeDirectory("bbb/jjj2",&suc,ROOT);
      // makeDirectory("ccc/jjj3",&suc,ROOT);
      

      // makeDirectory("ccc/jjj3/222",&suc,ROOT);

      // makeDirectory("ccc/jjj3/222/333",&suc,ROOT);
      // argv[0]="/";
      putArgs(ROOT,0,"");
      executeProgram("shell",0x2000,&suc,ROOT);
   }
}

void handleInterrupt21 (int AX, int BX, int CX, int DX) {
   char AL, AH;
   AL = (char) (AX);
   AH = (char) (AX >> 8);
   switch (AL) {
      case 0x00:
         printString(BX);
         break;
      case 0x01:
         readString(BX);
         break;
      case 0x02:
         readSector(BX, CX);
         break;
      case 0x03:
         writeSector(BX, CX);
         break;
      case 0x04:
         readFile(BX, CX, DX, AH);
         break;
      case 0x05:
         writeFile(BX, CX, DX, AH);
         break;
      case 0x06:
         executeProgram(BX, CX, DX, AH);
         break;
      case 0x07:
         terminateProgram(BX);
         break;
      case 0x08:
         makeDirectory(BX, CX, AH);
         break;
      case 0x09:
         deleteFile(BX, CX, AH);
         break;
      case 0x0A:
         deleteDirectory(BX, CX, AH);
         break;
      case 0x20:
         putArgs(BX, CX,DX);
         break;
      case 0x21:
         getCurdir(BX);
         break;
      case 0x22:
         getArgc(BX);
         break;
      case 0X23:
         getArgv(BX, CX);
         break;
      case 0x10:
         pathParser(BX,CX,DX,AH);
         break;
      case 0x11:
         finder(BX,CX,AH,DX);
         break;
      case 0x12:
         isDirectory(BX,CX,AH);
         break;
      case 0x13:
         getFileSize(BX,CX,AH);
         break;
      default:
         printString("Invalid interrupt");
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

void clear(char *buffer, int length) {
   int i;
   for(i = 0; i < length; ++i) {
      buffer[i] = EMPTY;
   }
}

void readSector(char *buffer, int sector){
   interrupt(0x13, 0x201, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1, mod(div(sector, 18), 2) * 0x100);
}

void writeSector(char *buffer, int sector){
   interrupt(0x13, 0x301, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1, mod(div(sector, 18), 2) * 0x100);
}

void pathParser(char *path, char *fileName, int *dirIndex, char parentIndex){
   char parentTemp;
   char dirs[SECTOR_SIZE];
   char dirSearch[MAX_DIRECTORYNAME];
   int pathLength;
   boolean filePart;
   int lastEnd;
   int temp;
   boolean found;

   parentTemp = parentIndex;
   pathLength = stringLen(path);
   filePart = false;
   lastEnd = 0;

   readSector(dirs, LOC_DIR_SECTOR);
   
   *dirIndex = parentIndex;

   lastEnd = 0;
   while(!filePart){
      splitString(path,'/',lastEnd,&temp);
      if(temp+1==pathLength){
         filePart = true;
      }else{
         stringCopy(path,dirSearch,lastEnd,temp-lastEnd+1);
         found = false;
         *dirIndex = 0;
         while((*dirIndex)<MAX_DIRECTORIES && !found){
            if(stringCompare(dirs+(*dirIndex)*LENGTH_DIR_ENTRY+1,dirSearch,MAX_DIRECTORYNAME) && (dirs[(*dirIndex)*LENGTH_DIR_ENTRY]==parentTemp)){
               found = true;
               parentTemp = (*dirIndex);
            }else{
               (*dirIndex)++;
            }
         }   

         if(found){
            lastEnd = temp+2;
         }else{
            *dirIndex = ERROR_NOT_FOUND;
            return;
         }
      }
   }

   stringCopy(path,fileName,lastEnd,temp+1);
}

void finder(char* name,char* dir, char parent,int* idx){
   boolean found=false;
   *idx=0;
   while((*idx)<MAX_FILESYSTEM_ITEM_COUNT && !found){
      if(stringCompare(name,dir+(*idx)*LENGTH_DIR_ENTRY+1,MAX_FILENAME)&&dir[(*idx)*LENGTH_DIR_ENTRY]==parent){
         found = true;
      }else{
         (*idx)++;
      }
   }
   if(!found){
      *idx=ERROR_NOT_FOUND;
   }
}

void writeFile(char *buffer, char *path, int *sectors, char parentIndex){
   char map[SECTOR_SIZE];
   char dirs[SECTOR_SIZE];
   char files[SECTOR_SIZE];
   char saveSectors[SECTOR_SIZE];

   char dirSearch[MAX_DIRECTORYNAME];
   char fileName[MAX_FILENAME];

   char sectorBuffer[SECTOR_SIZE];
   int fileIndex;
   boolean found;
   boolean fileExists;
   int dirIndex;

   int sectorCount;

   int lastEnd;
   int temp;

   int i;
   int j;
   

   readSector(map, LOC_MAP_SECTOR);
   readSector(dirs, LOC_DIR_SECTOR);
   readSector(saveSectors,LOC_SECTOR_SECTOR);
   readSector(files,LOC_FILE_SECTOR);

   // Cari direktori
   pathParser(path,fileName,&dirIndex,parentIndex);
   if(dirIndex==ERROR_NOT_FOUND){
      *sectors = ERROR_NOT_FOUND;
      return;
   }

   // Cek jumlah sektor
   i = 0;
   sectorCount = 0;
   while(i<MAX_BYTE && sectorCount<*sectors){
      if(map[i]==EMPTY){
         sectorCount++;
      }
      i++;
   }

   if(sectorCount < *sectors){
      *sectors = ERROR_INSUFFICIENT_SECTORS;
      return;
   }else{
      // Cari lokasi file kosong
      fileIndex = 0;
      found = false;
      fileExists = false;
      while(fileIndex<MAX_FILES && !found && !fileExists){
         if (files[fileIndex * LENGTH_DIR_ENTRY+1] == '\0') {
            found = true;
         }else{
            if(stringCompare(files+fileIndex*LENGTH_DIR_ENTRY+1,fileName,0,MAX_FILENAME)&&files[fileIndex*LENGTH_DIR_ENTRY]==dirIndex){
               fileExists = true;
            }
            fileIndex++;
         }
      }

      if(fileIndex < MAX_FILES){
         if(!fileExists){
            // Entry file
            clear(files + fileIndex * LENGTH_DIR_ENTRY, LENGTH_DIR_ENTRY);
            stringCopy(fileName,files+fileIndex * LENGTH_DIR_ENTRY+ 1 ,0,MAX_FILENAME);

            files[fileIndex*LENGTH_DIR_ENTRY] = dirIndex;

            // Entry sector
            for (i = 0, sectorCount = 0; i < MAX_BYTE && sectorCount < *sectors; ++i) {
               if (map[i] == EMPTY) {
                  map[i] = USED;
                  saveSectors[fileIndex * LENGTH_DIR_ENTRY + sectorCount] = i;
                  clear(sectorBuffer, SECTOR_SIZE);
                  for (j = 0; j < SECTOR_SIZE; ++j) {
                     sectorBuffer[j] = buffer[sectorCount * SECTOR_SIZE + j];
                  }
                  writeSector(sectorBuffer, i);
                  ++sectorCount;
               }
            }

            // *sectors = 0;

            writeSector(map,LOC_MAP_SECTOR);
            writeSector(files,LOC_FILE_SECTOR);
            writeSector(dirs,LOC_DIR_SECTOR);
            writeSector(saveSectors,LOC_SECTOR_SECTOR);

         }else{
            *sectors = ERROR_ALREADY_EXISTS;
            return;
         }
      }else{
         *sectors = ERROR_INSUFFICIENT_ENTRIES;
         return;
      }
   }
}

void readFile(char *buffer, char *path, int *result, char parentIndex){
   char directories[SECTOR_SIZE];
   char files[SECTOR_SIZE];
   char sectors[SECTOR_SIZE];
   char filename[MAX_FILENAME];
   int listOfSector[MAX_FILES];
   int dirIdx=0;
   int fileIdx=0;
   int i=0;


   readSector(directories,LOC_DIR_SECTOR);  
   readSector(files,LOC_FILE_SECTOR);
   readSector(sectors,LOC_SECTOR_SECTOR);

   pathParser(path,filename,&dirIdx,parentIndex);
   if(dirIdx==ERROR_NOT_FOUND){
      *result = ERROR_NOT_FOUND;
      return;
   }
   
   finder(filename,files,dirIdx,&fileIdx);

   if(fileIdx!=ERROR_NOT_FOUND){
      for(i=0;i<MAX_SECTORS;i++){
         listOfSector[i] = *(sectors+fileIdx*LENGTH_DIR_ENTRY+i);
      }
      i = 0;
      while(i<MAX_SECTORS && listOfSector[i]!=EMPTY){
         readSector(buffer+i*SECTOR_SIZE, listOfSector[i]);
         i++;
      }
      *result = 0;
   } else {
      *result = ERROR_NOT_FOUND;
   }
}

void printString(char *string){
   int i =0;
   while(string[i]!='\0'){
      if(string[i]=='\n'){
         interrupt(0x10,0xE00+'\r',0,0,0);
      }
      interrupt(0x10,0xE00+string[i],0,1,0);
      i++;
   }
}

void readString(char *string){
   char buffer[SECTOR_SIZE];
   char c;
   char sc;
   unsigned full;
   int counter = 0;
   int i =0;
   do{
      full = getKeyboardFull();
      c = full & 0xff;
      sc = full >> 8;
      if (c=='\b'){
         if (counter>0) {
            counter--; 
            interrupt(0x10,0xE00+'\b',0,0,0);
            interrupt(0x10,0xE00+'\0',0,0,0);
            interrupt(0x10,0xE00+'\b',0,0,0);
         }
      }else if(c!='\r'){
         interrupt(0x10,0xE00+c,0,0,0);
         buffer[counter] = c;
         counter++;
      }
   }while(c!='\r' && counter < SECTOR_SIZE-1);

   for (i;i<counter;i++){
      string[i]=buffer[i];
   }
   string[i]='\0';
   interrupt(0x10, 0xE00 + '\n', 0, 0, 0);
   interrupt(0x10, 0xE00 + '\r', 0, 0, 0);
}

void executeProgram(char *path, int segment, int *result, char parentIndex){
   char buf[SECTOR_SIZE*MAX_SECTORS];
   int i = 0;
   readFile(buf,path,result,parentIndex);

   if(*result==0){
      while(i<SECTOR_SIZE*MAX_SECTORS){
         putInMemory(segment,i,buf[i]);
         i++;
      }
      launchProgram(segment);
   }
}

void terminateProgram (int *result) {
   char shell[6];
   shell[0] = 's';
   shell[1] = 'h';
   shell[2] = 'e';
   shell[3] = 'l';
   shell[4] = 'l';
   shell[5] = '\0';
   executeProgram(shell, 0x2000, result, 0xFF);
}

void printTxt(char *filename,int x, int y,int color){
   int succ;
   char buff[SECTOR_SIZE*MAX_SECTORS];
   int i=0;
   int j=y;
   int k=x;
   readFile(buff,filename,&succ,0xFF);
   if(succ==0){
      while(buff[i]!='\0'){
         if (buff[i]=='\n'){
            j++;
            k=x;
         } else {
            putInMemory(0xB000, 0x8000 + (80 * j + k) * 2, buff[i]);
            putInMemory(0xB000, 0x8001 + (80 * j + k) * 2, color);
            k++;
         }
         i++;
      }
   }
}

void clearScreen(int height){
   int i,j;
   for (i=0;i<height;i++){
      for (j=0;j<80;j++){
         putInMemory(0xB000, 0x8000 + (80 * i + j) * 2,' ');
      }
   }
}

void putArgs (char curdir, char argc, char **argv) {
   char args[SECTOR_SIZE];
   int i, j, p;
   clear(args, SECTOR_SIZE);

   args[0] = curdir;
   args[1] = argc;
   i = 0;
   j = 0;
   for (p = 2; p < SECTOR_SIZE && i < argc; ++p) {
      args[p] = argv[i][j];
      if (argv[i][j] == '\0') {
         ++i;
         j = 0;
      }
      else {
         ++j;
      }
   }

   writeSector(args, LOC_ARGS_SECTOR);
}

void getCurdir (char *curdir) {
   char args[SECTOR_SIZE];
   readSector(args, LOC_ARGS_SECTOR);
   *curdir = args[0];
}

void getArgc (char *argc) {
   char args[SECTOR_SIZE];
   readSector(args, LOC_ARGS_SECTOR);
   *argc = args[1];
}

void getArgv (char index, char *argv) {
   char args[SECTOR_SIZE];
   int i, j, p;
   readSector(args, LOC_ARGS_SECTOR);

   i = 0;
   j = 0;
   for (p = 2; p < LOC_ARGS_SECTOR; ++p) {
      if (i == index) {
         argv[j] = args[p];
         ++j;
      }

      if (args[p] == '\0') {
         if (i == index) {
            break;
         }
         else {
            ++i;
         }
      }
   }
} 

void makeDirectory(char *path, int *result, char parentIndex){
   char directories[SECTOR_SIZE];
   char filename[MAX_FILENAME];
   boolean emptyFound;
   int dirIdx;
   int foundIdx;
   int parentIdx;
   int i;
   char temp[2];

   readSector(directories,LOC_DIR_SECTOR);
   emptyFound=false;
   dirIdx=0;
   
   while(dirIdx<MAX_DIRECTORIES && !emptyFound){
      if(directories[dirIdx*LENGTH_DIR_ENTRY]==EMPTY&&directories[dirIdx*LENGTH_DIR_ENTRY+1]=='\0'){
         emptyFound=true;
      } else {
         dirIdx++;
      }
   }

   if (emptyFound){
      pathParser(path,filename,&parentIdx,parentIndex);
      if(parentIdx==ERROR_NOT_FOUND){
         *result=ERROR_NOT_FOUND;
         return;
      }else{
         finder(filename,directories,parentIdx,&foundIdx);
         if(foundIdx!=ERROR_NOT_FOUND){
            *result=ERROR_ALREADY_EXISTS;
            return;
         }else{
            directories[dirIdx*LENGTH_DIR_ENTRY]=parentIdx;
            stringCopy(filename,directories+dirIdx*LENGTH_DIR_ENTRY+1,0,MAX_FILENAME);
            writeSector(directories,LOC_DIR_SECTOR);
            *result=0;
         }
      }
   } else {
      *result=ERROR_INSUFFICIENT_ENTRIES;
   }
}

void deleteDirectory(char *path, int *success, char parentIndex){
   char directories[SECTOR_SIZE];
   char files[SECTOR_SIZE];
   char dirName[MAX_DIRECTORYNAME];
   char filename[MAX_FILENAME];
   char test[2];
   int dirIdx;
   int i;
   int succ=1;

   readSector(directories,LOC_DIR_SECTOR);
   readSector(files,LOC_FILE_SECTOR);
   pathParser(path,dirName,&dirIdx,parentIndex);
   if (dirIdx==ERROR_NOT_FOUND){
     *success = ERROR_NOT_FOUND;
      return;
   }
   finder(dirName,directories,dirIdx,&dirIdx);
   if(dirIdx!=ERROR_NOT_FOUND){
      directories[dirIdx*LENGTH_DIR_ENTRY]=EMPTY;
      for(i=0;i<MAX_DIRECTORYNAME;i++){
         directories[dirIdx*LENGTH_DIR_ENTRY+i+1]=EMPTY;
      }
      for(i=0;i<MAX_DIRECTORIES;i++){
         if(directories[i*LENGTH_DIR_ENTRY]==dirIdx && directories[i*LENGTH_DIR_ENTRY+1]!='\0'){
            stringCopy(directories,dirName,i*LENGTH_DIR_ENTRY+1,MAX_DIRECTORYNAME);
            writeSector(directories,LOC_DIR_SECTOR);
            deleteDirectory(dirName,&succ,dirIdx);
            readSector(directories,LOC_DIR_SECTOR);
         }
      }
      for(i=0;i<MAX_FILES;i++){
         if(files[i*LENGTH_DIR_ENTRY]==dirIdx && files[i*LENGTH_DIR_ENTRY+1]!='\0'){
            stringCopy(files,filename,i*LENGTH_DIR_ENTRY+1,MAX_FILENAME);
            deleteFile(filename,&succ,dirIdx);
         }
      }
      writeSector(directories,LOC_DIR_SECTOR);
      *success=0;
   } else {
      *success = ERROR_NOT_FOUND;
   }
}

void deleteFile(char *path, int *result, char parentIndex){
   char directories[SECTOR_SIZE];
   char files[SECTOR_SIZE];
   char map[SECTOR_SIZE];
   char sectors[SECTOR_SIZE];
   char filename[MAX_FILENAME];
   char test[2];
   int dirIdx;
   int fileIdx;
   int i;
   int succ=1;

   readSector(directories,LOC_DIR_SECTOR);
   readSector(files,LOC_FILE_SECTOR);
   readSector(map,LOC_MAP_SECTOR);
   readSector(sectors,LOC_SECTOR_SECTOR);

   pathParser(path,filename,&dirIdx,parentIndex);
   if (dirIdx==ERROR_NOT_FOUND){
     *result = ERROR_NOT_FOUND;
      return;
   }

   finder(filename,files,dirIdx,&fileIdx);

   if(fileIdx!=ERROR_NOT_FOUND){
      files[fileIdx*LENGTH_DIR_ENTRY]=EMPTY;
      for(i=0;i<MAX_FILENAME;i++){
         files[fileIdx*LENGTH_DIR_ENTRY+i+1]=EMPTY;
      }
      for(i=0;i<MAX_SECTORS;i++){
         if(*(sectors+fileIdx*LENGTH_DIR_ENTRY+i) != '\0'){
            map[*(sectors+fileIdx*LENGTH_DIR_ENTRY+i)]=EMPTY;
            *(sectors+fileIdx*LENGTH_DIR_ENTRY+i)=EMPTY;
         }
      }
      
      writeSector(files,LOC_FILE_SECTOR);
      writeSector(map,LOC_MAP_SECTOR);
      writeSector(sectors,LOC_SECTOR_SECTOR);
      *result=0;
   } else {
      *result=ERROR_NOT_FOUND;
   }
}

void isDirectory(char * path,int * result, char parentIdx){
   char dirs[SECTOR_SIZE];
   char files[SECTOR_SIZE];
   char filename[MAX_FILENAME];
   int dirIdx;
   int foundIdx;

   readSector(dirs,LOC_DIR_SECTOR);
   readSector(files,LOC_FILE_SECTOR);

   pathParser(path,filename,&dirIdx,parentIdx);
   if(dirIdx!=ERROR_NOT_FOUND){
      finder(filename,dirs,dirIdx,&foundIdx);
      if(foundIdx==ERROR_NOT_FOUND){
         finder(filename,files,dirIdx,&foundIdx);
         if(foundIdx==ERROR_NOT_FOUND){
            *result = ERROR_NOT_FOUND;
         }else{
            *result = 0;
         }
      }else{
         *result = 1;
      }
   }else{
      *result=ERROR_NOT_FOUND;
   }

}

void getFileSize(char *path, int *result, char parentIndex){
   char files[SECTOR_SIZE];
   char filename[MAX_FILENAME];
   char sectors[SECTOR_SIZE];
   char tempAngka[10];

   int dirIndex;
   int fileIndex;

   readSector(files,LOC_FILE_SECTOR);
   readSector(sectors, LOC_SECTOR_SECTOR);

   pathParser(path,filename,&dirIndex,parentIndex);
   finder(filename,files,parentIndex,&fileIndex);


   *result = stringLen(sectors+fileIndex*LENGTH_DIR_ENTRY);
}