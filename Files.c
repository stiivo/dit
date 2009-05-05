
#define _GNU_SOURCE
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>

#include "Prototypes.h"

/*{

typedef bool(*Method_Files_fileHandler)(void*, char*);
  
}*/

FILE* Files_open(const char* mode, const char* picture, const char* value) {
   char fileName[4097];
   char* dataDir = PKGDATADIR;
   char* sysconfDir = SYSCONFDIR;
   FILE* fd = Files_openHome(mode, picture, value);
   if (fd) return fd;

   snprintf(fileName, 4096, "%s/dit/", sysconfDir);
   int len = strlen(fileName);
   snprintf(fileName + len, 4096 - len, picture, value);
   fd = fopen(fileName, mode);
   if (fd) return fd;

   snprintf(fileName, 4096, "%s/", dataDir);
   len = strlen(fileName);
   snprintf(fileName + len, 4096 - len, picture, value);
   fd = fopen(fileName, mode);

   return fd;
}

static void Files_nameHome(char* fileName, const char* picture, const char* value) {
   char* homeDir = getenv("HOME");
   snprintf(fileName, 4096, "%s/.dit/", homeDir);
   int len = strlen(fileName);
   snprintf(fileName + len, 4096 - len, picture, value);
}

bool Files_existsHome(const char* picture, const char* value) {
   char fileName[4097];
   Files_nameHome(fileName, picture, value);
   return (access(fileName, F_OK) == 0);
}

FILE* Files_openHome(const char* mode, const char* picture, const char* value) {
   char fileName[4097];
   Files_nameHome(fileName, picture, value);
   return fopen(fileName, mode);
}

int Files_deleteHome(const char* picture, const char* value) {
   char fileName[4097];
   Files_nameHome(fileName, picture, value);
   return unlink(fileName);
}

void Files_encodePathInFileName(char* fileName, char* encodedFileName) {
   char rpath[4097];
   realpath(fileName, rpath);
   for(char *c = rpath; *c; c++)
      if (*c == '/')
         *c = ':';
   strncpy(encodedFileName, rpath, 4096);
   encodedFileName[4095] = '\0';
}

void Files_makeHome() {
   static const char* dirs[] = {
      "%s/.dit",
      "%s/.dit/undo",
      "%s/.dit/lock",
      NULL
   };
   char fileName[4097];
   const char* home = getenv("HOME");
   for (int i = 0; dirs[i]; i++) {
      snprintf(fileName, 4096, dirs[i], home);
      fileName[4095] = '\0';
      mkdir(fileName, 0700);
   }
}

void Files_forEachInDir(char* dirName, Method_Files_fileHandler fileHandler, void* data) {
   bool result = false;
   FILE* fd;
   char homeName[4097];
   char* homeDir = getenv("HOME");
   snprintf(homeName, 4096, "%s/.dit/", homeDir);
   int homeLen = strlen(homeName);
   snprintf(homeName + homeLen, 4096 - homeLen, "%s", dirName);
   DIR* dir = opendir(homeName);
   while (dir) {
      struct dirent* entry = readdir(dir);
      if (!entry) break;
      if (entry->d_name[0] == '.') continue;
      snprintf(homeName + homeLen, 4096 - homeLen, "%s/%s", dirName, entry->d_name);
      if (fileHandler(data, homeName)) {
         if (dir) closedir(dir);
         return;
      }
   }
   char dataName[4097];
   char* dataDir = PKGDATADIR;
   snprintf(dataName, 4096, "%s/", dataDir);
   int dataLen = strlen(dataName);
   snprintf(dataName + dataLen, 4096 - homeLen, "%s", dirName);
   dir = opendir(dataName);
   while (dir) {
      struct dirent* entry = readdir(dir);
      if (!entry) break;
      if (entry->d_name[0] == '.') continue;
      snprintf(homeName + homeLen, 4096 - homeLen, "%s/%s", dirName, entry->d_name);
      if (access(homeName, R_OK) == 0)
         continue;
      snprintf(dataName + dataLen, 4096 - dataLen, "%s/%s", dirName, entry->d_name);
      if (fileHandler(data, dataName)) {
         if (dir) closedir(dir);
         return;
      }
   }
}