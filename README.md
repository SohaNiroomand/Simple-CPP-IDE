# 🧠 Simple C++ IDE

A minimal **C++ command-line IDE** implemented entirely in a single file (`main.cpp`), demonstrating how compilation, execution, and basic tooling can be built from scratch.

---

## 🚀 Overview

This project is a simplified implementation of a development environment that:

- Compiles C++ files using GCC  
- Executes compiled programs  
- Captures and displays compiler output  
- Stores compilation logs  
- Provides a basic command-driven interface  

Everything is implemented inside:

```
main.cpp
```

---

## ⚙️ Features

### 🧾 Command-Based Interface

The program runs as a simple interactive shell:

```
IDE>
```

Supported commands:

```
compile <filename>   Compile a C++ file
run                  Run compiled program
runw                 Run in new terminal window
logs [count]         Show recent logs
exit                 Exit IDE
```

---

### ⚙️ Compilation System

- Uses `g++` via system calls
- Captures compiler output (errors/warnings)
- Determines success/failure automatically

```cpp
std::string cmd = "g++ " + filename + " -o program.exe 2>&1";
```

---

### 📤 Output Capture

Compiler output is captured using pipes:

```cpp
FILE* pipe = _popen(cmd.c_str(), "r");
```

---

### 📝 Logging System

Each compilation generates a log entry:

- Timestamp  
- File name  
- Status (success/failure)  
- Compiler output  

Logs are stored in:

```
logs/compile_log.txt
```

---

### ▶️ Program Execution

Supports two modes:

- Run in current terminal:
  ```
  run
  ```
- Run in a new window:
  ```
  runw
  ```

---

### 🧱 Internal Structure

Even though it's one file, the code is logically structured:

- Command parsing
- Compilation handler
- Execution handler
- Logging system
- Utility functions

Key structs:

```cpp
struct CompilerResult;
struct Command;
struct CompileLog;
```

---

## 🏗️ Project Structure

```
.
├── main.cpp
└── logs/
    └── compile_log.txt
```

---

## 🔧 Build & Run

### Compile

```bash
g++ main.cpp -o ide -std=c++17
```

### Run

```bash
./ide
```

---

## 💡 Example Usage

```
IDE> compile hello.cpp
Compilation successful!

IDE> run
Hello, World!

IDE> logs 3
```

---

## 🧠 What This Project Shows

- OS interaction in C++
- Integrating compilers into tools
- CLI-based program design
- Process output handling

---

## ⚠️ Limitations

- No GUI  
- No text editor  
- Windows-specific parts (`_popen`, `.exe`)  
- Basic functionality only  

---

## 🚀 Possible Improvements

- Modularize into multiple files
- Cross-platform support
- Add editor (SDL / ncurses)
- Improve error handling

---

## 💬 Final Note

This project focuses on understanding how development tools work internally rather than building a full-featured IDE.
