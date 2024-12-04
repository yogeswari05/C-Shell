#include "allFunctions.h"
#include <libgen.h>

//  to check if a file is executable based on the permissions it has
int isExecutable(const struct stat *file_stat){
   return (file_stat->st_mode & S_IXUSR) ||
          (file_stat->st_mode & S_IXGRP) ||
          (file_stat->st_mode & S_IXOTH);
}

// ls -l
void printDetailedInfo(const char *name, const struct stat *file_stat) {
   // File type
   char file_type = '-';
   if (S_ISDIR(file_stat->st_mode)) file_type = 'd';
   else if (S_ISLNK(file_stat->st_mode)) file_type = 'l';
   else if (S_ISCHR(file_stat->st_mode)) file_type = 'c';
   else if (S_ISBLK(file_stat->st_mode)) file_type = 'b';
   else if (S_ISFIFO(file_stat->st_mode)) file_type = 'p';
   else if (S_ISSOCK(file_stat->st_mode)) file_type = 's';

   // Permissions
   char permissions[10];
   snprintf(permissions, sizeof(permissions), "%c%c%c%c%c%c%c%c%c",
      (file_stat->st_mode & S_IRUSR) ? 'r' : '-',
      (file_stat->st_mode & S_IWUSR) ? 'w' : '-',
      (file_stat->st_mode & S_IXUSR) ? 'x' : '-',
      (file_stat->st_mode & S_IRGRP) ? 'r' : '-',
      (file_stat->st_mode & S_IWGRP) ? 'w' : '-',
      (file_stat->st_mode & S_IXGRP) ? 'x' : '-',
      (file_stat->st_mode & S_IROTH) ? 'r' : '-',
      (file_stat->st_mode & S_IWOTH) ? 'w' : '-',
      (file_stat->st_mode & S_IXOTH) ? 'x' : '-'
   );

   // Number of links
   int links = file_stat->st_nlink;
   // User and group name
   struct passwd *pw = getpwuid(file_stat->st_uid);
   struct group *gr = getgrgid(file_stat->st_gid);
   // File size
   long size = file_stat->st_size;
   // Last modified time
   char time_str[20];
   struct tm *time_info = localtime(&file_stat->st_mtime);
   strftime(time_str, sizeof(time_str), "%b %d %H:%M", time_info);
   // Print the detailed info
   printf("%c%s %d %s %s %ld %s ", file_type, permissions, links, pw ? pw->pw_name : "unknown", gr ? gr->gr_name : "unknown", size, time_str);
}

// Function to print file with color coding
void printWithColors(const char *name, const struct stat *file_stat) {
   if (S_ISDIR(file_stat->st_mode)) {
      printf("\033[1;34m%s\033[0m\n", name); // blue
   } 
   else if (isExecutable(file_stat)) {
      printf("\033[1;32m%s\033[0m\n", name); // green
   }
   else {
      printf("\033[0;37m%s\033[0m\n", name); // white
   }
}
// comparison function for lexicographical sorting
int customAlphasort(const struct dirent **a, const struct dirent **b){
   return strcasecmp((*a)->d_name, (*b)->d_name);
}
// Function to list files in a directory with the given flags
int getDirectoriesList(const char *path, int hyp_a, int hyp_l) {
   struct dirent **dirList;
   int n = scandir(path, &dirList, NULL, customAlphasort); // to read contents of a directory
   if (n < 0) {
      perror("scandir");
      return 0;
   }
   int count = 0;
   for (int i = 0; i < n; i++) {
      if (!hyp_a && dirList[i]->d_name[0] == '.') {
         // Files or directories starting with a dot (.) are considered hidden.
         continue;
      }
      count++;
   }
   // print the total number of files once if the -l flag is set
   if (hyp_l){
      printf("Total files: %d\n", count);
   }
   for (int i = 0; i < n; i++){
      if (!hyp_a && dirList[i]->d_name[0] == '.'){
         // skip hidden files
         continue;
      }
      char fullPath[1024];
      snprintf(fullPath, sizeof(fullPath), "%s/%s", path, dirList[i]->d_name);
      struct stat file_stat;
      if (stat(fullPath, &file_stat) == -1) {
         perror("stat");
         free(dirList[i]);
         continue;
      }
      if (hyp_l) {
         printDetailedInfo(dirList[i]->d_name, &file_stat);
      }
      printWithColors(dirList[i]->d_name, &file_stat);
      free(dirList[i]);
   }
   free(dirList);
}

