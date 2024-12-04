[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/Qiz9msrr)
# mini-project-1-template

# Shell Specifications

## General

1. **Path Length Limits**:
   - Maximum length for paths: `4096` characters.
   - Maximum length for input commands: `4096` characters.
   - Maximum number of commands in history: `100`.

2. **Directory Change Requirements**:
   - If no directory change has occurred previously, commands such as `hop -` or `reveal -a -` will result in an error stating that no previous directory is set.

3. ***If the current working directory is the home directory (the directory where the shell was run), then even if the hop .. command is entered, the shell remains in the home directory instead of moving up one level.***

## Specifications

### Spec-3: Home Directory Handling
- The `~` symbol represents the home directory.
- All directories relative to `~` will be handled accordingly.
- If no argument is provided, the directory changes to the shell's home path, where it was executed.

### Spec-4: File and Directory Listing
- Directories will be printed in blue color.
- File names will be printed in green color.

### Spec-5: Command History
- Only the most recent 15 commands are stored.
- Duplicate commands (identical to the previous command) will not be stored again.

### Spec-6: Process Timing
- For multiple foreground processes, if some take more than 2 seconds, the one that ends last is considered.

### Spec-8: Path Handling
- Paths will be managed similarly to the `hop` and `reveal` commands.

