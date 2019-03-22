#define ARGS_SECTOR 512
#define MAX_BYTE 256
#define SECTOR_SIZE 512
#define MAX_DIRECTORY 32
#define MAX_FILES 32
#define MAX_FILESYSTEM_ITEM 32
#define MAX_DIRECTORYNAME 15
#define MAX_FILENAME 15
#define MAX_SECTORS 16
#define DIR_ENTRY_LENGTH 16
#define MAP_SECTOR 256
#define DIRS_SECTOR 257
#define FILES_SECTOR 258
#define SECTORS_SECTOR 259
#define TRUE 1
#define FALSE 0
#define INSUFFICIENT_SECTORS 0
#define INSUFFICIENT_ENTRIES -3
#define ALREADY_EXISTS -2
#define NOT_FOUND -1
#define EMPTY 0x00
#define USED 0xFF

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

// Helper function
int mod(int a, int b);
int div(int a, int b);
void clear(char *buffer, int length);
void printTxt(char * filename,int x, int y, int color);
void clearScreen(int height);
void pathParser(char *path, char *fileName, int *dirIndex, char parentIndex);
void finder(char* name,char* dir, int parent,int* idx);

int main() {
   char tempFile[SECTOR_SIZE*MAX_SECTORS];
   int suc = 0;

   makeInterrupt21();
   clearScreen(30);
   printTxt("title.txt",20,0,0xF);
   printTxt("logo.txt",25,5,0x6);
   printString("Press any key to continue...");
   interrupt(0x16, 0, 0, 0, 0);

   while (1){
      readFile(tempFile,"title.txt",&suc,0xff);
      suc = 1;
      makeDirectory("abc",&suc,0xff);
      makeDirectory("abc/josh",&suc,0xff);
      makeDirectory("abc/josh",&suc,0xff);

      if(suc==ALREADY_EXISTS){
         suc =1;
         writeFile(tempFile,"abc/josh/a.txt",&suc,0xff);
         suc =1;
         writeFile(tempFile,"abc/a.txt",&suc,0xff);
         suc =1;
         writeFile(tempFile,"a.txt",&suc,0xff);
         printString("expected 3 file");
         readString(&tempFile);
         deleteFile("a.txt",&suc,0xff);
         if(suc==0){
            deleteDirectory("abc",&suc,0xff);
            printString("kalimat di bawah ini salah");
         }
      }
      printString("gagal beneran nih");
      while(1){}
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
   // char tempOut[512];
   int pathLength;
   int filePart;
   int lastEnd;
   int temp;
   int found;

   parentTemp = parentIndex;
   pathLength = stringLen(path);
   filePart = 0;
   lastEnd = 0;

   readSector(dirs, DIRS_SECTOR);
   
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
   *idx=0;
   while((*idx)<MAX_FILESYSTEM_ITEM && !found){
      if(stringCompare(name,dir+(*idx)*DIR_ENTRY_LENGTH+1,MAX_FILENAME)&&dir[(*idx)*DIR_ENTRY_LENGTH]==parent){
         found = 1;
      }else{
         (*idx)++;
      }
   }
   if(!found){
      *idx=NOT_FOUND;
   }
}

void writeFile(char *buffer, char *path, int *sectors, char parentIndex){
   char map[SECTOR_SIZE];
   char dirs[SECTOR_SIZE];
   char files[SECTOR_SIZE];
   char saveSectors[SECTOR_SIZE];

   char dirSearch[MAX_DIRECTORYNAME];
   char fileName[MAX_FILENAME];

   char temp22[3];

   char sectorBuffer[SECTOR_SIZE];
   int fileIndex;
   int found;
   int fileExists;
   int dirIndex;
   int filePart;
   int pathLength;
   char parentTemp;

   int sectorCount;

   int lastEnd;
   int temp;

   int i;
   int j;
   

   readSector(map, MAP_SECTOR);
   readSector(dirs, DIRS_SECTOR);
   readSector(saveSectors,SECTORS_SECTOR);
   readSector(files,FILES_SECTOR);

   // Cari direktori
   pathParser(path,fileName,&dirIndex,parentIndex);
   if(dirIndex==NOT_FOUND){
      *sectors = NOT_FOUND;
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
      *sectors = INSUFFICIENT_SECTORS;
      return;
   }else{
      // Cari lokasi file kosong
      fileIndex = 0;
      found = 0;
      fileExists = 0;
      while(fileIndex<MAX_FILES && !found && !fileExists){
         if (files[fileIndex * DIR_ENTRY_LENGTH+1] == '\0') {
            found = 1;
         }else{
            if(stringCompare(files+fileIndex*DIR_ENTRY_LENGTH+1,fileName,0,MAX_FILENAME)&&files[fileIndex*DIR_ENTRY_LENGTH]==dirIndex){
               fileExists = 1;
            }
            fileIndex++;
         }
      }

      if(fileIndex < MAX_FILES){
         if(!fileExists){
            // Entry file
            clear(files + fileIndex * DIR_ENTRY_LENGTH, DIR_ENTRY_LENGTH);
            stringCopy(fileName,files+fileIndex * DIR_ENTRY_LENGTH+ 1 ,0,MAX_FILENAME);

            files[fileIndex*DIR_ENTRY_LENGTH] = dirIndex;

            // Entry sector
            for (i = 0, sectorCount = 0; i < MAX_BYTE && sectorCount < *sectors; ++i) {
               if (map[i] == EMPTY) {
                  map[i] = USED;
                  saveSectors[fileIndex * DIR_ENTRY_LENGTH + sectorCount] = i;
                  clear(sectorBuffer, SECTOR_SIZE);
                  for (j = 0; j < SECTOR_SIZE; ++j) {
                     sectorBuffer[j] = buffer[sectorCount * SECTOR_SIZE + j];
                  }
                  writeSector(sectorBuffer, i);
                  ++sectorCount;
               }
            }

            *sectors = 0;

            writeSector(map,MAP_SECTOR);
            writeSector(files,FILES_SECTOR);
            writeSector(dirs,DIRS_SECTOR);
            writeSector(saveSectors,SECTORS_SECTOR);

         }else{
            *sectors = ALREADY_EXISTS;
            return;
         }
      }else{
         *sectors = INSUFFICIENT_ENTRIES;
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


   readSector(directories,DIRS_SECTOR);  
   readSector(files,FILES_SECTOR);
   readSector(sectors,SECTORS_SECTOR);

   pathParser(path,filename,&dirIdx,parentIndex);
   if(dirIdx==NOT_FOUND){
      *result = NOT_FOUND;
      return;
   }
   
   finder(filename,files,dirIdx,&fileIdx);

   if(fileIdx!=NOT_FOUND){
      for(i=0;i<MAX_SECTORS;i++){
         listOfSector[i] = *(sectors+fileIdx*DIR_ENTRY_LENGTH+i);
      }
      i = 0;
      while(i<MAX_SECTORS && listOfSector[i]!=EMPTY){
         readSector(buffer+i*SECTOR_SIZE, listOfSector[i]);
         i++;
      }
      *result = 0;
   } else {
      *result = NOT_FOUND;
   }
}

void printString(char *string){
   int i =0;
   while(string[i]!='\0'){
      interrupt(0x10,0xE00+string[i],0,1,0);
      i++;
   }
   interrupt(0x10, 0xE00 + '\n', 0, 1, 0) ;
   interrupt(0x10, 0xE00 + '\r', 0, 1, 0) ;
}

void readString(char *string){
   char buffer[SECTOR_SIZE];
   char c;
   int counter = 0;
   int i =0;
   do{
      c = interrupt(0x16, 0, 0, 0, 0);
      if (c=='\b'){
         if (counter>0) counter--; 
         interrupt(0x10,0xE00+'\b',0,0,0);
         interrupt(0x10,0xE00+'\0',0,0,0);
         interrupt(0x10,0xE00+'\b',0,0,0);
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

   if(*result){
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
   for (p = 1; p < ARGS_SECTOR && i < argc; ++p) {
      args[p] = argv[i][j];
      if (argv[i][j] == '\0') {
         ++i;
         j = 0;
      }
      else {
         ++j;
      }
   }

   writeSector(args, ARGS_SECTOR);
}

void getCurdir (char *curdir) {
   char args[SECTOR_SIZE];
   readSector(args, ARGS_SECTOR);
   *curdir = args[0];
}

void getArgc (char *argc) {
   char args[SECTOR_SIZE];
   readSector(args, ARGS_SECTOR);
   *argc = args[1];
}

void getArgv (char index, char *argv) {
   char args[SECTOR_SIZE];
   int i, j, p;
   readSector(args, ARGS_SECTOR);

   i = 0;
   j = 0;
   for (p = 1; p < ARGS_SECTOR; ++p) {
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
   int emptyFound;
   int dirIdx;
   int foundIdx;
   int parentIdx;
   int i;
   char temp[2];

   readSector(directories,DIRS_SECTOR);
   emptyFound=0;
   dirIdx=0;
   
   while(dirIdx<MAX_FILESYSTEM_ITEM && !emptyFound){
      if(*(directories+dirIdx*DIR_ENTRY_LENGTH)==EMPTY){
         emptyFound=1;
      } else {
         dirIdx++;
      }
   }

   if (emptyFound){
      pathParser(path,filename,&parentIdx,parentIndex);
      if(parentIdx==NOT_FOUND){
         *result=NOT_FOUND;
         return;
      }else{
         finder(filename,directories,parentIdx,&foundIdx);
         if(foundIdx!=NOT_FOUND){
            *result=ALREADY_EXISTS;
            return;
         }else{
            directories[dirIdx*DIR_ENTRY_LENGTH]=parentIdx;
            stringCopy(filename,directories+dirIdx*DIR_ENTRY_LENGTH+1,0,MAX_FILENAME);
            writeSector(directories,DIRS_SECTOR);
            *result=0;
         }
      }
   } else {
      *result=INSUFFICIENT_ENTRIES;
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

   readSector(directories,DIRS_SECTOR);
   readSector(files,FILES_SECTOR);
   pathParser(path,dirName,&dirIdx,parentIndex);
   if (dirIdx==NOT_FOUND){
     *success = NOT_FOUND;
      return;
   }
   finder(dirName,directories,dirIdx,&dirIdx);
   if(dirIdx!=NOT_FOUND){
      directories[dirIdx*DIR_ENTRY_LENGTH]=EMPTY;
      for(i=0;i<MAX_DIRECTORYNAME;i++){
         directories[dirIdx*DIR_ENTRY_LENGTH+i+1]=EMPTY;
      }
      for(i=0;i<MAX_DIRECTORY;i++){
         if(directories[i*DIR_ENTRY_LENGTH]==dirIdx && directories[i*DIR_ENTRY_LENGTH+1]!='\0'){
            stringCopy(directories,dirName,i*DIR_ENTRY_LENGTH+1,MAX_DIRECTORYNAME);
            writeSector(directories,DIRS_SECTOR);
            deleteDirectory(dirName,&succ,dirIdx);
            readSector(directories,DIRS_SECTOR);
         }
      }
      for(i=0;i<MAX_FILES;i++){
         if(files[i*DIR_ENTRY_LENGTH]==dirIdx && files[i*DIR_ENTRY_LENGTH+1]!='\0'){
            stringCopy(files,filename,i*DIR_ENTRY_LENGTH+1,MAX_FILENAME);
            deleteFile(filename,&succ,dirIdx);
         }
      }
      writeSector(directories,DIRS_SECTOR);
      *success=0;
   } else {
      *success = NOT_FOUND;
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

   readSector(directories,DIRS_SECTOR);
   readSector(files,FILES_SECTOR);
   readSector(map,MAP_SECTOR);
   readSector(sectors,SECTORS_SECTOR);

   pathParser(path,filename,&dirIdx,parentIndex);
   if (dirIdx==NOT_FOUND){
     *result = NOT_FOUND;
      return;
   }
   finder(filename,files,dirIdx,&fileIdx);

   if(fileIdx!=NOT_FOUND){
      files[fileIdx*DIR_ENTRY_LENGTH]=EMPTY;
      for(i=0;i<MAX_FILENAME;i++){
         files[fileIdx*DIR_ENTRY_LENGTH+i+1]=EMPTY;
      }
      for(i=0;i<MAX_SECTORS;i++){
         map[*(sectors+fileIdx*DIR_ENTRY_LENGTH+i)]=EMPTY;
         *(sectors+fileIdx*DIR_ENTRY_LENGTH+i)=EMPTY;
      }
      
      writeSector(files,FILES_SECTOR);
      writeSector(map,MAP_SECTOR);
      writeSector(sectors,SECTORS_SECTOR);
      *result=0;
   } else {
      *result=NOT_FOUND;
   }
}