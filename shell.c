#include "definition.h"

void splitStringArray(char *s, char delim, int* length, char result[SHELL_MAX_PART][SHELL_MAX_STRINGLENGTH]);
void executeProgram(char concatedInput[SHELL_MAX_PART][SHELL_MAX_STRINGLENGTH], char argc, char* pathNow, int curDir);
void getPathNow(char curDir, char* pathNow);

int main(){
   int length;
   int i;
   int j;
   int result;
   char curDir;
   char pathNow[MAX_PATHNAME];
   char directories[SECTOR_SIZE];
   char input[MAX_PATHNAME];
   char concatedInput[SHELL_MAX_PART][SHELL_MAX_STRINGLENGTH];
   char tempPath[SHELL_MAX_PART][SHELL_MAX_STRINGLENGTH];
   char tempCurrPath[SHELL_MAX_PART][SHELL_MAX_STRINGLENGTH];
   int pathPartCount;
   int currPathPartCount;
   int tempFindResult;
   char tempCurrDir;
   boolean cdError;

   interrupt(0x21,0x21,&curDir,0,0); // ambil directori sekarang
   getPathNow(curDir,pathNow);
   while(1){
      interrupt(0x21,0x02,directories,LOC_DIR_SECTOR,0); // readSector directori
      interrupt(0x21,0x00,pathNow,0,0);
      interrupt(0x21,0x00,"$ ",0,0);
      interrupt(0x21,0x01,input,0,0); // ambil input
      splitStringArray(input,' ',&length,concatedInput);//split input
      if(stringCompare(concatedInput[0],"cd",2)){
         if(length==2){
            cdError = false;
            if(concatedInput[1][0]=='/'){
               tempCurrDir = ROOT;
               splitStringArray(concatedInput[1]+1,'/',&pathPartCount,&tempPath);
            }else{
               tempCurrDir = curDir;
               splitStringArray(concatedInput[1],'/',&pathPartCount,&tempPath);
            }

            splitStringArray(pathNow+1,'/',&currPathPartCount,&tempCurrPath);
            
            for(i=0;i<pathPartCount;i++){
               if(stringCompare(tempPath[i],"..",2)){
                  // Up directory
                  if(tempCurrPath[currPathPartCount-1][0]!='\0'){
                     if(currPathPartCount>=1){
                        currPathPartCount--;
                        interrupt(0x21,directories[tempCurrDir*LENGTH_DIR_ENTRY]<<8|0x11,tempCurrPath[currPathPartCount],directories,&tempFindResult);//finder

                        //assert found
                        if(tempFindResult==ERROR_NOT_FOUND){
                           cdError = true;
                           break;
                        }
                        tempCurrDir = directories[tempFindResult*LENGTH_DIR_ENTRY];
                     }
                  }
               }else if(tempPath[i][0]!='\0'){
                  // Go to
                  interrupt(0x21,tempCurrDir<<8|0x11,tempPath[i],directories,&tempFindResult);//finder
                  //Assert found
                  if(tempFindResult==ERROR_NOT_FOUND){
                     cdError = true;
                     break;
                  }
                  currPathPartCount++;
                  stringCopy(tempPath[i],tempCurrPath[currPathPartCount-1],0,SHELL_MAX_STRINGLENGTH);

                  tempCurrDir = (char)tempFindResult;
               }
            }

            if(!cdError){
               curDir = tempCurrDir;
               getPathNow(curDir,pathNow);
            }else{
               interrupt(0x21,0x00,"Error, invalid path\n\r",0,0);
            }
         }else{
            interrupt(0x21,0x00,"Error, invalid path\n\r",0,0);
         }
      }else if(stringCompare(concatedInput[0], "./",2)){ // run program dari currDir
         for(i=0;i<stringLen(concatedInput[0])-2;i++){
            concatedInput[0][i]=concatedInput[0][i+2];//delete ./ dari command
         }
         concatedInput[0][i]='\0';

         executeProgram(concatedInput, length-1, pathNow, curDir<<8 | curDir);
      }else{ // run program dari root
         executeProgram(concatedInput, length-1, pathNow, curDir<<8|0xFF);
      }
      interrupt(0x21,0x00,"\n\r",0,0);
   }
}

void splitStringArray(char *s, char delim, int* length, char result[SHELL_MAX_PART][SHELL_MAX_STRINGLENGTH]){
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

void executeProgram(char concatedInput[SHELL_MAX_PART][SHELL_MAX_STRINGLENGTH], char argc, char* pathNow, int curDir){
   char* argv[50];
   int result;
   int i;
   
   for(i=1;i<=argc;i++){
      argv[i-1]=concatedInput[i];
   }
   interrupt(0x21,0x20,((curDir>>8) & 0xFF),argc,argv); // putArgs
   interrupt(0x21,(curDir & 0xFF)<<8|0x6,concatedInput[0],0x2000,&result); // executeProgram
   if(result!=0){
      interrupt(0x21,0x00,"No program found\n\r",0,0);
   }
}

void getPathNow(char curDir, char* pathNow){
   char directories[SECTOR_SIZE];
   char tempName[MAX_PATHNAME];
   char tempPath[MAX_PATHNAME];
   char dirName[MAX_DIRECTORYNAME];
   char dirIdx;

   interrupt(0x21,0x2,directories,LOC_DIR_SECTOR,0); // readSector directori
   dirIdx = curDir;
   stringCopy("",tempPath,0,1);
   while(dirIdx!=ROOT){
      stringCopy(directories,dirName,dirIdx*LENGTH_DIR_ENTRY+1,MAX_DIRECTORYNAME);
      stringConcat("/",dirName,tempName);
      stringConcat(tempName,tempPath,pathNow);
      stringCopy(pathNow,tempPath,0,MAX_PATHNAME);
      dirIdx = directories[dirIdx*LENGTH_DIR_ENTRY];
   }
   if(curDir==ROOT) stringCopy("/",pathNow,0,2);
}