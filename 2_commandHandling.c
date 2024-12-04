#include "allFunctions.h"
void trimWhitespace(char *str) {
   // Remove leading whitespace
   char *start = str;
   while (isspace((unsigned char)*start)) start++;
   // Remove trailing whitespace
   char *end = start + strlen(start) - 1;
   while (end > start && isspace((unsigned char)*end)) end--;
   // Write new null terminator character
   *(end + 1) = '\0';
   // Shift the trimmed string back to the start of the buffer
   if (start > str) {
      memmove(str, start, end - start + 2);
   }
}

void tokenizeExecuteCommands(char command[INP_MAX], char*commandList[INP_MAX], char currDir[PATH_MAX_], char prevDir[PATH_MAX_], char homePath[PATH_MAX_], char startDirectory[PATH_MAX_]){
   // Create a copy of the original command to store the full command in history
   int flag = 1;
   char cmdCpy[PATH_MAX_];
   strcpy(cmdCpy, command);
   char *token = strtok(cmdCpy, ";");
   int cmd_count = 0;
   while (token != NULL) {
      commandList[cmd_count++] = token;
      token = strtok(NULL, ";");
   }
   for (int i = 0; i < cmd_count; i++) {
      char *cmd = commandList[i];
      char subcmdCpy[INP_MAX];
      strcpy(subcmdCpy, cmd);
      char subCommandList[MAX][PATH_MAX_];
      char *subtoken = strtok(cmd, "&");
      int sub_cmd_c = 0;
      while(subtoken != NULL){
         trimWhitespace(subtoken);
         strcpy(subCommandList[sub_cmd_c], subtoken);
         subCommandList[sub_cmd_c][PATH_MAX_ - 1] = '\0'; // Ensure null termination
         sub_cmd_c++;
         subtoken = strtok(NULL, "&");
      }
      for (int j = 0; j < sub_cmd_c-1; j++){
         runInBackground(subCommandList[j]); // Execute background command
      }
      // need to check if the last command ends with & or not
      int n = strlen(subcmdCpy);
      if (subcmdCpy[n - 1] == '&')
         runInBackground(subCommandList[sub_cmd_c - 1]);
      else if (commandHandler(subCommandList[sub_cmd_c - 1], currDir, prevDir, homePath, startDirectory) == -1)
         flag = 0;
   }
   if(flag) addCommandToHist(command, startDirectory); // Store command in history`
   // k subcommands, k-1 commands will be background definitely, for kth one, check if lastchar == &
}

int commandHandler(char command[PATH_MAX_], char curWorkingDir[PATH_MAX_], char prevDir[PATH_MAX_], char homePath[PATH_MAX_], char startDirectory[PATH_MAX_]){
   // Tokenize command
   // Check if the command is empty (i.e., just a newline or nothing)
   if (command[0] == '\0' || command[0] == '\n')
      return -1;
   char cmdCpy[PATH_MAX_];
   strcpy(cmdCpy, command);
   char *inputToken = strtok(cmdCpy, DELIMS);
   char *terms[MAX_CMDS];
   int index = 0;

   while (inputToken != NULL){
      terms[index++] = inputToken;
      inputToken = strtok(NULL, DELIMS);
   }
   terms[index] = NULL;
   if(strcmp(terms[0], "hop") == 0){
      hopHandler(terms, curWorkingDir, index, prevDir, startDirectory);
   }
   else if(strcmp(terms[0], "reveal") == 0){
      revealMainHandler(terms, index, startDirectory, prevDir, curWorkingDir);
   }
   else if(index >=2 && strcmp(terms[0], "log") == 0){
      if(strcmp(terms[1], "purge") == 0){
         return logPurgeHandler(homePath);
      }
      else if(strcmp(terms[1], "execute") == 0){
         char *commandList[MAX_CMDS];
         return logExecuteHandler(atoi(terms[2]), commandList, curWorkingDir, prevDir, homePath, startDirectory);
      }
      else
         printf("\033[1;31mError: Command '%s' is not a valid command.\033[0m\n", command);
   }
   else if(strcmp(terms[0], "log") == 0){
      return printTotalHist(startDirectory);
   }
   else if(strcmp(terms[0], "proclore") == 0){
      pid_t pid;
      if(index >= 2){
         pid = atoi(terms[1]);
      }
      else{
         pid = getpid();
      }
      procloreHandler(pid);
   }
   // ...
   else if(strcmp(terms[0], "seek") == 0){
      seekHandler(terms, curWorkingDir, index, prevDir, startDirectory);
   }
   else {
      clock_t startTime;
      pid_t pid = fork();
      if(pid < 0) {
         perror("fork failed");
         return -1;
      }
      if(pid == 0) { // Child process
         // Execute the command using execvp
         if(execvp(terms[0], terms) == -1) {
            // perror("execvp failed");
            exit(EXIT_FAILURE); // Ensure child process exits if exec fails
         }
      } 
      else { // Parent process
         int status;
         struct timeval startTime, endTime;
         gettimeofday(&startTime, NULL);
         waitpid(pid, &status, 0); // Wait for the child process to finish
         gettimeofday(&endTime, NULL);
         double totalTime = (endTime.tv_sec - startTime.tv_sec) +
                            (endTime.tv_usec - startTime.tv_usec) / 1000000.0;
         if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            printf("\033[1;31mError: Command '%s' is not a valid command.\033[0m\n", terms[0]);
            return 0;
         }
         if (totalTime > 2.0){
            // Check if process took more than 2 seconds
            // printf("\033[1;33mProcess '%s' took %.3f seconds to complete.\033[0m\n", terms[0], totalTime);
            strcpy(processName, terms[0]);
            processTime = totalTime;
         }
      }
   }
}