// function to parse the flags and path from the input
int revealMainHandler(char **terms, int index, char startDirectory[PATH_MAX_], char prevDir[PATH_MAX_], char curWorkingDir[PATH_MAX_]) {
   int hyp_a = 0; // -a ??
   int hyp_l = 0; // -l
   int a_l_flag = 0;
   // parse the tokens to handle flags and path
   for (int i = 1; i < index; i++) { // Start from 1 to skip the command name
      if (terms[i][0] == '-') {
         for (int j = 1; terms[i][j] != '\0'; j++) {
            if (terms[i][j] == 'a') {
               hyp_a = 1; // we just need 1 a or 1 l
               a_l_flag = 1;
            }
            if (terms[i][j] == 'l'){
               hyp_l = 1;
               a_l_flag = 1;
            }
         }
      } 
   }
   char path[PATH_MAX_];
   if(index == 2 && a_l_flag){
      strcpy(path, ".");
   }
   else if(index == 2 && !a_l_flag){
      if(strcmp(terms[index-1], "..") == 0){
         // printf("..Case!!\n");
         char temp[PATH_MAX_];
         strcpy(temp, startDirectory);
         strcat(temp, "/");
         if (strcmp(curWorkingDir, "~") != 0){
            strcat(temp, curWorkingDir);
         }
         char *parentDir = dirname(temp); // Get the parent directory
         // printf("ParentDir: %s\n", parentDir);
         strcpy(path, temp);
      }
      else{
         char temp[PATH_MAX_];
         strcpy(temp, startDirectory);
         if (strcmp(curWorkingDir, "~") != 0)
            strcat(temp, curWorkingDir);
         strcpy(path, temp);
      }
   }
   else{
      if(strcmp(terms[index-1], "~") == 0){
         strcpy(path, startDirectory);
      }
      else if(strcmp(terms[index-1], "-") == 0){
         if(flag_prevDir__ == 0){
            printf("\033[1;31mError: Previous directory not yet set.\033[0m\n");
            return 0;
         }
         else{
            // printf("- case!!\n");
            char temp[PATH_MAX_];
            strcpy(temp, startDirectory);
            if(strcmp(prevDir, "~") != 0)
            strcat(temp, prevDir);
            strcpy(path, temp);
         }
      }
      else if(terms[index-1][0] == '~'){
         // printf("~/dirPath case\n");
         // to handle cases for path: ~/new/new1
         // printf("startDir: %s\n", startDirectory);
         char temp[PATH_MAX_];
         strcpy(temp, startDirectory);
         strcat(temp, terms[index - 1] + 1); // +1 tp ignore ~ in ~/path
         strcpy(path, temp);
      }
      else if(strcmp(terms[index-1], "..") == 0){
         // printf("..Case!!\n");
         char temp[PATH_MAX_];
         strcpy(temp, startDirectory);
         strcat(temp, "/");
         if (strcmp(curWorkingDir, "~") != 0){
            strcat(temp, curWorkingDir);
         }
         char *parentDir = dirname(temp); // Get the parent directory
         // printf("ParentDir: %s\n", parentDir);
         strcpy(path, temp);
      }
      else{
         char temp[PATH_MAX_];
         strcpy(temp, startDirectory);
         if (strcmp(curWorkingDir, "~") != 0)
            strcat(temp, curWorkingDir);
         strcpy(path, temp);
      }
   }
   // printf("path: %s\n", path);
   // store the current directory in prev_dir before changing if `-` flag is used later
   char curDir[1024];
   getcwd(curDir, sizeof(curDir));
   getDirectoriesList(path, hyp_a, hyp_l);
   strcpy(curDir, path);
}