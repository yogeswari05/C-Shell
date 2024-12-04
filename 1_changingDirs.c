#include "allFunctions.h"
// re re changed
// Get the current working directory
int getCurrWorkingDir(char curWorkingDir[PATH_MAX_], size_t size){
   if (getcwd(curWorkingDir, size) != NULL) {
      // printf("pwd from fnuc: %s\n", curWorkingDir);
      return 0;
   }
   else {
      perror("getcwd() error");
      return -1;
   }
}

void getHostName_(char hostname[MAX], int size){
   if (gethostname(hostname, size) != 0) {
      perror("Error in getting host name");
      return;
   }
}

void getUserName(char username[MAX], int size){
   struct passwd *pw = getpwuid(getuid());
   if (pw == NULL) {
      perror("Error in getting puid");
      return;
   }
   snprintf(username, size, "%s", pw->pw_name); // Safely copy the username to the buffer
}

int changeDirectory(char path[PATH_MAX_], char curWorkingDir[PATH_MAX_], char startDirectory[PATH_MAX_], char prevDir[PATH_MAX_]){
   // handle '~' at the beginning of the path
   if(strcmp(path, "..") == 0 && strcmp(curWorkingDir, "~") == 0) // to ignore leaving shell directory
      return 1;
   if (path[0] == '~') {
      char temp[PATH_MAX_];
      strcpy(temp, startDirectory);
      strcat(temp, path + 1);
      strcpy(path, temp);
   }
   // printf("pathh: %s\n", path);
   // change directory
   char tteemmpp[PATH_MAX_];
   strcpy(tteemmpp, curWorkingDir);
   if (chdir(path) != 0) {
      // perror("chdir failed");
      printf("\033[1;31mError: There's no such directory.\033[0m\n");
      return -1;
   }
   else{
      strcpy(prevDir, tteemmpp);
      flag_prevDir__ = 1;
   }

   // update the current working directory
   if (getcwd(curWorkingDir, PATH_MAX_) == NULL) {
      perror("getcwd failed");
      return -1;
   }  
   // remove the startDirectory prefix from curWorkingDir
   size_t startDirLen = strlen(startDirectory);
   if (strncmp(curWorkingDir, startDirectory, startDirLen) == 0) {
      // shift the curWorkingDir to skip the startDirectory part
      memmove(curWorkingDir, curWorkingDir + startDirLen, strlen(curWorkingDir) - startDirLen + 1);      
      // if the result is an empty string, replace it with "/"
      if (strlen(curWorkingDir) == 0) {
         strcpy(curWorkingDir, "~");
      }
   }  
   if(strcmp(curWorkingDir, "~") == 0) {
      printf("%s\n", startDirectory);
   }
   else {
      printf("%s%s\n", startDirectory, curWorkingDir);
   }
   return 0; // Success
}
