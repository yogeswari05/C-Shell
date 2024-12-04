#include "allFunctions.h"
int flag_prevDir__ = 0;
int isDirectory(char *path){
   struct stat statBuffer;
   if(stat(path, &statBuffer) != 0)
      return 0;
   if(S_ISDIR(statBuffer.st_mode))
      return 1;
   return 0;
}
// hop case
int hopHandler(char *terms[PATH_MAX_], char curWorkingDir[PATH_MAX_], int wcount, char prevDir[PATH_MAX_], char startDirectory[PATH_MAX_]){
   // terms[0] = "hop" already
   int flag = 0;
   if(wcount == 1){
      // "hop" case
      // strcpy(prevDir, curWorkingDir); 
      // printf("updated_prevDir: %s\n", curWorkingDir);
      changeDirectory(startDirectory, curWorkingDir, startDirectory, prevDir);
   }
   for(int i = 1; i < wcount; i++){
      if(strcmp(terms[i], "~") == 0){
         // strcpy(prevDir, curWorkingDir);
         if(changeDirectory(startDirectory, curWorkingDir, startDirectory, prevDir) == -1)
            flag = 1;
      }
      // 'hop -' case
      else if(strcmp(terms[i], "-") == 0){
         if(flag_prevDir__ == 0){
            printf("\033[1;31mError: Previous directory not yet set.\033[0m\n");
         }
         else{
            // printf("curDir: %s\n", curWorkingDir);
            // printf("prevDir: %s\n", prevDir);
            char temp[PATH_MAX_];
            strcpy(temp, curWorkingDir); // Store current directory in temp
            char tempPrev[PATH_MAX_];
            strcpy(tempPrev, startDirectory);
            strcat(tempPrev, "/");
            if(strcmp(prevDir, "~") != 0){
               strcat(tempPrev, prevDir);
            }
            if (changeDirectory(tempPrev, curWorkingDir, startDirectory, prevDir) == -1)
               flag = 1;
            strcpy(prevDir, temp);
         }
      }
      else{
         // strcpy(prevDir, curWorkingDir);
         if(changeDirectory(terms[i], curWorkingDir, startDirectory, prevDir) == -1)
            flag = 1;
      }
      // Update current directory after each change
   }
   return 1;
}
