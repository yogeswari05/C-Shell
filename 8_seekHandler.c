#include "allFunctions.h"
#define MAX_FILE_SIZE 1024

// Function to print the path with color coding
void pathPrinter(char *startDir, char *path, int is_dir, char *curWorkingDir, char *pathWord) {
	char *finalPath = path;
	// finalPath =
	// Check if 'str' starts with 'prefix'
	if (strlen(finalPath) >= strlen(curWorkingDir) && strncmp(finalPath, curWorkingDir, strlen(curWorkingDir)) == 0){
		// Move the start of 'str' past the 'prefix'
		memmove(finalPath, finalPath + strlen(curWorkingDir), strlen(finalPath) - strlen(curWorkingDir) + 1);
	}
	
	if(pathWord[0] == '.'){
		memmove(pathWord, pathWord + 1, strlen(pathWord));
		pathWord[strlen(pathWord)] = '\0';
	}
	if (strlen(finalPath) >= strlen(pathWord) && strncmp(finalPath, pathWord, strlen(pathWord)) == 0){
		// Move the start of 'str' past the 'prefix'
		memmove(finalPath, finalPath + strlen(pathWord), strlen(finalPath) - strlen(pathWord) + 1);
	}
	// printf("FIINAL: %s\n", finalPath);
	// printf("Pahtword: %s\n", pathWord);
	if (is_dir){
		printf("\033[34m.%s\033[0m\n", finalPath); // Print directory in blue
	}
	else{
		printf("\033[32m.%s\033[0m\n", finalPath); // Print file in green
	}
}

// Recursive function to search the directory tree
void search_directory(char *dir_path, char *target, int flag_d, int flag_f, int flag_e, int *countDirs, int *countFIles, char onlyDirPath[PATH_MAX_], char oneFilePath[PATH_MAX_], char *startDir, int *isMatch, char *curWorkingDir, char pathWord[PATH_MAX_]) {
	DIR *dir = opendir(dir_path);
	if (!dir) return;
	struct dirent *entry;
	char path[PATH_MAX_];
	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
		snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);
		struct stat statbuf;
		stat(path, &statbuf);
		// Calculate the relative path from the starting directory
		char relativePath[PATH_MAX_];
		snprintf(relativePath, sizeof(relativePath), "%s/%s", dir_path, entry->d_name);
		if (strncmp(relativePath, startDir, strlen(startDir)) == 0){
			snprintf(relativePath, sizeof(relativePath), "%s", relativePath + strlen(startDir));
		}
		if (S_ISDIR(statbuf.st_mode)) {
			// If it's a directory and -f flag is not set
			if (!flag_f && strncmp(entry->d_name, target, strlen(target)) == 0) {
				// printf("relative: %s, dirPath: %s\n", relativePath, startDir);
				pathPrinter(dir_path, relativePath, 1, curWorkingDir, pathWord);
				strcpy(onlyDirPath, path);
				(*countDirs)++;
				(*isMatch) = 1;
			}
			search_directory(path, target, flag_d, flag_f, flag_e, countDirs, countFIles, onlyDirPath, oneFilePath, startDir, isMatch, curWorkingDir, pathWord);  // Recursive call
		} 
		else {
			// If it's a file and -d flag is not set
			if (!flag_d && strncmp(entry->d_name, target, strlen(target)) == 0) {
				// printf("relative: %s, dirPath: %s\n", relativePath, startDir);
				pathPrinter(dir_path, relativePath, 0, curWorkingDir, pathWord);
				strcpy(oneFilePath, path);
				(*countFIles)++;
				(*isMatch) = 1;
			}
		}
	}
	closedir(dir);
}

// Entry function
void seek(char *target, char *dir_path, int flag_d, int flag_f, int flag_e, char prevDir[PATH_MAX_], char curWorkingDir[PATH_MAX_], char startDir[PATH_MAX_], char pathWord[PATH_MAX_]) {
	char onlyDirPath[PATH_MAX_], oneFilePath[PATH_MAX_];
	int countDirs = 0, countFIles = 0, isMatch = 0;
	search_directory(dir_path, target, flag_d, flag_f, flag_e, &countDirs, &countFIles, onlyDirPath, oneFilePath, startDir, &isMatch, curWorkingDir, pathWord);
	if(isMatch == 0){
		printf("No match found!\n");
	}
	else if (flag_e && countDirs + countFIles == 1) {
		if(countDirs == 1){
			DIR *dir = opendir(onlyDirPath);
			if (dir != NULL) {
				// If a single directory is found and flag_e is set, change to that directory
				strcpy(prevDir, curWorkingDir);
				changeDirectory(onlyDirPath, curWorkingDir, startDir, prevDir);
			}
		}
		else {
			// If a single file is found and flag_e is set, print its contetnts
			int fd = open(oneFilePath, O_RDONLY);
			if (fd < 0){
				if (errno == EACCES){
					fprintf(stderr, "Missing permissions for task!\n");
				}
				else{
					perror("Error opening file");
				}
				perror("Error opening file");
				return;
			}
			char Contents[MAX_FILE_SIZE];
			ssize_t bytes_read;
			while ((bytes_read = read(fd, Contents, sizeof(Contents))) > 0){
				// Write the read buffer to stdout
				if (write(STDOUT_FILENO, Contents, bytes_read) != bytes_read){
					perror("Error writing to stdout");
					close(fd);
					return;
				}
			}
			if (bytes_read < 0){
				perror("Error reading file");
			}
			close(fd);
		}
	}
}

