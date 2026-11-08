#include "shell.h"

typedef enum
{
   TOKEN_WORD,
   TOKEN_SEMICOLON,
   TOKEN_BACKGROUND,
   TOKEN_PIPE,
   TOKEN_INPUT,
   TOKEN_OUTPUT,
   TOKEN_APPEND
} TokenType;

typedef struct
{
   TokenType type;
   char *text;
} ShellToken;

typedef struct
{
   char *arguments[MAX_ARGS + 1];
   int argumentCount;
   char *inputPath;
   char *outputPath;
   int appendOutput;
} PipelineCommand;

static int is_operator(char character)
{
   return character == ';' || character == '&' || character == '|' ||
          character == '<' || character == '>';
}

void trimWhitespace(char *str)
{
   char *start = str;
   while (*start && isspace((unsigned char)*start))
      start++;
   if (*start == '\0')
   {
      str[0] = '\0';
      return;
   }
   char *end = start + strlen(start) - 1;
   while (end > start && isspace((unsigned char)*end))
      end--;
   *(end + 1) = '\0';
   if (start > str)
      memmove(str, start, (size_t)(end - start + 2));
}

static int add_token(ShellToken *tokens, int *tokenCount, TokenType type, char *text)
{
   if (*tokenCount >= INP_MAX)
      return -1;
   tokens[*tokenCount].type = type;
   tokens[*tokenCount].text = text;
   (*tokenCount)++;
   return 0;
}

static int lex_command(const char *input, ShellToken *tokens, int *tokenCount,
                       char *storage, size_t storageSize)
{
   size_t inputIndex = 0;
   size_t storageIndex = 0;
   *tokenCount = 0;

   while (input[inputIndex] != '\0')
   {
      if (isspace((unsigned char)input[inputIndex]))
      {
         inputIndex++;
         continue;
      }

      char character = input[inputIndex];
      TokenType operatorType;
      int isOperator = 1;
      switch (character)
      {
      case ';':
         operatorType = TOKEN_SEMICOLON;
         break;
      case '&':
         operatorType = TOKEN_BACKGROUND;
         break;
      case '|':
         operatorType = TOKEN_PIPE;
         break;
      case '<':
         operatorType = TOKEN_INPUT;
         break;
      case '>':
         if (input[inputIndex + 1] == '>')
         {
            operatorType = TOKEN_APPEND;
            inputIndex++;
         }
         else
         {
            operatorType = TOKEN_OUTPUT;
         }
         break;
      default:
         isOperator = 0;
         operatorType = TOKEN_WORD;
         break;
      }
      if (isOperator)
      {
         if (add_token(tokens, tokenCount, operatorType, NULL) != 0)
            return -1;
         inputIndex++;
         continue;
      }

      size_t wordStart = storageIndex;
      int wordStarted = 0;
      char quote = '\0';
      while (input[inputIndex] != '\0')
      {
         character = input[inputIndex];
         if (quote != '\0')
         {
            if (character == quote)
            {
               quote = '\0';
               wordStarted = 1;
               inputIndex++;
               continue;
            }
            if (character == '\\' && quote == '"' && input[inputIndex + 1] != '\0')
            {
               inputIndex++;
               character = input[inputIndex];
            }
         }
         else
         {
            if (isspace((unsigned char)character) || is_operator(character))
               break;
            if (character == '\'' || character == '"')
            {
               quote = character;
               wordStarted = 1;
               inputIndex++;
               continue;
            }
            if (character == '\\')
            {
               if (input[inputIndex + 1] == '\0')
               {
                  fprintf(stderr, "Syntax error: trailing escape character\n");
                  return -1;
               }
               inputIndex++;
               character = input[inputIndex];
            }
         }
         if (storageIndex + 1 >= storageSize)
            return -1;
         storage[storageIndex++] = character;
         wordStarted = 1;
         inputIndex++;
      }
      if (quote != '\0')
      {
         fprintf(stderr, "Syntax error: unmatched quote\n");
         return -1;
      }
      if (wordStarted)
      {
         if (storageIndex >= storageSize)
            return -1;
         storage[storageIndex++] = '\0';
         if (add_token(tokens, tokenCount, TOKEN_WORD, &storage[wordStart]) != 0)
            return -1;
      }
   }
   return 0;
}

static int is_builtin(const char *name)
{
   return strcmp(name, "hop") == 0 || strcmp(name, "reveal") == 0 ||
          strcmp(name, "log") == 0 || strcmp(name, "proclore") == 0 ||
          strcmp(name, "seek") == 0 || strcmp(name, "jobs") == 0 ||
          strcmp(name, "fg") == 0 || strcmp(name, "bg") == 0 ||
          strcmp(name, "exit") == 0;
}

