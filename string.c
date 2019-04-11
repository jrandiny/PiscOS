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

int stringLen(char* in){
    int index = 0;
    while(in[index]!='\0'){
        index++;
    }
    return index;
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

void stringConcat(char *first, char *second, char *out){
   int i;
   int j;

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
   out[j] ='\0';    
   j++; // tambah \0 di akhir
}