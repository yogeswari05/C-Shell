#include "allFunctions.h"
char processName[MAX] = "";
double processTime = 0.0;
char histStore[HIST_MAX_SIZE][MAX_CMDS];
int countOfHist = 0;
int curIndHist = 0;

void initiator(char hostName[PATH_MAX_], char userName[PATH_MAX_], char startDirectory[PATH_MAX_], char curWorkingDir[PATH_MAX_], char prevDir[PATH_MAX_], char homePath[PATH_MAX_]){
   getCurrWorkingDir(startDirectory, PATH_MAX_);
   getHostName_(hostName, PATH_MAX_);
   getUserName(userName, PATH_MAX_);
   getCurrWorkingDir(homePath, PATH_MAX_);
   strcpy(curWorkingDir, "~");
   getcwd(prevDir, PATH_MAX_); // Initialize prevDir with the starting directory
}

void promptHandler(char hostName[PATH_MAX_], char userName[PATH_MAX_], char curWorkingDir[PATH_MAX_]){
   if(processName[0] != '\0' && (int)processTime > 2){
      printf("\033[35m<%s@%s:%s %s : %d>\033[0m", userName, hostName, curWorkingDir, processName, (int)processTime);
      processName[0] = '\0';
      processTime = 0.0;
   }
   else{
      if(strcmp(curWorkingDir, "~") == 0)
      printf("\033[35m<%s@%s:%s>\033[0m", userName, hostName, curWorkingDir);
      else
         printf("\033[35m<%s@%s:~%s>\033[0m", userName, hostName, curWorkingDir);
   }
}

int main(){
   char hostName[PATH_MAX_];
   char userName[PATH_MAX_];
   char homePath[PATH_MAX_];
   char startDirectory[PATH_MAX_]; // the direcory from which we started
   char curWorkingDir[PATH_MAX_]; // dirctory in which we r working now 
   char prevDir[PATH_MAX_]; // to store
   initiator(hostName, userName, startDirectory, curWorkingDir, prevDir, homePath);
   setupSignalHandling();
   loadHistory(startDirectory);

   while (1){
      promptHandler(hostName, userName, curWorkingDir);
      char command[INP_MAX];
      if (fgets(command, sizeof(command), stdin) == NULL) {
         perror("fgets");
         break;  // Exit on input error
      }
      // Remove the newline character that fgets includes
      command[strcspn(command, "\n")] = '\0';
      char *commandList[MAX_CMDS];
      if (strcmp(command, "exit") == 0){
         printf("Terminating the shell...!\n");
         break;
      }
      tokenizeExecuteCommands(command, commandList, curWorkingDir, prevDir, homePath, startDirectory);
   }
   return 0;
}