#include "shell.h"

ShellJob shellJobs[MAX_JOBS];
int shellInteractive = 0;
pid_t shellPgid = -1;
volatile sig_atomic_t childEvent = 0;
volatile sig_atomic_t interruptEvent = 0;
int shellExitRequested = 0;
static int nextJobId = 1;

static void handle_sigint(int signalNumber)
{
   (void)signalNumber;
   interruptEvent = 1;
}

void handle_sigchld(int signalNumber)
{
   (void)signalNumber;
   childEvent = 1;
}

static void set_signal_action(int signalNumber, void (*handler)(int), int flags)
{
   struct sigaction action;
   memset(&action, 0, sizeof(action));
   action.sa_handler = handler;
   action.sa_flags = flags;
   sigemptyset(&action.sa_mask);
   sigaction(signalNumber, &action, NULL);
}

void setupSignalHandling(void)
{
   shellInteractive = isatty(STDIN_FILENO);
   if (shellInteractive)
   {
      while (tcgetpgrp(STDIN_FILENO) != (shellPgid = getpgrp()))
         kill(-shellPgid, SIGTTIN);

      shellPgid = getpid();
      if (setpgid(shellPgid, shellPgid) < 0 && errno != EACCES && errno != EPERM)
         perror("setpgid");
      if (tcsetpgrp(STDIN_FILENO, shellPgid) < 0)
         perror("tcsetpgrp");
      set_signal_action(SIGQUIT, SIG_IGN, 0);
      set_signal_action(SIGTSTP, SIG_IGN, 0);
      set_signal_action(SIGTTIN, SIG_IGN, 0);
      set_signal_action(SIGTTOU, SIG_IGN, 0);
   }
   set_signal_action(SIGINT, handle_sigint, 0);
   set_signal_action(SIGCHLD, handle_sigchld, 0);
}

static int live_process_count(const ShellJob *job)
{
   int liveCount = 0;
   for (int index = 0; index < job->processCount; index++)
      if (job->processes[index].state != JOB_PROCESS_DONE)
         liveCount++;
   return liveCount;
}

static int job_is_stopped(const ShellJob *job)
{
   int liveCount = 0;
   for (int index = 0; index < job->processCount; index++)
   {
      if (job->processes[index].state == JOB_PROCESS_DONE)
         continue;
      liveCount++;
      if (job->processes[index].state != JOB_PROCESS_STOPPED)
         return 0;
   }
   return liveCount > 0;
}

static void update_job_process(pid_t pid, int status)
{
   for (int jobIndex = 0; jobIndex < MAX_JOBS; jobIndex++)
   {
      ShellJob *job = &shellJobs[jobIndex];
      if (!job->used)
         continue;
      for (int processIndex = 0; processIndex < job->processCount; processIndex++)
      {
         JobProcess *process = &job->processes[processIndex];
         if (process->pid != pid)
            continue;
         if (WIFEXITED(status) || WIFSIGNALED(status))
            process->state = JOB_PROCESS_DONE;
         else if (WIFSTOPPED(status))
            process->state = JOB_PROCESS_STOPPED;
         else if (WIFCONTINUED(status))
            process->state = JOB_PROCESS_RUNNING;
         return;
      }
   }
}

int registerShellJob(pid_t pgid, pid_t *pids, int processCount,
                     const char *command, int background)
{
   if (processCount <= 0 || processCount > MAX_PIPELINE_COMMANDS)
      return -1;
   for (int index = 0; index < MAX_JOBS; index++)
   {
      if (shellJobs[index].used)
         continue;
      ShellJob *job = &shellJobs[index];
      memset(job, 0, sizeof(*job));
      job->id = nextJobId++;
      job->pgid = pgid;
      job->processCount = processCount;
      job->background = background;
      job->used = 1;
      snprintf(job->command, sizeof(job->command), "%s", command);
      for (int processIndex = 0; processIndex < processCount; processIndex++)
      {
         job->processes[processIndex].pid = pids[processIndex];
         job->processes[processIndex].state = JOB_PROCESS_RUNNING;
      }
      return index;
   }
   return -1;
}

