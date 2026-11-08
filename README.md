# C-Shell

A small Unix-like command shell written in C. It combines custom filesystem and process utilities with external command execution, pipelines, redirection, and basic interactive job control.

## Features

- Displays a colored prompt with the current user, host, and working directory. Foreground commands taking longer than two seconds are reported in the next prompt.
- Runs external programs through `PATH` and provides built-ins for directory navigation, listing, search, process inspection, history, and job control.
- Parses single- and double-quoted arguments and backslash escapes.
- Runs commands sequentially with `;`, in the background with `&`, and connects pipeline stages with `|`.
- Supports input redirection with `<`, output truncation with `>`, and output append with `>>`.
- Tracks foreground and background process groups. `jobs`, `fg`, and `bg` list and resume jobs; terminal signals such as Ctrl+C and Ctrl+Z apply to the foreground job.
- Saves up to 15 recent commands in `history.txt` in the directory where the shell starts. Consecutive duplicate entries are skipped.
- Sends `SIGHUP` to remaining jobs and restores terminal ownership when exiting.

## Built-In Commands

- `hop [directory ...]` changes directory. Supports the starting directory (`~`), the previous directory (`-`), and relative or absolute paths. With no argument, returns to the starting directory.
- `reveal [-a] [-l] [directory]` lists directory entries. `-a` includes hidden entries; `-l` displays permissions, ownership, size, and modification time. Names are color-coded by file type.
- `seek [-d|-f] [-e] <prefix> [directory]` recursively searches for names beginning with the given prefix. `-d` selects directories, `-f` selects files, and `-e` acts on a unique match: it changes into a directory or prints a file's contents.
- `proclore [pid]` displays process state, process group, virtual-memory information, and executable path using Linux `/proc` data. Without a PID, it reports the shell process.
- `log` prints command history; `log purge` clears it; `log execute <n>` re-runs a saved entry, where `1` refers to the most recent entry.
- `jobs` lists tracked jobs. `fg <job-id|pid>` brings a job to the foreground; `bg <job-id|pid>` resumes it in the background. Job IDs can be written as `%1`.
- `exit` or Ctrl+D exits the shell.

## Operators

```sh
sleep 5; printf '%s\n' "hello world" | wc -l
cat < input.txt | sort > output.txt
printf '%s\n' "background task" > task.txt &
```

## Build and Run

Build on a Linux/POSIX environment with GCC and GNU Make:

```sh
make
./a.out
```

Use `make clean` to remove the executable.

## Source Layout

- `main.c` starts the shell and manages the prompt/input loop.
- `shell.h` contains shared declarations and limits.
- `command_parser.c` parses and executes commands, pipelines, and redirections.
- `directory.c` provides working-directory, user, and host information.
- `hop.c`, `reveal.c`, `seek.c`, and `proclore.c` implement the corresponding built-ins.
- `history.c` handles persistent command history.
- `jobs.c` manages process groups, signals, and job control.
- `Makefile` builds the shell.

## Platform and Limits

The shell uses POSIX process and terminal APIs; `proclore` specifically requires Linux `/proc`. The input buffer is limited to 4,096 bytes, each command to 100 arguments, pipelines to 32 stages, and the job table to 100 jobs.

The parser does not perform variable or command substitution, wildcard expansion, or logical `&&`/`||` evaluation. Redirection is limited to standard input and standard output; stderr redirection and heredocs are not implemented. Aliases, shell functions, `iMan`, `neonate`, and `ping` are not part of this source version.