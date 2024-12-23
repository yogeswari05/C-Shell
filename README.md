# C Shell Documentation

## Overview

This C Shell implementation provides a variety of features and functionalities to manage directories, execute commands, handle background processes, and maintain command history. Below is a detailed explanation of each feature and its corresponding functionality.

## Features and Functionalities

### 1. Directory Management

- **Get Current Working Directory**
  - Function: `getCurrWorkingDir`
  - Retrieves the current working directory and stores it in the provided buffer.

- **Change Directory**
  - Function: `changeDirectory`
  - Changes the current working directory to the specified path. Handles special cases like `~` and `..`.

- **Get Hostname**
  - Function: `getHostName_`
  - Retrieves the hostname of the system.

- **Get Username**
  - Function: `getUserName`
  - Retrieves the username of the current user.

### 2. Command Handling

- **Tokenize and Execute Commands**
  - Function: `tokenizeExecuteCommands`
  - Tokenizes the input command string and executes each command sequentially. Supports background execution using `&`.

- **Command Handler**
  - Function: `commandHandler`
  - Handles individual commands by invoking the appropriate function based on the command name.

- **Trim Whitespace**
  - Function: `trimWhitespace`
  - Removes leading and trailing whitespace from a string.

### 3. Directory Navigation

- **Hop Handler**
  - Function: `hopHandler`
  - Handles the `hop` command to change directories. Supports special cases like `~` and `-`.

### 4. File and Directory Listing

- **Reveal Main Handler**
  - Function: `revealMainHandler`
  - Handles the `reveal` command to list files and directories. Supports flags `-a` for all files and `-l` for detailed information.

- **Print Detailed Info**
  - Function: `printDetailedInfo`
  - Prints detailed information about a file or directory.

- **Print with Colors**
  - Function: `printWithColors`
  - Prints file or directory names with color coding based on their type.

### 5. Command History

- **Load History**
  - Function: `loadHistory`
  - Loads command history from a file.

- **Save History**
  - Function: `saveHistory`
  - Saves command history to a file.

- **Add Command to History**
  - Function: `addCommandToHist`
  - Adds a command to the history, avoiding duplicates.

- **Print Total History**
  - Function: `printTotalHist`
  - Prints the entire command history.

- **Remove Unnecessary Spaces and Tabs**
  - Function: `removeUncessecarySpaceTabs`
  - Removes unnecessary spaces and tabs from a string.

- **Log Purge Handler**
  - Function: `logPurgeHandler`
  - Handles the `log purge` command to clear the command history.

- **Log Execute Handler**
  - Function: `logExecuteHandler`
  - Handles the `log execute` command to re-execute a command from history.

### 6. Background Process Management

- **Setup Signal Handling**
  - Function: `setupSignalHandling`
  - Sets up signal handling for background processes.

- **Add Background Process**
  - Function: `addBackgroundProcess`
  - Adds a background process to the list of background processes.

- **Remove Background Process**
  - Function: `removeBackgroundProcess`
  - Removes a background process from the list of background processes.

- **Handle SIGCHLD**
  - Function: `handle_sigchld`
  - Handles the `SIGCHLD` signal to clean up terminated background processes.

- **Run in Background**
  - Function: `runInBackground`
  - Executes a command in the background.

### 7. Process Information

- **Proclore Handler**
  - Function: `procloreHandler`
  - Handles the `proclore` command to display detailed information about a process.

### 8. File Search

- **Seek Handler**
  - Function: `seekHandler`
  - Handles the `seek` command to search for files and directories. Supports flags `-d` for directories, `-f` for files, and `-e` for executing the found file or changing to the found directory.

## Available Commands

### hop

- **Description**: Changes the current directory.
- **Usage**: `hop [directory]`
- **Examples**:
  - `hop ~` - Changes to the home directory.
  - `hop -` - Changes to the previous directory.

### reveal

- **Description**: Lists files and directories.
- **Usage**: `reveal [-a] [-l] [directory]`
- **Examples**:
  - `reveal -a` - Lists all files including hidden ones.
  - `reveal -l` - Lists files with detailed information.

### log

- **Description**: Manages command history.
- **Usage**: 
  - `log` - Prints the command history.
  - `log purge` - Clears the command history.
  - `log execute [n]` - Re-executes the nth command from history.

### proclore

- **Description**: Displays detailed information about a process.
- **Usage**: `proclore [pid]`
- **Examples**:
  - `proclore` - Displays information about the current process.
  - `proclore 1234` - Displays information about the process with PID 1234.

### seek

- **Description**: Searches for files and directories.
- **Usage**: `seek [-d] [-f] [-e] target [directory]`
- **Examples**:
  - `seek -d target` - Searches for directories matching the target.
  - `seek -f target` - Searches for files matching the target.
  - `seek -e target` - Executes or changes to the found file or directory.

## Usage

To use this C Shell, compile the source files and run the resulting executable. The shell supports various built-in commands as described above. For example:

- `hop ~` to change to the home directory.
- `reveal -a -l` to list all files and directories with detailed information.
- `log purge` to clear the command history.
- `seek -d target` to search for directories matching the target.

## Conclusion

This C Shell provides a comprehensive set of features for managing directories, executing commands, handling background processes, and maintaining command history. It is designed to be a functional and user-friendly shell environment.