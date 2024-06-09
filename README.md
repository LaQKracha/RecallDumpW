# 🚀 RecallDump

RecallDump is a C program designed to automate the extraction and management of Windows Recall data. It handles tasks such as modifying permissions, creating directories, copying files and directories, renaming images, and processing databases.

## ✨ Flow

- 🛠️ Modify directory permissions
- 📁 Create directory
- 📄 Copy files and directories
- 🖼️ Rename images to a specific format
- 🗄️ Process SQLite databases

## 💻 Usage

Compile the program using a C compiler and then run it with the desired options.

### 🔧 Compilation

```sh
gcc -o RecallDump RecallDump.c -lsqlite3
```
or
```sh
gcc -o Recall Recall.c -lshlwapi -lshell32 -lsqlite3
```

### ▶️ Running

```sh
./RecallDump [options]
```

### ⚙️ Options

- `-h` : Show the help message
- `-f` : Specify the starting date for the extraction (format: YYYY-MM-DD)
- `-t` : Specify the ending date for the extraction (format: YYYY-MM-DD)
- `-s` : Specify a search term to filter the extraction

## 📝 Example

```sh
./RecallDump -f 2023-01-01 -t 2023-12-31 -s "example search term"
```

## 🔍 Functions

- `modify_permissions()`
  - Modifies the permissions of the specified path (Windows only).

- `create_directory()`
  - Creates a directory at the specified path.

- `copy_file()`
  - Copies a file from the source path to the destination path.

- `copy_directory()`
  - Copies a directory from the source path to the destination path (Windows only).

- `get_current_time_string()`
  - Gets the current time as a string.

- `process_database()`
  - Processes the SQLite database at the specified path.

- `rename_images()`
  - Renames images in the specified directory (Windows only).

- `void display_help()`
  - Displays the help message.

## 🎨 Color Coding

The program uses color coding to differentiate between different types of messages:
- **BLUE**: Banner and usage messages
- **GREEN**: Successful operations
- **YELLOW**: Warnings and unsupported operations
- **RED**: Errors
- **CYAN**: Prompts and processing messages
- **MAGENTA**: Option descriptions
- **WHITE**: Database entry details

## 🤝 Contributing

Feel free to submit issues and pull requests. For major changes, please open an issue first to discuss what you would like to change.

---

Enjoy using RecallDump! 🎉