static int run_builtin(PipelineCommand *command, char *curWorkingDir, char *prevDir,
                       char *homePath, char *startDirectory, int inParent, int *status)
{
   char **terms = command->arguments;
   int count = command->argumentCount;
   *status = 0;

   if (strcmp(terms[0], "hop") == 0)
   {
      *status = hopHandler(terms, curWorkingDir, count, prevDir, startDirectory);
   }
   else if (strcmp(terms[0], "reveal") == 0)
   {
      *status = revealMainHandler(terms, count, startDirectory, prevDir, curWorkingDir);
   }
   else if (strcmp(terms[0], "log") == 0)
   {
      if (count >= 2 && strcmp(terms[1], "purge") == 0)
      {
         logPurgeHandler(homePath);
      }
      else if (count >= 2 && strcmp(terms[1], "execute") == 0)
      {
         if (count < 3)
         {
            fprintf(stderr, "Usage: log execute <index>\n");
            *status = 2;
         }
         else
         {
            char *commandList[MAX_CMDS];
            logExecuteHandler(atoi(terms[2]), commandList, curWorkingDir,
                              prevDir, homePath, startDirectory);
         }
      }
      else if (count == 1)
      {
         printTotalHist(startDirectory);
      }
      else
      {
         fprintf(stderr, "Usage: log [purge|execute <index>]\n");
         *status = 2;
      }
   }
   else if (strcmp(terms[0], "proclore") == 0)
   {
      procloreHandler(count >= 2 ? (pid_t)atoi(terms[1]) : getpid());
   }
   else if (strcmp(terms[0], "seek") == 0)
   {
      *status = seekHandler(terms, curWorkingDir, count, prevDir, startDirectory);
   }
   else if (strcmp(terms[0], "jobs") == 0 || strcmp(terms[0], "fg") == 0 ||
            strcmp(terms[0], "bg") == 0)
   {
      handleJobBuiltin(terms, count, inParent);
   }
   else if (strcmp(terms[0], "exit") == 0)
   {
      if (inParent)
         shellExitRequested = 1;
   }
   else
   {
      return 0;
   }
   return 1;
}

static int apply_redirections(PipelineCommand *command)
{
   if (command->inputPath != NULL)
   {
      int inputFd = open(command->inputPath, O_RDONLY);
      if (inputFd < 0)
      {
         perror(command->inputPath);
         return -1;
      }
      if (dup2(inputFd, STDIN_FILENO) < 0)
      {
         perror("dup2");
         close(inputFd);
         return -1;
      }
      close(inputFd);
   }
   if (command->outputPath != NULL)
   {
      int flags = O_WRONLY | O_CREAT | (command->appendOutput ? O_APPEND : O_TRUNC);
      int outputFd = open(command->outputPath, flags, 0666);
      if (outputFd < 0)
      {
         perror(command->outputPath);
         return -1;
      }
      if (dup2(outputFd, STDOUT_FILENO) < 0)
      {
         perror("dup2");
         close(outputFd);
         return -1;
      }
      close(outputFd);
   }
   return 0;
}

static int execute_parent_builtin(PipelineCommand *command, char *curWorkingDir,
                                  char *prevDir, char *homePath, char *startDirectory)
{
   int savedInput = -1;
   int savedOutput = -1;
   if (command->inputPath != NULL)
      savedInput = dup(STDIN_FILENO);
   if (command->outputPath != NULL)
      savedOutput = dup(STDOUT_FILENO);
   if ((command->inputPath != NULL && savedInput < 0) ||
       (command->outputPath != NULL && savedOutput < 0))
   {
      perror("dup");
      if (savedInput >= 0)
         close(savedInput);
      if (savedOutput >= 0)
         close(savedOutput);
      return -1;
   }

   fflush(NULL);
   if (apply_redirections(command) != 0)
   {
      if (savedInput >= 0)
      {
         dup2(savedInput, STDIN_FILENO);
         close(savedInput);
      }
      if (savedOutput >= 0)
      {
         dup2(savedOutput, STDOUT_FILENO);
         close(savedOutput);
      }
      return -1;
   }
   int status = 0;
   run_builtin(command, curWorkingDir, prevDir, homePath, startDirectory, 1, &status);
   fflush(NULL);
   if (savedInput >= 0)
   {
      dup2(savedInput, STDIN_FILENO);
      close(savedInput);
   }
   if (savedOutput >= 0)
   {
      dup2(savedOutput, STDOUT_FILENO);
      close(savedOutput);
   }
   return status;
}