void waitForShellJob(int jobIndex)
{
   if (jobIndex < 0 || jobIndex >= MAX_JOBS || !shellJobs[jobIndex].used)
      return;
   ShellJob *job = &shellJobs[jobIndex];
   int wasBackground = job->background;
   job->background = 0;
   if (shellInteractive && tcsetpgrp(STDIN_FILENO, job->pgid) < 0)
      perror("tcsetpgrp");
   if (wasBackground && job_is_stopped(job))
   {
      if (kill(-job->pgid, SIGCONT) < 0)
         perror("fg");
      for (int index = 0; index < job->processCount; index++)
         if (job->processes[index].state == JOB_PROCESS_STOPPED)
            job->processes[index].state = JOB_PROCESS_RUNNING;
   }

   while (live_process_count(job) > 0 && !job_is_stopped(job))
   {
      int status;
      pid_t pid = waitpid(-job->pgid, &status, WUNTRACED | WCONTINUED);
      if (pid > 0)
      {
         update_job_process(pid, status);
      }
      else if (pid < 0 && errno == EINTR)
      {
         continue;
      }
      else
      {
         break;
      }
   }

   if (shellInteractive && tcsetpgrp(STDIN_FILENO, shellPgid) < 0)
      perror("tcsetpgrp");
   if (job_is_stopped(job))
   {
      job->background = 1;
      job->notified = 1;
      printf("\n[%d] Stopped\t%s\n", job->id, job->command);
      fflush(stdout);
   }
   else
   {
      memset(job, 0, sizeof(*job));
   }
}

void pollShellEvents(void)
{
   if (!childEvent)
      return;
   childEvent = 0;
   int status;
   pid_t pid;
   while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0)
      update_job_process(pid, status);

   for (int index = 0; index < MAX_JOBS; index++)
   {
      ShellJob *job = &shellJobs[index];
      if (!job->used || !job->background)
         continue;
      if (live_process_count(job) == 0)
      {
         printf("\n[%d] Done\t%s\n", job->id, job->command);
         memset(job, 0, sizeof(*job));
      }
      else if (job_is_stopped(job) && !job->notified)
      {
         printf("\n[%d] Stopped\t%s\n", job->id, job->command);
         job->notified = 1;
      }
   }
   fflush(stdout);
}

static int find_job(const char *identifier)
{
   if (identifier == NULL)
      return -1;
   char *end = NULL;
   long value = strtol(identifier[0] == '%' ? identifier + 1 : identifier, &end, 10);
   if (end == identifier || (identifier[0] == '%' && end == identifier + 1) || *end != '\0')
      return -1;
   for (int index = 0; index < MAX_JOBS; index++)
   {
      ShellJob *job = &shellJobs[index];
      if (!job->used)
         continue;
      if ((identifier[0] == '%' && job->id == value) ||
          (identifier[0] != '%' && (job->pgid == value || job->id == value)))
         return index;
      for (int processIndex = 0; identifier[0] != '%' && processIndex < job->processCount; processIndex++)
         if (job->processes[processIndex].pid == value)
            return index;
   }
   return -1;
}

int handleJobBuiltin(char **terms, int count, int inParent)
{
   if (strcmp(terms[0], "jobs") == 0)
   {
      pollShellEvents();
      for (int index = 0; index < MAX_JOBS; index++)
      {
         ShellJob *job = &shellJobs[index];
         if (!job->used)
            continue;
         const char *state = job_is_stopped(job) ? "Stopped" : "Running";
         printf("[%d] %s\t%s\n", job->id, state, job->command);
      }
      return 1;
   }
   if (strcmp(terms[0], "fg") != 0 && strcmp(terms[0], "bg") != 0)
      return 0;
   if (!inParent)
   {
      fprintf(stderr, "%s: job control is only available in the shell\n", terms[0]);
      return 1;
   }
   if (count != 2)
   {
      fprintf(stderr, "Usage: %s <%%job-id|pid>\n", terms[0]);
      return 1;
   }

   pollShellEvents();
   int jobIndex = find_job(terms[1]);
   if (jobIndex < 0)
   {
      fprintf(stderr, "%s: no such job: %s\n", terms[0], terms[1]);
      return 1;
   }
   ShellJob *job = &shellJobs[jobIndex];
   if (strcmp(terms[0], "fg") == 0)
   {
      printf("%s\n", job->command);
      job->notified = 0;
      waitForShellJob(jobIndex);
   }
   else
   {
      if (kill(-job->pgid, SIGCONT) < 0)
      {
         perror("bg");
         return 1;
      }
      for (int index = 0; index < job->processCount; index++)
         if (job->processes[index].state == JOB_PROCESS_STOPPED)
            job->processes[index].state = JOB_PROCESS_RUNNING;
      job->background = 1;
      job->notified = 0;
      printf("[%d] %s\n", job->id, job->command);
   }
   return 1;
}

void shutdownShell(void)
{
   pollShellEvents();
   for (int index = 0; index < MAX_JOBS; index++)
   {
      ShellJob *job = &shellJobs[index];
      if (!job->used)
         continue;
      kill(-job->pgid, SIGHUP);
      kill(-job->pgid, SIGCONT);
   }
   if (shellInteractive && shellPgid > 0)
      tcsetpgrp(STDIN_FILENO, shellPgid);
}
