#define MAIN
#include "proc.h"
#include "definition.h"

void handleInterrupt21 (int AX, int BX, int CX, int DX);
void handleTimerInterrupt(int segment, int stackPointer);
// String syscall
void printString(char *string);
void clearLine();
void readString(char *string, int disableProcessControls,char* preset);
// void readString(char *string);
// Program syscall
void executeProgram (char *path, int *result, char parentIndex, boolean par);
// void executeProgram(char *path, int segment, int *result, char parentIndex);
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
void consistentWriteFile(char *buffer, char *path, int *sectors, char parentIndex);
void writeFile(char *buffer, char *path, int *sectors, char parentIndex);
void deleteDirectory(char *path, int *success, char parentIndex);
void deleteFile(char *path, int *result, char parentIndex);
void makeDirectory(char *path, int *result, char parentIndex);
void isDirectory(char * path,int * result, char parentIdx);
void getFileSize(char *path, int *result, char parentIndex);
// Process syscall
void yieldControl();
void sleep();
void pauseProcess(int segment, int *result);
void resumeProcess(int segment, int *result);
void killProcess(int segment, int *result);

// Helper function
void clear(char *buffer, int length);
void printLogo();
void clearScreen(int height);
void pathParser(char *path, char *fileName, int *dirIndex, char parentIndex);
void finder(char* name,char* dir, char parent,int* idx);
int mod(int a, int b);
int div(int a, int b);
void getPCB(int index, struct PCB* result);

int main() {
   int suc = 0;
   // struct PCB pcb;
   
   initializeProcStructures();
   makeInterrupt21();
   makeTimerInterrupt();

   // printLogo();
   interrupt(0x10,0x3,0,0,0);
   printString("...");

   interrupt(0x16, 0, 0, 0, 0);
   interrupt(0x10,0x3,0,0,0);
   // interrupt(0x10,0xE00+'\n',0,0,0);

   interrupt(0x21,0xFF<<8|0x6, "shell",&suc);
   
   while(1){}
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
         readString(BX,CX,DX);
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
         executeProgram(BX, CX, AH, DX);
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
      case 0x14:
         consistentWriteFile(BX,CX,DX,AH);
         break;
      case 0x15:
         clearLine();
         break;
      case 0x16:
         getPCB(BX,CX);
         break;
      case 0x30:
         yieldControl();
         break;
      case 0x31:
         sleep();
         break;
      case 0x32:
         pauseProcess(BX,CX);
         break;
      case 0x33:
         resumeProcess(BX,CX);
         break;
      case 0x34:
         killProcess(BX,CX);
         break;
      default:
         printString("itr err");
   }
}

void handleTimerInterrupt(int segment, int stackPointer) {
   struct PCB *currPCB;
   struct PCB *nextPCB;

   setKernelDataSegment();


   currPCB = getPCBOfSegment(segment);
   currPCB->stackPointer = stackPointer;
   if (currPCB->state != PAUSED) {
      currPCB->state = READY;
      addToReady(currPCB);
   }
   do {
      nextPCB = removeFromReady();
   }
   while (nextPCB != NULL && (nextPCB->state == DEFUNCT || nextPCB->state == PAUSED));

   if (nextPCB != NULL) {
      nextPCB->state = RUNNING;
      segment = nextPCB->segment;
      stackPointer = nextPCB->stackPointer;
      running = nextPCB;
   } else {
      running = &idleProc;
   }

   restoreDataSegment();
   returnFromTimer(segment, stackPointer);
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
   char dirs[SIZE_SECTOR];
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
            if(stringCompare(dirs+(*dirIndex)*SIZE_DIR_ENTRY+1,dirSearch,MAX_DIRECTORYNAME) && (dirs[(*dirIndex)*SIZE_DIR_ENTRY]==parentTemp)){
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
      if(stringCompare(name,dir+(*idx)*SIZE_DIR_ENTRY+1,MAX_FILENAME)&&dir[(*idx)*SIZE_DIR_ENTRY]==parent){
         found = true;
      }else{
         (*idx)++;
      }
   }
   if(!found){
      *idx=ERROR_NOT_FOUND;
   }
}

void consistentWriteFile(char *buffer, char *path, int *sectors, char parentIndex){
   writeFile(buffer,path,sectors,parentIndex);
   if(*sectors==ERROR_INTERNAL_WRITEFILE_INSUFFICIENT_SECTORS){
      *sectors = ERROR_INSUFFICIENT_SECTORS;
   }else if(*sectors>0){
      *sectors = 0;
   }
}

