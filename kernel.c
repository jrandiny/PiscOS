#define MAX_BYTE 256
#define SECTOR_SIZE 512
#define MAX_FILES 16
#define MAX_FILENAME 12
#define MAX_SECTORS 20
#define DIR_ENTRY_LENGTH 32
#define MAP_SECTOR 1
#define DIR_SECTOR 2
#define TRUE 1
#define FALSE 0
#define INSUFFICIENT_SECTORS 0
#define NOT_FOUND -1
#define INSUFFICIENT_DIR_ENTRIES -1
#define EMPTY 0x00
#define USED 0xFF

void handleInterrupt21 (int AX, int BX, int CX, int DX);
void printString(char *string);
void readString(char *string);
void readSector(char *buffer, int sector);
void writeSector(char *buffer, int sector);
void readFile(char *buffer, char *filename, int *success);
void writeFile(char *buffer, char *filename, int *sectors);
void executeProgram(char *filename, int segment, int *success);

int stringCompare(char* a, char* b, int length);
int mod(int a, int b);
int div(int a, int b);
void clear(char *buffer, int length);
void printTxt(char * filename,int x, int y, int color);
void clearScreen(int height);

int main() {
   char tempFile[SECTOR_SIZE*MAX_SECTORS];
   char test[SECTOR_SIZE];
   int suc = 0;
   makeInterrupt21();
   clearScreen(30);
   printTxt("title.txt",20,0,0xF);
   printTxt("logo.txt",25,5,0x6);
   printString("Press any key to continue...");
   interrupt(0x16, 0, 0, 0, 0);
   while (1){
      printString("ketik:");
      readString(test);
      if (stringCompare(test,"./calc",SECTOR_SIZE)){
         executeProgram("program",0x2000,&suc);
      } else if(stringCompare(test,"./keyproc",SECTOR_SIZE)) {
         executeProgram("keyproc",0x2000,&suc);
      } else if (stringCompare(test,"./readFile",SECTOR_SIZE)) {
         printString("Nama File");
         readString(test);
         readFile(tempFile,test,&suc);
         if (suc){
            printString(tempFile);
         } else {
            printString("Gagal buka file");
         }
      }else {
         printString(test);
      }
   }
}

void handleInterrupt21 (int AX, int BX, int CX, int DX){
   switch (AX) {
      case 0x0:
         printString(BX);
         break;
      case 0x1:
         readString(BX);
         break;
      case 0x2:
         readSector(BX, CX);
         break;
      case 0x3:
         writeSector(BX, CX);
         break;
      case 0x4:
         readFile(BX, CX, DX);
         break;
      case 0x5:
         writeFile(BX, CX, DX);
         break;
      case 0x6:
         executeProgram(BX, CX, DX);
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

int stringCompare(char* a, char* b, int length){
   int isSame = 1;
   int index = 0;

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


void readSector(char *buffer, int sector){
   interrupt(0x13, 0x201, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1, mod(div(sector, 18), 2) * 0x100);
}

void writeSector(char *buffer, int sector){
   interrupt(0x13, 0x301, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1, mod(div(sector, 18), 2) * 0x100);
}


void writeFile(char *buffer, char *filename, int *sectors){
   char map[SECTOR_SIZE];
   char dir[SECTOR_SIZE];
   char sectorBuffer[SECTOR_SIZE];
   int dirIndex;

   readSector(map, MAP_SECTOR);
   readSector(dir, DIR_SECTOR);

   for (dirIndex = 0; dirIndex < MAX_FILES; ++dirIndex) {
      if (dir[dirIndex * DIR_ENTRY_LENGTH] == '\0') {
         break;
      }
   }

   if (dirIndex < MAX_FILES) {
      int i, j, sectorCount;
      for (i = 0, sectorCount = 0; i < MAX_BYTE && sectorCount < *sectors; ++i) {
         if (map[i] == EMPTY) {
            ++sectorCount;
         }
      }

      if (sectorCount < *sectors) {
         *sectors = INSUFFICIENT_SECTORS;
         return;
      } else {
         clear(dir + dirIndex * DIR_ENTRY_LENGTH, DIR_ENTRY_LENGTH);
         for (i = 0; i < MAX_FILENAME; ++i) {
            if (filename[i] != '\0') {
               dir[dirIndex * DIR_ENTRY_LENGTH + i] = filename[i];
            } else {
               break;
            }
         }

         for (i = 0, sectorCount = 0; i < MAX_BYTE && sectorCount < *sectors; ++i) {
            if (map[i] == EMPTY) {
               map[i] = USED;
               dir[dirIndex * DIR_ENTRY_LENGTH + MAX_FILENAME + sectorCount] = i;
               clear(sectorBuffer, SECTOR_SIZE);
               for (j = 0; j < SECTOR_SIZE; ++j) {
                  sectorBuffer[j] = buffer[sectorCount * SECTOR_SIZE + j];
               }
               writeSector(sectorBuffer, i);
               ++sectorCount;
            }
         }
      }
   } else { 
      *sectors = INSUFFICIENT_DIR_ENTRIES;
      return;
   }

   writeSector(map, MAP_SECTOR);
   writeSector(dir, DIR_SECTOR);
}



void readFile(char *buffer, char *filename, int *success){
   char directory[SECTOR_SIZE];
   int listOfSector[MAX_FILES];
   int* tempSectorAddr;
   int fileIdx=0;
   int fileFound = 0;
   int i = 0;
   *success = 0;
   readSector(directory,DIR_SECTOR);  
   
   while(fileIdx<SECTOR_SIZE && !fileFound){
      if(stringCompare(filename,directory+fileIdx,MAX_FILENAME)){
         fileFound = 1;
      }else{
         fileIdx += DIR_ENTRY_LENGTH;
      }
   }

   if(fileFound){
      for(i=0;i<MAX_FILES;i++){
         listOfSector[i] = *(directory+fileIdx+MAX_FILENAME+i);
      }

      i = 0;

      while(i<MAX_SECTORS && listOfSector[i]!=EMPTY){
         readSector(buffer+i*SECTOR_SIZE, listOfSector[i]);
         i++;
      }
      *success = 1;
   }
}

void printString(char *string){
   int i =0;
   while(string[i]!='\0'){
      interrupt(0x10,0xE00+string[i],0,0,0);
      i++;
   }
   interrupt(0x10, 0xE00 + '\n', 0, 0, 0) ;
   interrupt(0x10, 0xE00 + '\r', 0, 0, 0) ;
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

void executeProgram(char *filename, int segment, int *success){
   char buf[SECTOR_SIZE*MAX_SECTORS];
   int i = 0;

   readFile(buf,filename,success);

   if(*success){
      while(i<SECTOR_SIZE*MAX_SECTORS){
         putInMemory(segment,i,buf[i]);
         i++;
      }
      launchProgram(segment);
   }
}

void printTxt(char *filename,int x, int y,int color){
   int succ;
   char buff[SECTOR_SIZE*MAX_SECTORS];
   int i=0;
   int j=y;
   int k=x;
   readFile(buff,filename,&succ);
   if(succ){
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