#include "allFunctions.h"

void procloreHandler(pid_t pid) {
   char path[256], buffer[256];
   FILE *fp;
    // Check if the process is running by trying to open /proc/[pid]
   snprintf(path, sizeof(path), "/proc/%d", pid);
   if (access(path, F_OK) != 0) {
      printf("\033[1;31mError: No such process with pid %d\n.\033[0m\n", pid);
      return;
   }
   // Print PID
   printf("pid : %d\n", pid);
   // Process Status
   snprintf(path, sizeof(path), "/proc/%d/stat", pid);
   fp = fopen(path, "r");
   if (fp) {
      int pgrp, tpgid;
      char state;
      fscanf(fp, "%*d %*s %c %*d %d %d", &state, &pgrp, &tpgid);
      fclose(fp);
      char *status;
      switch (state) {
         case 'R': status = (pid == tpgid) ? "R+" : "R"; break;
         case 'S': status = (pid == tpgid) ? "S+" : "S"; break;
         case 'Z': status = "Z"; break;
         default: status = "Unknown"; break;
      }
      printf("process status : %s\n", status);
      // Process Group
      printf("Process Group : %d\n", pgrp);
   } 
   else {
      perror("Failed to read process status");
      return;
   }
   // Virtual Memory
   snprintf(path, sizeof(path), "/proc/%d/statm", pid);
   fp = fopen(path, "r");
   if (fp) {
      unsigned long vm_size;
      fscanf(fp, "%lu", &vm_size);
      fclose(fp);
      printf("Virtual memory : %lu\n", vm_size);
   } 
   else {
      perror("Failed to read virtual memory");
      return;
   }
   // Executable Path
   snprintf(path, sizeof(path), "/proc/%d/exe", pid);
   ssize_t len = readlink(path, buffer, sizeof(buffer) - 1);
   if (len != -1) {
      buffer[len] = '\0';
      printf("executable Path : %s\n", buffer);
   } 
   else {
      perror("Failed to read executable path");
      return;
   }
}