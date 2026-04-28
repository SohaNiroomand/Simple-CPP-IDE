## Overview

This project is a simple graphical C++ IDE built with **SDL2**.

It provides a custom text editor where users can write C++ code, edit multiple lines, move the cursor, select text, copy/paste/cut, undo/redo changes, switch between light and dark mode, and run the written code through `g++`.

The editor stores the current file as a list of text lines, renders the code manually using `SDL_ttf`, and applies basic syntax highlighting for keywords, data types, strings, comments, numbers, operators, and brackets.

The program also includes a basic debugging panel that detects simple syntax issues such as missing semicolons, invalid variable names, unclosed strings, invalid operators, unmatched multiline comments, and repeated `if` conditions.

## Features

- Graphical editor window using SDL2
- Syntax highlighting
- Light and dark mode backgrounds
- Multi-line text editing
- Cursor movement with arrow keys
- Mouse-based cursor positioning
- Text selection with mouse drag
- Copy, cut, paste
- Undo and redo
- Vertical and horizontal scrolling
- Auto-completion for brackets: `()`, `{}`, `[]`
- Save project name list
- New file button
- Debug panel for basic syntax checks
- Compile and run current code using `g++`
- Run button and `Ctrl + R` shortcut

## Shortcuts

| Shortcut | Action |
|---|---|
| `Ctrl + C` | Copy selected text |
| `Ctrl + V` | Paste copied text |
| `Ctrl + X` | Cut selected text |
| `Ctrl + A` | Select all |
| `Ctrl + Z` | Undo |
| `Ctrl + Y` | Redo |
| `Ctrl + S` | Save project name |
| `Ctrl + G` | Go to line |
| `Ctrl + R` | Compile and run |

## Build

```bash
g++ main.cpp -o editor -lSDL2 -lSDL2_ttf -lSDL2_image
```
## Collaborators

- Soha Niroomand
- Mohammad Mahdi Goudarzi
