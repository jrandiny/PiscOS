//loadFile.c
//Michael Black, 2007
//
//Loads a file into the file system
//This should be compiled with gcc and run outside of the OS

#include <stdio.h>

#define MAX_BYTE 256
#define SIZE_SECTOR 512
#define MAX_ENTRIES 32
#define NAME_OFFSET 1
#define MAX_NAME 15
#define MAX_SECTORS 16
#define ENTRY_LENGTH 16
#define LOC_MAP_SECTOR 256
#define LOC_FILE_SECTOR 258
#define LOC_SECTOR_SECTOR 259
#define ERROR_NOT_FOUND -1

void readSector (char *buffer, FILE *file, int sector) {
  fseek(file, sector * SIZE_SECTOR, SEEK_SET);
  int i;
	for (i = 0; i < SIZE_SECTOR; ++i) {
		buffer[i] = fgetc(file);
  }
}

void writeSector (char *buffer, FILE *file, int sector) {
  fseek(file, sector * SIZE_SECTOR, SEEK_SET);
  int i;
  for (i = 0; i < SIZE_SECTOR; ++i) {
		fputc(buffer[i], file);
  }
}

void copySector (FILE *src, int srcSector, FILE *dest, int destSector) {
  fseek(src, srcSector * SIZE_SECTOR, SEEK_SET);
  fseek(dest, destSector * SIZE_SECTOR, SEEK_SET);
  int i;
  for (i = 0; i < SIZE_SECTOR; ++i) {
    if (feof(src)) {
      fputc(0x00, dest);
      return;
    }
    else {
      fputc(fgetc(src), dest);
    }
  }
}

void writeName (char *entries, int index, char *name) {
  int i;
  for (i = 0; name[i] != '\0'; ++i) {
    entries[index * ENTRY_LENGTH + NAME_OFFSET + i] = name[i];
  }
  for (; i < MAX_NAME; ++i) {
    entries[index * ENTRY_LENGTH + NAME_OFFSET + i] = '\0';
  }
}

int findUnusedEntry (char *entries) {
  int i;
  for (i = 0; i < MAX_ENTRIES; ++i) {
    if (entries[i * ENTRY_LENGTH + NAME_OFFSET] == '\0') {
      return i;
    }
  }
  return ERROR_NOT_FOUND;
}

int findUnusedSector (char *map) {
  int i;
  for (i = 0; i < MAX_BYTE; ++i) {
    if (map[i] == 0x00) {
      return i;
    }
  }
  return ERROR_NOT_FOUND;
}

int main (int argc, char* argv[]) {
	if (argc < 2) {
		printf("Specify file name to load\n");
		return;
	}

	//open the source file
	FILE* loadFile;
	loadFile = fopen(argv[1], "r");
	if (loadFile == 0) {
		printf("File not found\n");
		return;
	}

	//open the floppy image
	FILE* floppy;
	floppy= fopen("floppya.img","r+");
	if (floppy==0) {
		printf("floppya.img not found\n");
		return;
	}

	//load map sector
	char map[SIZE_SECTOR];
	readSector(map, floppy, LOC_MAP_SECTOR);
    
  //load files sector
	char files[SIZE_SECTOR];
	readSector(files, floppy, LOC_FILE_SECTOR);
    
  //load sectors sector
	char sectors[SIZE_SECTOR];
	readSector(sectors, floppy, LOC_SECTOR_SECTOR);

	//find a free entry
  int index = findUnusedEntry(files);
  if (index != ERROR_NOT_FOUND) {
    int sectorCount = 0;
    while (!feof(loadFile)) {
      int sector = findUnusedSector(map);
      if (sector != ERROR_NOT_FOUND) {
        copySector(loadFile, sectorCount, floppy, sector);
        printf("Loaded %s to sector %d\n", argv[1], sector);
        map[sector] = 0xFF;
        sectors[index * ENTRY_LENGTH + sectorCount] = sector;
        ++sectorCount;
      }
      else {
        printf("Cannot load more files: sectors full\n");
        return;
      }
    }
    files[index * ENTRY_LENGTH] = 0xFF;
    writeName(files, index, argv[1]);
    writeSector(map, floppy, LOC_MAP_SECTOR);
    writeSector(files, floppy, LOC_FILE_SECTOR);
    writeSector(sectors, floppy, LOC_SECTOR_SECTOR);
  }
  else {
    printf("Cannot load more files: reached max files\n");
  }

	fclose(floppy);
	fclose(loadFile);
}
