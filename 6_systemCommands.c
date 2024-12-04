#include "allFunctions.h"
#include <asm-generic/signal-defs.h>
BackgroundProcess bgProcesses[MAX_BG_PROCESSES];
int bgProcessCount = 0;

void setupSignalHandling() {
   struct sigaction sa;
   sa.sa_handler = handle_sigchld;
   sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
   sigemptyset(&sa.sa_mask);
   sigaction(SIGCHLD, &sa, NULL);
}

void addBackgroundProcess(pid_t pid, char *name) {
   if (bgProcessCount < MAX_BG_PROCESSES) {
      bgProcesses[bgProcessCount].pid = pid;
      bgProcesses[bgProcessCount].name = strdup(name);
      bgProcessCount++;
   }
}

void removeBackgroundProcess(pid_t pid) {
   for (int i = 0; i < bgProcessCount; i++) {
      if (bgProcesses[i].pid == pid) {
         int status;
         if(waitpid(pid, &status, WNOHANG) > 0){
            if (WIFSIGNALED(status)) {
               int signal_number = WTERMSIG(status);
               printf("\n%s exited abnormally due to signal %d (%d)\n", bgProcesses[i].name, signal_number, pid);
            } 
            else {
               printf("\n%s exited abnormally (%d)\n", bgProcesses[i].name, pid);
            }
         }
         else {
            printf("\n%s exited normally (%d)\n", bgProcesses[i].name, pid);
         }
         fflush(stdout);            // Ensure the message is displayed immediately
         free(bgProcesses[i].name); // Free allocated memory for the process name
         for (int j = i; j < bgProcessCount - 1; j++){
            bgProcesses[j] = bgProcesses[j + 1];
         }
         bgProcessCount--;
         break;
      }
   }
}

void handle_sigchld(int sig) {
   pid_t pid;
   while ((pid = waitpid(-1, NULL, WNOHANG)) > 0){
      removeBackgroundProcess(pid);
   }
}

void runInBackground(char *command) {
   // Check if the last argument is '&' indicating background execution
   // Split the command into arguments
   char commandCopy[PATH_MAX_];
   strcpy(commandCopy, command); // Create a copy of the command string
   char *terms[MAX_CMDS];
   char *token = strtok(commandCopy, " ");
   int index = 0;
   while (token != NULL) {
      terms[index++] = token;
      token = strtok(NULL, " ");
   }
   terms[index] = NULL;  // Null-terminate the argument list
   pid_t pid = fork();
   if (pid == 0) {
      // Child process
      if (execvp(terms[0], terms) == -1) {
         perror("Error executing command");
      }
      exit(EXIT_FAILURE);  // Exit if exec fails
   } 
   else if (pid > 0) {
      // Parent process
      printf("%d\n", pid);
      addBackgroundProcess(pid, terms[0]); // Add the process to the background list
   } 
   else {
      perror("Failed to fork");
   }
} 
