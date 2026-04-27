# 🧠 Simple C++ IDE (Educational Project)

A lightweight **C++ IDE built from scratch**, designed to understand how real development environments work under the hood.

This project was developed as the final assignment for the *Fundamentals of Programming (C++)* course at Sharif University of Technology.

---

## 🚀 What is this?

This project implements a **mini IDE** with core features of real tools like CLion:

- Text editing
- Syntax highlighting
- File management
- Keyboard shortcuts
- Basic debugging
- Compilation & execution using GCC

The goal is not to compete with real IDEs — it’s to **build one from scratch and actually understand it**.

---

## 🧩 Features

### ✏️ Text Editor (Core)

- Multi-line text editing
- Cursor movement & control
- Scroll handling for large files
- Efficient rendering (only visible lines)

```cpp
std::vector<std::string> lines;
```

---

### 🎨 Syntax Highlighting

Supports basic C++ syntax coloring:

- Keywords (`if`, `while`, `class`)
- Types (`int`, `float`)
- Functions
- Variables
- Strings & numbers
- Comments

Includes:
- Light mode
- Dark mode

---

### 💾 File Management

- Save projects with custom names
- Project list (tree-style)
- Load existing projects

---

### ⌨️ Keyboard Shortcuts

| Shortcut | Action |
|--------|--------|
| Ctrl + C | Copy |
| Ctrl + V | Paste |
| Ctrl + X | Cut |
| Ctrl + A | Select All |
| Ctrl + Z | Undo |
| Ctrl + S | Save |
| Ctrl + G | Go to Line |

---

### 🐞 Debugging (Static Analysis)

Detects basic syntax errors:

- Missing `;`
- Unmatched brackets
- Misspelled keywords
- Invalid operators
- Undefined variables
- String errors

---

### ⚙️ Compiler Integration

Uses GCC via system calls:

```cpp
system("g++ file.cpp -o program.exe");
```

Captures output:

```cpp
FILE* pipe = _popen(cmd.c_str(), "r");
```

---

### ▶️ Run & Logs

Commands:

```
compile main.cpp
run
runw
logs 5
exit
```

---

## 🏗️ Project Structure

```
project/
├── src/
├── include/
├── logs/
└── README.md
```

---

## 🔧 Build & Run

```bash
g++ main.cpp -o ide -std=c++17
./ide
```

---

## 🧠 What You Learn

- How text editors work
- How IDEs interact with compilers
- File & state management
- OS interaction in C++
- System-level programming basics

---

## 🧑‍💻 Course Info

Sharif University of Technology  
Fundamentals of Programming (C++)  
Fall 2024  

---

## 💬 Final Note

This project is about understanding how tools are built — not just using them.