static void reset_child_signals(void)
{
   struct sigaction action;
   memset(&action, 0, sizeof(action));
   action.sa_handler = SIG_DFL;
   sigemptyset(&action.sa_mask);
   sigaction(SIGINT, &action, NULL);
   sigaction(SIGQUIT, &action, NULL);
   sigaction(SIGTSTP, &action, NULL);
   sigaction(SIGTTIN, &action, NULL);
   sigaction(SIGTTOU, &action, NULL);
   sigaction(SIGCHLD, &action, NULL);
}

static int parse_pipeline(ShellToken *tokens, int start, int end,
                          PipelineCommand *commands, int *commandCount)
{
   memset(commands, 0, sizeof(PipelineCommand) * MAX_PIPELINE_COMMANDS);
   *commandCount = 1;
   for (int index = start; index < end; index++)
   {
      PipelineCommand *current = &commands[*commandCount - 1];
      TokenType type = tokens[index].type;
      if (type == TOKEN_WORD)
      {
         if (current->argumentCount >= MAX_ARGS)
         {
            fprintf(stderr, "Too many command arguments\n");
            return -1;
         }
         current->arguments[current->argumentCount++] = tokens[index].text;
         current->arguments[current->argumentCount] = NULL;
      }
      else if (type == TOKEN_PIPE)
      {
         if (current->argumentCount == 0 || *commandCount >= MAX_PIPELINE_COMMANDS)
         {
            fprintf(stderr, "Syntax error near '|': invalid pipeline\n");
            return -1;
         }
         (*commandCount)++;
      }
      else if (type == TOKEN_INPUT || type == TOKEN_OUTPUT || type == TOKEN_APPEND)
      {
         if (index + 1 >= end || tokens[index + 1].type != TOKEN_WORD)
         {
            fprintf(stderr, "Syntax error: expected a file after redirection\n");
            return -1;
         }
         if (type == TOKEN_INPUT)
            current->inputPath = tokens[++index].text;
         else
         {
            current->outputPath = tokens[++index].text;
            current->appendOutput = type == TOKEN_APPEND;
         }
      }
      else
      {
         fprintf(stderr, "Syntax error: unexpected operator\n");
         return -1;
      }
   }
   for (int index = 0; index < *commandCount; index++)
   {
      if (commands[index].argumentCount == 0)
      {
         fprintf(stderr, "Syntax error: empty command in pipeline\n");
         return -1;
      }
      commands[index].arguments[commands[index].argumentCount] = NULL;
   }
   return 0;
}

