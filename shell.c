#include "definition.h"

void splitStringArray(char *s, char delim, int* length, char result[SHELL_MAX_PART][SHELL_MAX_STRINGLENGTH]);
void executeProgram(char concatedInput[SHELL_MAX_PART][SHELL_MAX_STRINGLENGTH], char argc, char* pathNow, int curDir);
void getPathNow(char curDir, char* pathNow);
void addToCommandHistory(char* cmd);
void getCommandHistory(char* cmd, boolean next);
void clearInput();

int mod(int a, int b);

char errMsg[SIZE_SECTOR];
char commandHistory[SHELL_MAX_HISTORY][SHELL_MAX_STRINGLENGTH];
int commandHistoryNeff = 0;
int commandHistoryCurr = 0;
int commandHistoryTravelCount = 0;

int main(){
   int length;
   int i;
   int j;
   int result;
   char curDir;
   char pathNow[MAX_PATHNAME];
   char directories[SIZE_SECTOR];
   char input[MAX_PATHNAME];
   char inputPreset[MAX_PATHNAME];
   char concatedInput[SHELL_MAX_PART][SHELL_MAX_STRINGLENGTH];
   char tempPath[SHELL_MAX_PART][SHELL_MAX_STRINGLENGTH];
   char tempCurrPath[SHELL_MAX_PART][SHELL_MAX_STRINGLENGTH];
   int pathPartCount;
   int currPathPartCount;
   int tempFindResult;
   char tempCurrDir;
   boolean cdError;
   boolean showHist = false;

   curDir = 0xFF;
   interrupt(0x21,0xFF<<8|0x04,errMsg,"e.msg",0);
   getPathNow(curDir,pathNow);
   while(1){
      interrupt(0x21,0x02,directories,LOC_DIR_SECTOR,0); // readSector directori
      interrupt(0x21,0x00,pathNow,0,0);
      interrupt(0x21,0x00,"$ ",0,0);

      if(showHist){
         interrupt(0x21,0x01,input,0,inputPreset); // ambil input
      }else{
         interrupt(0x21,0x01,input,0,0); // ambil input
      }

      if(input[0]==0x00 && input[1] == 0x48 && input[2] == '\0'){
         getCommandHistory(inputPreset,false);
         interrupt(0x21,0x15,0,0,0);
         showHist = true;
      }else if(input[0]==0x00 && input[1] == 0x50 && input[2] == '\0'){
         getCommandHistory(inputPreset,true);
         interrupt(0x21,0x15,0,0,0);
         showHist = true;
      }else{
         showHist = false;
         addToCommandHistory(input);
         commandHistoryTravelCount = 0;
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
                           interrupt(0x21,directories[tempCurrDir*SIZE_DIR_ENTRY]<<8|0x11,tempCurrPath[currPathPartCount],directories,&tempFindResult);//finder

                           //assert found
                           if(tempFindResult==ERROR_NOT_FOUND){
                              cdError = true;
                              break;
                           }
                           tempCurrDir = directories[tempFindResult*SIZE_DIR_ENTRY];
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
                  interrupt(0x21,0x00,errMsg+EMSG_INVALID_PATH*SIZE_EMSG_ENTRY,0,0);
               }
            }else{
               interrupt(0x21,0x00,errMsg+EMSG_INVALID_PATH*SIZE_EMSG_ENTRY,0,0);
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
      }
      // interrupt(0x21,0x00,"\n\r",0,0);
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
   interrupt(0x21,(curDir & 0xFF)<<8|0x6,concatedInput[0],&result); // executeProgram
   if(result==ERROR_NOT_FOUND){
      interrupt(0x21,0x00,errMsg+EMSG_PROGRAM*SIZE_EMSG_ENTRY,0,0);
      interrupt(0x21,0x00,errMsg+EMSG_NOT_FOUND*SIZE_EMSG_ENTRY,0,0);
   }
}

void getPathNow(char curDir, char* pathNow){
   char directories[SIZE_SECTOR];
   char tempName[MAX_PATHNAME];
   char tempPath[MAX_PATHNAME];
   char dirName[MAX_DIRECTORYNAME];
   char dirIdx;

   interrupt(0x21,0x2,directories,LOC_DIR_SECTOR,0); // readSector directori
   dirIdx = curDir;
   stringCopy("",tempPath,0,1);
   while(dirIdx!=ROOT){
      stringCopy(directories,dirName,dirIdx*SIZE_DIR_ENTRY+1,MAX_DIRECTORYNAME);
      stringConcat("/",dirName,tempName);
      stringConcat(tempName,tempPath,pathNow);
      stringCopy(pathNow,tempPath,0,MAX_PATHNAME);
      dirIdx = directories[dirIdx*SIZE_DIR_ENTRY];
   }
   if(curDir==ROOT){
      stringCopy("/",pathNow,0,2);
   }
}

void getCommandHistory(char* cmd, boolean next){
   if(next){
      commandHistoryTravelCount--;
   }else{
      commandHistoryTravelCount++;
   }

   if(commandHistoryTravelCount < 0){
      commandHistoryTravelCount = 0;
   }else if(commandHistoryTravelCount > commandHistoryNeff){
      commandHistoryTravelCount = commandHistoryNeff;
   }else{
      if(next){
         commandHistoryCurr++;
      }else{
         commandHistoryCurr--;
      }
   }

   if(commandHistoryCurr>=SHELL_MAX_HISTORY){
      commandHistoryCurr = 0;
   }else if(commandHistoryCurr<0){
      commandHistoryCurr = SHELL_MAX_HISTORY-1;
   }

   if(commandHistoryTravelCount==0){
      cmd[0] = '\0';
   }else{
      stringCopy(commandHistory[commandHistoryCurr],cmd,0,SHELL_MAX_STRINGLENGTH);
   }

}

void addToCommandHistory(char *cmd){
   int tempIndex;

   if(commandHistoryTravelCount!=0){
      commandHistoryCurr+=commandHistoryTravelCount;
      if(commandHistoryCurr>=SHELL_MAX_HISTORY){
         commandHistoryCurr = mod(commandHistoryCurr,SHELL_MAX_HISTORY);
      }else if(commandHistoryCurr<0){
         commandHistoryCurr += SHELL_MAX_HISTORY;
      }
   }

   tempIndex = commandHistoryCurr-1;
   if(tempIndex<0){
      tempIndex = commandHistoryNeff-1;
   }

   if(stringCompare(cmd,commandHistory[tempIndex],SHELL_MAX_STRINGLENGTH)){
      return;
   }

   if(commandHistoryNeff<SHELL_MAX_HISTORY){
      commandHistoryNeff++;
   }

   
   stringCopy(cmd,commandHistory[commandHistoryCurr],0,SHELL_MAX_STRINGLENGTH);

   commandHistoryCurr++;
   if(commandHistoryCurr>=SHELL_MAX_HISTORY){
      commandHistoryCurr = 0;
   }
}

void clearInput(){
   int i;
   char clean[2];

   clean[0] = '\b';
   clean[1] = '\0';

   for(i=0;i<SHELL_MAX_STRINGLENGTH;i++){
      interrupt(0x21,0x00,clean,0,0);
   }
}

int mod(int a, int b) {
   while(a >= b) {
      a = a - b;
   }
   return a;
}