# ZainOS — Advanced Operating System Simulator

![C++17](https://img.shields.io/badge/C%2B%2B-17-blue?style=flat-square)
![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-lightgrey?style=flat-square)
![Version](https://img.shields.io/badge/Version-2.0.0-green?style=flat-square)
![Status](https://img.shields.io/badge/Status-Active-brightgreen?style=flat-square)

> A Linux-inspired terminal OS simulator built entirely from scratch in C++17 — featuring a real shell, CPU scheduling algorithms, virtual filesystem, process management, multi-user authentication, and a fully themed ANSI terminal UI.

---

## What is ZainOS?

ZainOS is not a toy project. It is a fully functional operating system simulator that mimics the behavior of a real Linux terminal environment — built without any OS libraries or frameworks. Every subsystem was designed and implemented from scratch using modern C++17.

---

## Features

### Animated Boot Sequence
- Custom ASCII boot screen with animated progress bar
- Linux-style initialization messages on startup

### Multi-User Authentication System
- Secure login system with username and password
- Support for multiple user accounts
- Session management with logout and re-login

### Virtual In-Memory Filesystem
- Create, delete, read, and write files
- Directory creation and navigation
- Supports: `cd`, `ls`, `mkdir`, `rm`, `pwd`, `cat`, `touch`, `cp`, `mv`
- Persistent in-session directory tree

### Process Management
- Live process table (like Linux `top` command)
- View PID, name, user, status, priority, CPU time, and memory usage
- `kill <pid>` — terminate any running process
- `renice <pid> <priority>` — change process priority dynamically
- Real-time refresh with Enter key

### CPU Scheduling Simulator
Three algorithms fully implemented with visual Gantt chart output:

- **FCFS** — First Come First Serve
- **Priority Scheduling** — with user-defined priorities
- **Round Robin** — with configurable time quantum

Each algorithm outputs:
- Gantt chart visualization
- Waiting time per process
- Turnaround time per process
- Average waiting and turnaround times

### ANSI Themed Terminal UI
4 built-in themes, switchable at runtime:

| Theme | Style |
|---|---|
| `dark` | Default — white text on black background |
| `hacker` | Classic green-on-black Matrix style |
| `blue` | Cyan/blue professional terminal |
| `red` | High contrast red accent theme |

### Shell Command Interface

| Command | Description |
|---|---|
| `ls` | List directory contents |
| `cd` | Change directory |
| `mkdir` | Create new directory |
| `touch` | Create new file |
| `cat` | Display file contents |
| `rm` | Remove file or directory |
| `cp` / `mv` | Copy / move files |
| `pwd` | Print working directory |
| `ps` | Show process table |
| `top` | Live process monitor |
| `kill` | Terminate a process by PID |
| `renice` | Change process priority |
| `fcfs` | Run FCFS CPU scheduler |
| `priority` | Run Priority CPU scheduler |
| `roundrobin` | Run Round Robin CPU scheduler |
| `theme` | Switch terminal theme |
| `sysinfo` | Display system information |
| `uptime` | Show system uptime |
| `history` | View command history |
| `clear` | Clear terminal screen |
| `logout` | Logout current user |
| `exit` | Shutdown ZainOS |

---

## Tech Stack

| Category | Details |
|---|---|
| Language | C++17 |
| Terminal UI | ANSI Escape Codes (cross-platform) |
| Data Structures | `std::map`, `std::unordered_map`, `std::vector` |
| Concurrency | `std::chrono`, `std::thread` |
| Platform Support | Windows (ANSI enabled) + Linux / macOS |
| Build | g++ / any C++17 compatible compiler |

---

## How to Run

### Windows
```bash
g++ -std=c++17 main.cpp -o ZainOS.exe
ZainOS.exe
```

### Linux / macOS
```bash
g++ -std=c++17 main.cpp -o ZainOS
./ZainOS
```

> Requires a C++17 compatible compiler — GCC 7+, Clang 5+, or MSVC 2017+

---

## Project Structure

```
ZainOS/
│
├── main.cpp                  # Complete source (1,700+ lines)
│
└── Internal Sections:
    ├── Section 1  — ANSI Color & Theme Engine
    ├── Section 2  — Theme Manager (4 themes)
    ├── Section 3  — Utility Helpers
    ├── Section 4  — Animated Boot Sequence
    ├── Section 5  — Multi-User Authentication System
    ├── Section 6  — Virtual In-Memory Filesystem
    ├── Section 7  — Process Manager
    ├── Section 8  — CPU Scheduling Algorithms (FCFS, Priority, RR)
    ├── Section 9  — Login System
    ├── Section 10 — Shell (Command Interpreter)
    ├── Section 11 — Scheduling Commands
    └── Section 12 — Main Entry Point
```

---

## Why I Built This

Most CS students study operating systems from a textbook.
I wanted to actually build one.

ZainOS taught me how real systems work under the hood — how a shell parses commands, how a filesystem stores data in memory, how a CPU scheduler decides which process runs next, and how a login system manages user sessions securely.

Every line was written by hand. No templates, no frameworks, no shortcuts.

---

## Author

**Muhammad Zain Tahir**  
B.S. Software Engineering — University of Lahore (4th Semester)  
Lahore, Pakistan

[![LinkedIn](https://img.shields.io/badge/LinkedIn-Connect-blue?style=flat-square&logo=linkedin)](https://www.linkedin.com/in/muhammad-zain-tahir07)
[![GitHub](https://img.shields.io/badge/GitHub-Follow-black?style=flat-square&logo=github)](https://github.com/m-zain-tahir)

---

> *"The best way to understand an operating system is to build one."*
