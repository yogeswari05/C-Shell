#include "allFunctions.h"

void loadHistory(char startDirectory[PATH_MAX_]) {
   char fullPath[PATH_MAX_];
   strcpy(fullPath, startDirectory);
   strcat(fullPath, "/");
   strcat(fullPath, "history.txt");
   int file = open(fullPath, O_RDONLY | O_CREAT, 0644);
   if (file == -1) {
      perror("Failed to open file for reading");
      return;
   }
   char buffer[MAX_CMDS];
   ssize_t bytesRead;
   int index = 0;

   while ((bytesRead = read(file, buffer, sizeof(buffer) - 1)) > 0){
      buffer[bytesRead] = '\0'; // Null-terminate the buffer
      char *lineStart = buffer;
      char *lineEnd;
      while ((lineEnd = strchr(lineStart, '\n')) != NULL){
         *lineEnd = '\0'; // Replace newline with null-terminator
         // If the array is full, do not add more lines
         if (index >= HIST_MAX_SIZE){
            break;
         }
         // Store the line in histStore
         strncpy(histStore[index], lineStart, MAX_CMDS - 1);
         histStore[index][MAX_CMDS - 1] = '\0'; // Ensure null-termination
         index++;
         lineStart = lineEnd + 1; // Move to the start of the next line
      }
      // Handle the remaining content if it's not empty and not a new line
      if (index < HIST_MAX_SIZE && *lineStart != '\0'){
         strncpy(histStore[index], lineStart, MAX_CMDS - 1);
         histStore[index][MAX_CMDS - 1] = '\0';
         index++;
      }
   }
   countOfHist = index;
   // printf("After loading...\n");
   // printf("CountHist: %d\n", countOfHist);
   // for (int i = 0; i < countOfHist; i++){
   //    printf("%d %s\n", i, histStore[i]);
   // }
   close(file);
}

void saveHistory(char startDirectory[PATH_MAX_]) {
   char fullPath[PATH_MAX_];
   strcpy(fullPath, startDirectory);
   strcat(fullPath, "/");
   strcat(fullPath, "history.txt");
   int file = open(fullPath, O_TRUNC | O_WRONLY);
   if (file == -1){
      perror("Failed to open file");
      return;
   }
   // Write the entire history to the file
   for (int i = 0; i < countOfHist; i++){
      write(file, histStore[i], strlen(histStore[i]));
      write(file, "\n", 1); // Write the newline character
   }
   close(file);
}

int addCommandToHist(char command[INP_MAX], char startDirectory[PATH_MAX_]){
   char temp[INP_MAX];
   strcpy(temp, command);
   removeUncessecarySpaceTabs(temp);
   if(strcmp(temp, "log") == 0) {
      return -1;
   } // need to handle log purge, log execute i
   // Check for duplicate commands
   if (countOfHist > 0 && strcmp(histStore[countOfHist - 1], temp) == 0){
      return -1;
   }
   // Shift commands if necessary
   if (countOfHist >= HIST_MAX_SIZE){
      for (int i = 1; i < HIST_MAX_SIZE; i++){
         strncpy(histStore[i - 1], histStore[i], MAX_CMDS - 1);
         histStore[i - 1][MAX_CMDS - 1] = '\0'; // Ensure null-termination
      }
      countOfHist = HIST_MAX_SIZE - 1;
   }
   // Add new command to history
   strncpy(histStore[countOfHist], temp, MAX_CMDS - 1);
   histStore[countOfHist][MAX_CMDS - 1] = '\0'; // Ensure null-termination
   countOfHist++;
   saveHistory(startDirectory);
   return 0;
}

int printTotalHist(char startDirectory[PATH_MAX_]){
   for (int j = 0; j < countOfHist; j++) {
      printf("%d %s\n", j+1, histStore[j]);
   }
   return -1;
}

void removeUncessecarySpaceTabs(char *str) {
   char *end;
   // Trim leading space
   while(isspace((unsigned char)*str)) str++;
   if(*str == 0) {  // All spaces?
      *str = '\0';
      return;
   }
   // Trim trailing space
   end = str + strlen(str) - 1;
   while(end > str && isspace((unsigned char)*end)) end--;
   // Write new null terminator character
   *(end + 1) = '\0';
}

int logPurgeHandler(char homeDir[PATH_MAX_]){
   char filePath[PATH_MAX_];
   char command[300]; // Buffer to hold the command
   strcpy(filePath, homeDir);
   strcat(filePath, "/");
   strcat(filePath, "history.txt");
   int file = open(filePath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
   close(file);
   for (int i = 0; i < HIST_MAX_SIZE; i++){
      histStore[i][0] = '\0';
   }
   countOfHist = 0;
   return -1;
}

int logExecuteHandler(int n, char *commandList[4096], char currDir[4096], char prevDir[4096], char homePath[4096], char startDirectory[4096]){
   if (n <= 0 || n > countOfHist) {
      printf("\033[1;31mError: Invalid history index!\033[0m\n");
      return -1;
   }
   int ind = countOfHist - n;
   printf("%d: %s\n", ind, histStore[ind]);
   tokenizeExecuteCommands(histStore[ind], commandList, currDir, prevDir, homePath, startDirectory);
   return -1;
}