void writeFile(char *buffer, char *path, int *sectors, char parentIndex){
   char map[SIZE_SECTOR];
   char dirs[SIZE_SECTOR];
   char files[SIZE_SECTOR];
   char saveSectors[SIZE_SECTOR];

   char dirSearch[MAX_DIRECTORYNAME];
   char fileName[MAX_FILENAME];

   char sectorBuffer[SIZE_SECTOR];
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
         if (files[fileIndex * SIZE_DIR_ENTRY+1] == '\0') {
            found = true;
         }else{
            if(stringCompare(files+fileIndex*SIZE_DIR_ENTRY+1,fileName,MAX_FILENAME)&&files[fileIndex*SIZE_DIR_ENTRY]==dirIndex){
               fileExists = true;
            }
            fileIndex++;
         }
      }

      if(fileIndex < MAX_FILES){
         if(!fileExists){
            // Entry file
            clear(files + fileIndex * SIZE_DIR_ENTRY, SIZE_DIR_ENTRY);
            stringCopy(fileName,files+fileIndex * SIZE_DIR_ENTRY+ 1 ,0,MAX_FILENAME);

            files[fileIndex*SIZE_DIR_ENTRY] = dirIndex;

            // Entry sector
            for (i = 0, sectorCount = 0; i < MAX_BYTE && sectorCount < *sectors; ++i) {
               if (map[i] == EMPTY) {
                  map[i] = USED;
                  saveSectors[fileIndex * SIZE_DIR_ENTRY + sectorCount] = i;
                  clear(sectorBuffer, SIZE_SECTOR);
                  for (j = 0; j < SIZE_SECTOR; ++j) {
                     sectorBuffer[j] = buffer[sectorCount * SIZE_SECTOR + j];
                  }
                  writeSector(sectorBuffer, i);
                  ++sectorCount;
               }
            }

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
   char files[SIZE_SECTOR];
   char sectors[SIZE_SECTOR];
   char filename[MAX_FILENAME];
   int listOfSector[MAX_SECTORS];
   int dirIdx=0;
   int fileIdx=0;
   int i=0;

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
         listOfSector[i] = *(sectors+fileIdx*SIZE_DIR_ENTRY+i);
      }
      i = 0;
      while(i<MAX_SECTORS && listOfSector[i]!=EMPTY){
         readSector(buffer+i*SIZE_SECTOR, listOfSector[i]);
         i++;
      }
      *result = fileIdx;
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

void clearLine(){
   int i;
   
   for(i=0;i<SHELL_MAX_STRINGLENGTH;i++){
      interrupt(0x10,0xE00+'\b',0,0,0);
      interrupt(0x10,0xE00+'\0',0,0,0);
      interrupt(0x10,0xE00+'\b',0,0,0);
   }
}

void readString(char *string, int disableProcessControls, char* preset){
   char buffer[SIZE_SECTOR];
   char c;
   char sc;
   unsigned full;
   int counter = 0;
   int i =0;
   int suc = 0;
   boolean doneReading = false;
   boolean command = false;

   if(preset!=0){
      counter = stringLen(preset);
      stringCopy(preset,buffer,0,counter);
      printString(buffer);
   }

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
      }else if(full==0x5000||full==0x4800){
         buffer[0] = full & 0xFF;
         buffer[1] = (full>>8 )& 0xFF;
         command = true;
         doneReading = true;
         counter=2;
      }else if(c==0x03 || c==0x1A){
         if(!disableProcessControls){
            interrupt(0x10,0xE00+'^',0,0,0);
            if(c==0x03){
               interrupt(0x10,0xE00+'C',0,0,0);
            }else{
               interrupt(0x10,0xE00+'Z',0,0,0);
            }
            interrupt(0x10,0xE00+'\n',0,0,0);
            interrupt(0x10,0xE00+'\r',0,0,0);

            if(c==0x03){
               terminateProgram(&suc);
            }else{
               sleep();
               resumeProcess(0x2000,&suc); // resume kernel
            }
         }
      }else if(c=='\r'){
         doneReading = true;
      }else if(c!='\r'){
         interrupt(0x10,0xE00+c,0,0,0);
         buffer[counter] = c;
         counter++;
      }
   }while(!doneReading && counter < SIZE_SECTOR-1);

   for (i;i<counter;i++){
      string[i]=buffer[i];
   }
   string[i]='\0';
   if(!command){
      interrupt(0x10, 0xE00 + '\n', 0, 0, 0);
      interrupt(0x10, 0xE00 + '\r', 0, 0, 0);
   }
}

