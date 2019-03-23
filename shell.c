#include "stdio.h"
#include "string.h"

int main(){
    char input[512];
    char temp[512];
    char currDir=0xFF;

    printf("$");
    
    
    // interrupt(0x21,0x0,"$ ",0,0);
    // interrupt(0x21,0x1,input,0,0);
    fgets(input,512,stdin);
    input[strlen(input) - 1] = '\0';

    if(stringCompare(input,"cd ",3)){
        


    }else if(stringCompare(input, "./",2)){
        char curdir;
        char argc;
        char *argv[50];
        while()
        
        
        
        curdir = 0xFF;
        argc = 2;
        argv[0] = "abc";
        argv[1] = "123";
        interrupt(0x21, 0x20, curdir, argc, argv);



    }else{
        char curdir;
        char argc;
        char *argv[50];
        int currIdx;
        int nextIdx;
        int count;
        currIdx=0;
        count=0;
        while(currIdx<stringLen(input)){
            splitString(input,' ',currIdx,&nextIdx);
            stringCopy(input,argv[count],currIdx,nextIdx,nextIdx-currIdx+1);
            currIdx=nextIdx+2;
            count++;
        }
        curdir = 0xFF;
        argc=count;
        interrupt(0x21, 0x20, curdir, argc, argv);

    }
    
}