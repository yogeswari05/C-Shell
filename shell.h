#ifndef ALLFUNCTIONS_H
#define ALLFUNCTIONS_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <grp.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>

#define DELIMS " \t\n"

#define PATH_MAX_ 4096
#define INP_MAX 4096
#define MAX_CMDS 4096
#define MAX 100
#define MAX_ARGS 100
#define HIST_MAX_SIZE 15
#define MAX_PIPELINE_COMMANDS 32
#define MAX_JOBS 100

// functions declarations
// s-1
extern char processName[MAX];
extern double processTime;

int getCurrWorkingDir(char curWorkingDir[PATH_MAX_], size_t size);
void getHostName_(char hostname[PATH_MAX_], int size);
void getUserName(char username[PATH_MAX_], int size);
int changeDirectory(char path[PATH_MAX_], char curWorkingDir[PATH_MAX_], char startDirectory[PATH_MAX_], char prevDir[PATH_MAX_]);

// s-2
void tokenizeExecuteCommands(char command[INP_MAX], char *commandList[MAX_CMDS], char currDir[PATH_MAX_], char prevDir[PATH_MAX_], char homePath[PATH_MAX_], char startDirectory[PATH_MAX_]);
int commandHandler(char command[PATH_MAX_], char curWorkingDir[PATH_MAX_], char prevDir[PATH_MAX_], char homePath[PATH_MAX_], char startDirectory[PATH_MAX_]);
void trimWhitespace(char *str);
// s-3
int hopHandler(char **words, char curWorkingDir[PATH_MAX_], int wcount, char prevDir[PATH_MAX_], char startDirectory[PATH_MAX_]);
extern int flag_prevDir__;
// s-4
int revealMainHandler(char **terms, int index, char startDirectory[PATH_MAX_], char prevDir[PATH_MAX_], char curWorkingDir[PATH_MAX_]);

// s-5
// Declaration of the global array
extern char histStore[HIST_MAX_SIZE][MAX_CMDS];
extern int countOfHist;
extern int curIndHist;
int addCommandToHist(char command[INP_MAX], char homePath[PATH_MAX_]);
int printTotalHist(char startDirectory[PATH_MAX_]);
void saveHistory(char homePath[PATH_MAX_]);
void removeUncessecarySpaceTabs(char *str);
int logPurgeHandler(char homeDir[PATH_MAX_]);
int logExecuteHandler(int n, char *commandList[4096], char currDir[4096], char prevDir[4096], char homePath[4096], char startDirectory[4096]);
void loadHistory(char startDirectory[PATH_MAX_]);

// s-6
typedef enum
{
   JOB_PROCESS_RUNNING,
   JOB_PROCESS_STOPPED,
   JOB_PROCESS_DONE
} JobProcessState;

typedef struct
{
   pid_t pid;
   JobProcessState state;
} JobProcess;

typedef struct
{
   int id;
   pid_t pgid;
   char command[INP_MAX];
   JobProcess processes[MAX_PIPELINE_COMMANDS];
   int processCount;
   int background;
   int notified;
   int used;
} ShellJob;

extern ShellJob shellJobs[MAX_JOBS];
extern int shellInteractive;
extern pid_t shellPgid;
extern volatile sig_atomic_t childEvent;
extern volatile sig_atomic_t interruptEvent;
extern int shellExitRequested;

void handle_sigchld(int sig);
void setupSignalHandling();
int registerShellJob(pid_t pgid, pid_t *pids, int processCount, const char *command, int background);
void waitForShellJob(int jobIndex);
void pollShellEvents(void);
int handleJobBuiltin(char **terms, int count, int inParent);
void shutdownShell(void);

// s-7
void procloreHandler(pid_t pid);

// s-8
int seekHandler(char **words, char curWorkingDir[PATH_MAX_], int wcount, char prevDir[PATH_MAX_], char startDirectory[PATH_MAX_]);

#endif // ALLFUNCTIONS_H