void executeProgram (char *path, int *result, char parentIndex, boolean par) {
   struct PCB* pcb;
   int segment;
   int i, fileIndex;
   char buffer[SIZE_USABLE_STACK_MULTIPLIER * SIZE_SECTOR];
   readFile(buffer, path, result, parentIndex);
   if (*result != ERROR_NOT_FOUND) {
      setKernelDataSegment();
      segment = getFreeMemorySegment();
      restoreDataSegment();

      fileIndex = *result;
      if (segment != NO_FREE_SEGMENTS) {
         setKernelDataSegment();

         pcb = getFreePCB();
         pcb->index = fileIndex;
         pcb->state = STARTING;
         pcb->segment = segment;
         pcb->stackPointer = 0xFF00;
         pcb->parentSegment = running->segment;
         addToReady(pcb);

         restoreDataSegment();
         for (i = 0; i < SIZE_SECTOR * MAX_SECTORS; i++) {
            putInMemory(segment, i, buffer[i]);
         }
         initializeProgram(segment);
         if(!par){
            sleep();
         }
      } else {
         *result = ERROR_INSUFFICIENT_SEGMENTS;
      }
   }
} 

void terminateProgram (int *result) {
   int parentSegment;
   setKernelDataSegment();
   
   parentSegment = running->parentSegment;
   releaseMemorySegment(running->segment);
   releasePCB(running);
   
   restoreDataSegment();
   if (parentSegment != NO_PARENT) {
      resumeProcess(parentSegment, result);
   }
   yieldControl();
}

void printLogo(){
   // int succ;
   // char buff[SIZE_SECTOR];
   // int i=0;
   // int j=0;
   // int k=20;
   // int x=20;
   // char color = 0xF;

   // clearScreen(30);
   
   // readFile(buff,"lg",&succ,ROOT);
   // if(succ>=0){
   //    while(true){
   //       if(buff[i]=='\0'){
   //          if(x==20){
   //             x=25;
   //             k=25;
   //             j=5;
   //             i++;
   //             color=0x6;
   //          }else{
   //             break;
   //          }
   //       }
   //       if (buff[i]=='\n'){
   //          j++;
   //          k=x;
   //       } else {
   //          putInMemory(0xB000, 0x8000 + (80 * j + k) * 2, buff[i]);
   //          putInMemory(0xB000, 0x8001 + (80 * j + k) * 2, color);
   //          k++;
   //       }
   //       i++;
   //    }
   // }
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
   char args[SIZE_SECTOR];
   int i, j, p;
   clear(args, SIZE_SECTOR);

   args[0] = curdir;
   args[1] = argc;
   i = 0;
   j = 0;
   for (p = 2; p < SIZE_SECTOR && i < argc; ++p) {
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
   char args[SIZE_SECTOR];
   readSector(args, LOC_ARGS_SECTOR);
   *curdir = args[0];
}

void getArgc (char *argc) {
   char args[SIZE_SECTOR];
   readSector(args, LOC_ARGS_SECTOR);
   *argc = args[1];
}

void getArgv (char index, char *argv) {
   char args[SIZE_SECTOR];
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
   char directories[SIZE_SECTOR];
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
      if(directories[dirIdx*SIZE_DIR_ENTRY]==EMPTY&&directories[dirIdx*SIZE_DIR_ENTRY+1]=='\0'){
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
            directories[dirIdx*SIZE_DIR_ENTRY]=parentIdx;
            stringCopy(filename,directories+dirIdx*SIZE_DIR_ENTRY+1,0,MAX_FILENAME);
            writeSector(directories,LOC_DIR_SECTOR);
            *result=SUCCESS;
         }
      }
   } else {
      *result=ERROR_INSUFFICIENT_ENTRIES;
   }
}