static int execute_pipeline(PipelineCommand *commands, int commandCount, int background,
                            const char *line, char *curWorkingDir, char *prevDir,
                            char *homePath, char *startDirectory)
{
   if (commandCount == 1 && !background && is_builtin(commands[0].arguments[0]))
   {
      return execute_parent_builtin(&commands[0], curWorkingDir, prevDir, homePath, startDirectory);
   }

   sigset_t childMask;
   sigset_t previousMask;
   sigemptyset(&childMask);
   sigaddset(&childMask, SIGCHLD);
   sigprocmask(SIG_BLOCK, &childMask, &previousMask);

   pid_t processIds[MAX_PIPELINE_COMMANDS];
   int processCount = 0;
   pid_t processGroup = 0;
   int previousRead = -1;
   int spawnFailed = 0;

   for (int index = 0; index < commandCount; index++)
   {
      int pipeDescriptors[2] = {-1, -1};
      if (index + 1 < commandCount && pipe(pipeDescriptors) < 0)
      {
         perror("pipe");
         spawnFailed = 1;
         break;
      }

      pid_t pid = fork();
      if (pid == 0)
      {
         pid_t childGroup = processGroup == 0 ? getpid() : processGroup;
         setpgid(0, childGroup);
         reset_child_signals();
         sigprocmask(SIG_SETMASK, &previousMask, NULL);
         if (previousRead >= 0 && dup2(previousRead, STDIN_FILENO) < 0)
            _exit(126);
         if (pipeDescriptors[1] >= 0 && dup2(pipeDescriptors[1], STDOUT_FILENO) < 0)
            _exit(126);
         if (previousRead >= 0)
            close(previousRead);
         if (pipeDescriptors[0] >= 0)
            close(pipeDescriptors[0]);
         if (pipeDescriptors[1] >= 0)
            close(pipeDescriptors[1]);
         if (apply_redirections(&commands[index]) != 0)
            _exit(126);

         int childStatus = 0;
         if (run_builtin(&commands[index], curWorkingDir, prevDir, homePath,
                         startDirectory, 0, &childStatus))
         {
            fflush(NULL);
            _exit(childStatus < 0 ? 1 : childStatus);
         }
         execvp(commands[index].arguments[0], commands[index].arguments);
         fprintf(stderr, "%s: %s\n", commands[index].arguments[0], strerror(errno));
         _exit(errno == ENOENT ? 127 : 126);
      }
      if (pid < 0)
      {
         perror("fork");
         if (pipeDescriptors[0] >= 0)
            close(pipeDescriptors[0]);
         if (pipeDescriptors[1] >= 0)
            close(pipeDescriptors[1]);
         spawnFailed = 1;
         break;
      }

      if (processGroup == 0)
         processGroup = pid;
      setpgid(pid, processGroup);
      processIds[processCount++] = pid;
      if (previousRead >= 0)
         close(previousRead);
      if (pipeDescriptors[1] >= 0)
         close(pipeDescriptors[1]);
      previousRead = pipeDescriptors[0];
   }
   if (previousRead >= 0)
      close(previousRead);

   if (spawnFailed)
   {
      if (processGroup > 0)
         kill(-processGroup, SIGTERM);
      for (int index = 0; index < processCount; index++)
         waitpid(processIds[index], NULL, 0);
      sigprocmask(SIG_SETMASK, &previousMask, NULL);
      return -1;
   }

   int jobIndex = registerShellJob(processGroup, processIds, processCount, line, background);
   sigprocmask(SIG_SETMASK, &previousMask, NULL);
   if (jobIndex < 0)
   {
      kill(-processGroup, SIGTERM);
      for (int index = 0; index < processCount; index++)
         waitpid(processIds[index], NULL, 0);
      fprintf(stderr, "Maximum number of active jobs reached\n");
      return -1;
   }

   if (background)
   {
      printf("[%d] %d\n", shellJobs[jobIndex].id, (int)processGroup);
      fflush(stdout);
   }
   else
   {
      struct timeval startTime;
      struct timeval endTime;
      gettimeofday(&startTime, NULL);
      waitForShellJob(jobIndex);
      gettimeofday(&endTime, NULL);
      double elapsed = (endTime.tv_sec - startTime.tv_sec) +
                       (endTime.tv_usec - startTime.tv_usec) / 1000000.0;
      if (elapsed > 2.0)
      {
         snprintf(processName, MAX, "%s", commands[0].arguments[0]);
         processTime = elapsed;
      }
   }
   return 0;
}

static int execute_input(const char *input, char *curWorkingDir, char *prevDir,
                         char *homePath, char *startDirectory)
{
   ShellToken tokens[INP_MAX];
   char storage[INP_MAX];
   int tokenCount = 0;
   if (lex_command(input, tokens, &tokenCount, storage, sizeof(storage)) != 0)
   {
      fprintf(stderr, "Unable to parse command line\n");
      return -1;
   }

   int result = 0;
   int start = 0;
   for (int index = 0; index <= tokenCount; index++)
   {
      int atEnd = index == tokenCount;
      int separator = !atEnd && (tokens[index].type == TOKEN_SEMICOLON ||
                                 tokens[index].type == TOKEN_BACKGROUND);
      if (!atEnd && !separator)
         continue;
      if (index > start)
      {
         PipelineCommand commands[MAX_PIPELINE_COMMANDS];
         int commandCount = 0;
         if (parse_pipeline(tokens, start, index, commands, &commandCount) != 0)
         {
            result = -1;
         }
         else
         {
            int background = separator && tokens[index].type == TOKEN_BACKGROUND;
            result = execute_pipeline(commands, commandCount, background, input,
                                      curWorkingDir, prevDir, homePath, startDirectory);
         }
      }
      else if (separator && tokens[index].type == TOKEN_BACKGROUND)
      {
         fprintf(stderr, "Syntax error: '&' must follow a command\n");
         result = -1;
      }
      start = index + 1;
      if (shellExitRequested)
         break;
   }
   return result;
}

void tokenizeExecuteCommands(char command[INP_MAX], char *commandList[MAX_CMDS],
                             char currDir[PATH_MAX_], char prevDir[PATH_MAX_],
                             char homePath[PATH_MAX_], char startDirectory[PATH_MAX_])
{
   (void)commandList;
   if (command[0] == '\0')
      return;
   execute_input(command, currDir, prevDir, homePath, startDirectory);
   addCommandToHist(command, startDirectory);
}

int commandHandler(char command[PATH_MAX_], char curWorkingDir[PATH_MAX_],
                   char prevDir[PATH_MAX_], char homePath[PATH_MAX_],
                   char startDirectory[PATH_MAX_])
{
   return execute_input(command, curWorkingDir, prevDir, homePath, startDirectory);
}