int seekHandler(char *terms[PATH_MAX_], char curWorkingDir[PATH_MAX_], int wcount, char prevDir[PATH_MAX_], char startDirectory[PATH_MAX_]){
	int flag_d = 0, flag_f = 0, flag_e = 0;
	char target[PATH_MAX_];
	char path[PATH_MAX_];
	int flagCount = 0;
	int target_set = 0, path_set = 0;
	for (int i = 1; i < wcount; i++) {
		if (strcmp(terms[i], "-d") == 0) {
			flag_d = 1;
			flagCount++;
		} 
		else if (strcmp(terms[i], "-f") == 0) {
			flag_f = 1;
			flagCount++;
		} 
		else if (strcmp(terms[i], "-e") == 0) {
			flag_e = 1;
			flagCount++;
		} 
	}
	// terms[0] -> seek, flagCount flags, target, path (path is optional, target is always provided)
	if (flagCount + 2 < wcount){ // 2 -> seek, target ( seek -e new ./folders)
		// arguments for both target and path
		// printf("Both target, path!!\n");
		strcpy(target, terms[wcount - 2]);
		strcpy(path, terms[wcount - 1]);
	}
	else if (flagCount + 2 == wcount){
		// printf("No path !!\n");
		// ( seek -e new => no path)
		strcpy(target, terms[wcount - 1]);
		strcpy(path, startDirectory); // Use the current working directory
		if(strcmp(curWorkingDir, "~") != 0){
			strcat(path, curWorkingDir);
		}
	}
	printf("INITIAL seekPath: %s, target: %s\n", path, target);
	char pathWord[PATH_MAX_];
	strcpy(pathWord, path);
	// finding path 🔍
	if(strcmp(path, "-") == 0){
		// printf("- case\n");
		if (flag_prevDir__ == 0){
			printf("\033[1;31mError: Previous directory not yet set.\033[0m\n");
			return 0;
		}
		else{
			char temp[PATH_MAX_];
			strcpy(temp, startDirectory);
			// printf("prevDir: %s\n", prevDir);
			if(strcmp(prevDir, "~") != 0){
				strcat(temp, prevDir);
			}
			strcpy(path, temp);
		}
	}
	else if (strcmp(path, "~") == 0) {
		strcpy(path, startDirectory);
	} 
	else if (path[0] == '~' && strlen(path) > 1) {
      char temp[PATH_MAX_];
      strcpy(temp, startDirectory);
      strcat(temp, path + 1);
      strcpy(path, temp);
   }
	else if(strcmp(path, "..") == 0){
		char *lastSlash = strrchr(curWorkingDir, '/');
		if (lastSlash != NULL && lastSlash != curWorkingDir){
			*lastSlash = '\0'; // Remove the last part of the path
		}
		else{
			// If no slash is found or it is the root directory
			strcpy(curWorkingDir, "/");
		}
	}
	// Handle path special cases
	else if (strcmp(path, ".") == 0) {
		strcpy(path, startDirectory); // Use the current working directory
		if(strcmp(curWorkingDir, "~") != 0){
			strcat(path, curWorkingDir);
		}
	} 
	else if(path[0] == '.'){
		// printf("case\n");
		char temp[PATH_MAX_];
		strcpy(temp, startDirectory);
		if(strcmp(curWorkingDir, "~") != 0){
			strcat(temp, curWorkingDir);
		}
		strcat(temp, path + 1);
		strcpy(path, temp);
	}
	else if (path[0] == '\0') {
		// If no path is provided, use the startDirectory + curWorkingDir
		strcpy(path, startDirectory);
		if (strcmp(curWorkingDir, "~") != 0) {
			strcat(path, curWorkingDir);
		}
	}
	else if (strcmp(path, "") == 0) {
		strcpy(path, startDirectory);
		if (strcmp(curWorkingDir, "~") != 0) {
			strcat(path, curWorkingDir);
		}
	}
	// printf("Path: %s\n", path);
	// printf("seekPath: %s\ntarget: %s\n", path, target);
	if(flag_d && flag_f){
		printf("\n");
		printf("\033[1;31mError: Invalid flags!\033[0m\n");
	}
	else {
		seek(target, path, flag_d, flag_f, flag_e, prevDir, curWorkingDir, startDirectory, pathWord);
	}
	return 0;
}