void deleteDirectory(char *path, int *success, char parentIndex){
   char directories[SIZE_SECTOR];
   char files[SIZE_SECTOR];
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
      directories[dirIdx*SIZE_DIR_ENTRY]=EMPTY;
      for(i=0;i<MAX_DIRECTORYNAME;i++){
         directories[dirIdx*SIZE_DIR_ENTRY+i+1]=EMPTY;
      }
      for(i=0;i<MAX_DIRECTORIES;i++){
         if(directories[i*SIZE_DIR_ENTRY]==dirIdx && directories[i*SIZE_DIR_ENTRY+1]!='\0'){
            stringCopy(directories,dirName,i*SIZE_DIR_ENTRY+1,MAX_DIRECTORYNAME);
            writeSector(directories,LOC_DIR_SECTOR);
            deleteDirectory(dirName,&succ,dirIdx);
            readSector(directories,LOC_DIR_SECTOR);
         }
      }
      for(i=0;i<MAX_FILES;i++){
         if(files[i*SIZE_DIR_ENTRY]==dirIdx && files[i*SIZE_DIR_ENTRY+1]!='\0'){
            stringCopy(files,filename,i*SIZE_DIR_ENTRY+1,MAX_FILENAME);
            deleteFile(filename,&succ,dirIdx);
         }
      }
      writeSector(directories,LOC_DIR_SECTOR);
      *success=SUCCESS;
   } else {
      *success = ERROR_NOT_FOUND;
   }
}

void deleteFile(char *path, int *result, char parentIndex){
   char directories[SIZE_SECTOR];
   char files[SIZE_SECTOR];
   char map[SIZE_SECTOR];
   char sectors[SIZE_SECTOR];
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
      files[fileIdx*SIZE_DIR_ENTRY]=EMPTY;
      for(i=0;i<MAX_FILENAME;i++){
         files[fileIdx*SIZE_DIR_ENTRY+i+1]=EMPTY;
      }
      for(i=0;i<MAX_SECTORS;i++){
         if(*(sectors+fileIdx*SIZE_DIR_ENTRY+i) != '\0'){
            map[*(sectors+fileIdx*SIZE_DIR_ENTRY+i)]=EMPTY;
            *(sectors+fileIdx*SIZE_DIR_ENTRY+i)=EMPTY;
         }
      }
      
      writeSector(files,LOC_FILE_SECTOR);
      writeSector(map,LOC_MAP_SECTOR);
      writeSector(sectors,LOC_SECTOR_SECTOR);
      *result=SUCCESS;
   } else {
      *result=ERROR_NOT_FOUND;
   }
}

void isDirectory(char * path,int * result, char parentIdx){
   char dirs[SIZE_SECTOR];
   char files[SIZE_SECTOR];
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
            *result = SUCCESS;
         }
      }else{
         *result = 1;
      }
   }else{
      *result=ERROR_NOT_FOUND;
   }
}

void getFileSize(char *path, int *result, char parentIndex){
   char files[SIZE_SECTOR];
   char filename[MAX_FILENAME];
   char sectors[SIZE_SECTOR];
   char tempAngka[10];

   int dirIndex;
   int fileIndex;

   readSector(files,LOC_FILE_SECTOR);
   readSector(sectors, LOC_SECTOR_SECTOR);

   pathParser(path,filename,&dirIndex,parentIndex);
   finder(filename,files,parentIndex,&fileIndex);


   *result = stringLen(sectors+fileIndex*SIZE_DIR_ENTRY);
}

void yieldControl(){
   interrupt(0x08, 0, 0, 0, 0);
}

void sleep(){
   setKernelDataSegment();

   running->state = PAUSED;

   restoreDataSegment();
   yieldControl();
} 

void pauseProcess(int segment, int *result){
   struct PCB *pcb;
   int res;
   
   setKernelDataSegment();
   
   pcb = getPCBOfSegment(segment);
   if (pcb != NULL && pcb->state != PAUSED){
      pcb->state = PAUSED; res = SUCCESS;
   } else {
      res = ERROR_NOT_FOUND;
   }
   
   restoreDataSegment();
   *result = res;
} 

void resumeProcess(int segment, int *result){
   struct PCB *pcb;
   int res;
   setKernelDataSegment();
   
   pcb = getPCBOfSegment(segment);
   if (pcb != NULL && pcb->state == PAUSED) {
      pcb->state = READY;
      addToReady(pcb);
      res = SUCCESS;
   } else {
      res = ERROR_NOT_FOUND;
   }
   
   restoreDataSegment();
   *result = res;
}


void killProcess(int segment, int *result){
   struct PCB *pcb;
   int res;
   setKernelDataSegment();

   pcb = getPCBOfSegment(segment);
   if (pcb != NULL) {
      releaseMemorySegment(pcb->segment);
      releasePCB(pcb);
      res = SUCCESS;
   } else {
      res = ERROR_NOT_FOUND;
   }

   restoreDataSegment();
   *result = res;
}  

void getPCB(int index, struct PCB* result){
   struct PCB pcb;
   setKernelDataSegment();
   pcb=pcbPool[index];
   restoreDataSegment();
   *result = pcb;
}