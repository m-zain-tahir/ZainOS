/**
 * ============================================================
 *  ZainOS — Advanced Operating System Simulator
 *  Author  : ZainOS Development Team
 *  Version : 2.0.0
 *  Standard: C++17
 *
 *  A Linux-inspired terminal environment simulator featuring:
 *    • Animated boot sequence
 *    • Multi-user authentication system
 *    • Virtual in-memory file system
 *    • Process management & simulation
 *    • CPU scheduling algorithms (FCFS, Priority, Round-Robin)
 *    • Rich ANSI-colored terminal UI with themes
 *    • Command history, sysinfo, uptime and more
 * ============================================================
 */

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <functional>
#include <chrono>
#include <ctime>
#include <thread>
#include <iomanip>
#include <numeric>
#include <random>
#include <cassert>
#include <optional>
#include <variant>
#include <memory>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

// ============================================================
//  SECTION 1 — ANSI Color & Theme Engine
// ============================================================

namespace ANSI {
    // Reset / formatting
    constexpr const char* RESET   = "\033[0m";
    constexpr const char* BOLD    = "\033[1m";
    constexpr const char* DIM     = "\033[2m";
    constexpr const char* ITALIC  = "\033[3m";
    constexpr const char* UNDER   = "\033[4m";
    constexpr const char* BLINK   = "\033[5m";
    constexpr const char* REVERSE = "\033[7m";

    // Standard foreground
    constexpr const char* FG_BLACK   = "\033[30m";
    constexpr const char* FG_RED     = "\033[31m";
    constexpr const char* FG_GREEN   = "\033[32m";
    constexpr const char* FG_YELLOW  = "\033[33m";
    constexpr const char* FG_BLUE    = "\033[34m";
    constexpr const char* FG_MAGENTA = "\033[35m";
    constexpr const char* FG_CYAN    = "\033[36m";
    constexpr const char* FG_WHITE   = "\033[37m";

    // Bright foreground
    constexpr const char* FG_BRIGHT_BLACK   = "\033[90m";
    constexpr const char* FG_BRIGHT_RED     = "\033[91m";
    constexpr const char* FG_BRIGHT_GREEN   = "\033[92m";
    constexpr const char* FG_BRIGHT_YELLOW  = "\033[93m";
    constexpr const char* FG_BRIGHT_BLUE    = "\033[94m";
    constexpr const char* FG_BRIGHT_MAGENTA = "\033[95m";
    constexpr const char* FG_BRIGHT_CYAN    = "\033[96m";
    constexpr const char* FG_BRIGHT_WHITE   = "\033[97m";

    // Background
    constexpr const char* BG_BLACK   = "\033[40m";
    constexpr const char* BG_RED     = "\033[41m";
    constexpr const char* BG_GREEN   = "\033[42m";
    constexpr const char* BG_BLUE    = "\033[44m";
    constexpr const char* BG_CYAN    = "\033[46m";
    constexpr const char* BG_WHITE   = "\033[47m";

    // Cursor
    constexpr const char* CLEAR_SCREEN = "\033[2J\033[H";
    constexpr const char* CURSOR_UP    = "\033[A";
    constexpr const char* ERASE_LINE   = "\033[2K\r";

    inline void clearScreen() { std::cout << CLEAR_SCREEN; }
    inline void moveCursorUp(int n = 1) {
        std::cout << "\033[" << n << "A";
    }
}

// ============================================================
//  SECTION 2 — Theme Manager
// ============================================================

struct Theme {
    std::string name;
    const char* primary;    // main text / prompt color
    const char* secondary;  // accents, borders
    const char* success;    // success messages
    const char* error;      // error messages
    const char* warning;    // warnings
    const char* info;       // info / dim text
    const char* header;     // section headers
    const char* highlight;  // highlights / directories
};

class ThemeManager {
public:
    std::map<std::string, Theme> themes;
    std::string current = "dark";

    ThemeManager() {
        themes["dark"] = {
            "dark",
            ANSI::FG_BRIGHT_WHITE,
            ANSI::FG_BRIGHT_CYAN,
            ANSI::FG_BRIGHT_GREEN,
            ANSI::FG_BRIGHT_RED,
            ANSI::FG_BRIGHT_YELLOW,
            ANSI::FG_BRIGHT_BLACK,
            ANSI::FG_BRIGHT_CYAN,
            ANSI::FG_BRIGHT_BLUE
        };
        themes["hacker"] = {
            "hacker",
            ANSI::FG_GREEN,
            ANSI::FG_BRIGHT_GREEN,
            ANSI::FG_BRIGHT_GREEN,
            ANSI::FG_RED,
            ANSI::FG_YELLOW,
            ANSI::FG_GREEN,
            ANSI::FG_BRIGHT_GREEN,
            ANSI::FG_GREEN
        };
        themes["blue"] = {
            "blue",
            ANSI::FG_BRIGHT_CYAN,
            ANSI::FG_BRIGHT_BLUE,
            ANSI::FG_BRIGHT_GREEN,
            ANSI::FG_BRIGHT_RED,
            ANSI::FG_BRIGHT_YELLOW,
            ANSI::FG_BLUE,
            ANSI::FG_BRIGHT_BLUE,
            ANSI::FG_CYAN
        };
        themes["red"] = {
            "red",
            ANSI::FG_BRIGHT_WHITE,
            ANSI::FG_BRIGHT_RED,
            ANSI::FG_GREEN,
            ANSI::FG_BRIGHT_RED,
            ANSI::FG_YELLOW,
            ANSI::FG_RED,
            ANSI::FG_BRIGHT_RED,
            ANSI::FG_RED
        };
    }

    const Theme& get() const {
        auto it = themes.find(current);
        if (it != themes.end()) return it->second;
        return themes.at("dark");
    }

    bool setTheme(const std::string& name) {
        if (themes.count(name)) { current = name; return true; }
        return false;
    }
};

// Global theme manager
ThemeManager gTheme;

// Convenience helpers
#define CLR_PRI   gTheme.get().primary
#define CLR_SEC   gTheme.get().secondary
#define CLR_OK    gTheme.get().success
#define CLR_ERR   gTheme.get().error
#define CLR_WARN  gTheme.get().warning
#define CLR_INFO  gTheme.get().info
#define CLR_HEAD  gTheme.get().header
#define CLR_HI    gTheme.get().highlight

// ============================================================
//  SECTION 3 — Utility Helpers
// ============================================================

namespace Utils {

    inline void sleep_ms(int ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }

    // Print a horizontal rule
    inline void hline(const std::string& ch = "\xe2\x94\x80", int width = 70) {
        std::cout << CLR_SEC;
        for (int i = 0; i < width; ++i) std::cout << ch;
        std::cout << ANSI::RESET << "\n";
    }

    // Center a string within a given width
    inline std::string center(const std::string& s, int width) {
        int pad = (width - static_cast<int>(s.size())) / 2;
        if (pad < 0) pad = 0;
        return std::string(pad, ' ') + s;
    }

    // Print centered text with color
    inline void printCentered(const std::string& s, const char* color = ANSI::FG_BRIGHT_WHITE, int width = 70) {
        std::cout << color << center(s, width) << ANSI::RESET << "\n";
    }

    // Split a string by whitespace into tokens
    inline std::vector<std::string> tokenize(const std::string& line) {
        std::vector<std::string> tokens;
        std::istringstream iss(line);
        std::string tok;
        while (iss >> tok) tokens.push_back(tok);
        return tokens;
    }

    // Trim whitespace
    inline std::string trim(const std::string& s) {
        size_t a = s.find_first_not_of(" \t\r\n");
        size_t b = s.find_last_not_of(" \t\r\n");
        if (a == std::string::npos) return "";
        return s.substr(a, b - a + 1);
    }

    // Animated progress bar
    inline void progressBar(const std::string& label, int steps = 30, int delay_ms = 40) {
        std::cout << CLR_SEC << "  [";
        for (int i = 0; i <= steps; ++i) {
            int pct = (i * 100) / steps;
            // Overwrite the line
            std::cout << "\r" << CLR_INFO << "  " << std::left << std::setw(28) << label
                      << CLR_SEC << " [";
            int filled = i;
            for (int j = 0; j < steps; ++j) {
                if (j < filled) std::cout << CLR_OK  << "\xe2\x96\x88";
                else            std::cout << CLR_INFO << "\xe2\x96\x91";
                std::cout << CLR_SEC;
            }
            std::cout << CLR_SEC << "] " << CLR_PRI << std::setw(3) << pct << "%" << ANSI::RESET;
            std::cout.flush();
            sleep_ms(delay_ms);
        }
        std::cout << "\n";
    }

    // Typing effect
    inline void typewrite(const std::string& s, int delay_ms = 18, const char* color = nullptr) {
        if (color) std::cout << color;
        for (char c : s) {
            std::cout << c;
            std::cout.flush();
            sleep_ms(delay_ms);
        }
        if (color) std::cout << ANSI::RESET;
    }

    // Get current date string
    inline std::string currentDate() {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm* tm_info = std::localtime(&t);
        char buf[64];
        std::strftime(buf, sizeof(buf), "%A, %B %d, %Y", tm_info);
        return std::string(buf);
    }

    // Get current time string
    inline std::string currentTime() {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm* tm_info = std::localtime(&t);
        char buf[32];
        std::strftime(buf, sizeof(buf), "%H:%M:%S", tm_info);
        return std::string(buf);
    }

    // Masked password input (cross-platform)
    inline std::string readPassword() {
        std::string pwd;
#ifdef _WIN32
        char ch;
        while ((ch = _getch()) != '\r') {
            if (ch == '\b') {
                if (!pwd.empty()) { pwd.pop_back(); std::cout << "\b \b"; }
            } else {
                pwd += ch;
                std::cout << '*';
            }
        }
        std::cout << "\n";
#else
        struct termios oldt, newt;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ECHO | ICANON);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) {
            if (ch == 127 || ch == '\b') {
                if (!pwd.empty()) { pwd.pop_back(); std::cout << "\b \b"; std::cout.flush(); }
            } else {
                pwd += static_cast<char>(ch);
                std::cout << '*'; std::cout.flush();
            }
        }
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        std::cout << "\n";
#endif
        return pwd;
    }

    // Fake random integer in range
    inline int randInt(int lo, int hi) {
        static std::mt19937 rng(std::random_device{}());
        return std::uniform_int_distribution<int>(lo, hi)(rng);
    }
}

// ============================================================
//  SECTION 4 — Virtual File System
// ============================================================

/**
 * FSNode represents either a directory or a regular file.
 * Directories contain child nodes; files contain text content.
 */
struct FSNode {
    enum class Type { DIR, FILE };
    Type type;
    std::string name;
    std::string content;               // only for FILE
    std::map<std::string, FSNode> children; // only for DIR
    std::time_t created;

    FSNode() : type(Type::DIR), created(std::time(nullptr)) {}
    FSNode(Type t, const std::string& n, const std::string& c = "")
        : type(t), name(n), content(c), created(std::time(nullptr)) {}

    bool isDir()  const { return type == Type::DIR;  }
    bool isFile() const { return type == Type::FILE; }
};

class VirtualFileSystem {
public:
    FSNode root;
    std::vector<std::string> cwdPath; // current working directory path components

    VirtualFileSystem() {
        root.type = FSNode::Type::DIR;
        root.name = "/";

        // Seed with a realistic directory tree
        mkdirp({"home"});
        mkdirp({"home", "admin"});
        mkdirp({"home", "guest"});
        mkdirp({"etc"});
        mkdirp({"var"});
        mkdirp({"var", "log"});
        mkdirp({"usr"});
        mkdirp({"usr", "bin"});
        mkdirp({"tmp"});
        mkdirp({"proc"});

        // Default files
        createFile({"etc"}, "hostname",  "ZainOS-Machine\n");
        createFile({"etc"}, "os-release",
            "NAME=\"ZainOS\"\nVERSION=\"2.0.0\"\nID=zainos\nPRETTY_NAME=\"ZainOS 2.0.0 LTS\"\n");
        createFile({"home", "admin"}, "README.txt",
            "Welcome to ZainOS!\nThis is your home directory.\n");
        createFile({"var", "log"}, "syslog",
            "[BOOT] Kernel loaded\n[BOOT] File system mounted\n[BOOT] Services started\n");

        // Start in /home/admin
        cwdPath = {"home", "admin"};
    }

    // Navigate from root following a path list and return node pointer (or nullptr)
    FSNode* navigate(const std::vector<std::string>& path) {
        FSNode* node = &root;
        for (auto& seg : path) {
            auto it = node->children.find(seg);
            if (it == node->children.end() || !it->second.isDir()) return nullptr;
            node = &it->second;
        }
        return node;
    }

    // Navigate to a specific file from a directory path
    FSNode* navigateFile(const std::vector<std::string>& dirPath, const std::string& filename) {
        FSNode* dir = navigate(dirPath);
        if (!dir) return nullptr;
        auto it = dir->children.find(filename);
        if (it == dir->children.end() || !it->second.isFile()) return nullptr;
        return &it->second;
    }

    FSNode* cwd() { return navigate(cwdPath); }

    std::string cwdString() const {
        if (cwdPath.empty()) return "/";
        std::string s;
        for (auto& p : cwdPath) s += "/" + p;
        return s;
    }

    // Create nested directories
    void mkdirp(const std::vector<std::string>& path) {
        FSNode* node = &root;
        for (auto& seg : path) {
            if (!node->children.count(seg)) {
                node->children[seg] = FSNode(FSNode::Type::DIR, seg);
            }
            node = &node->children[seg];
        }
    }

    // Create a file with content
    void createFile(const std::vector<std::string>& dirPath,
                    const std::string& name, const std::string& content = "") {
        FSNode* dir = navigate(dirPath);
        if (!dir) return;
        dir->children[name] = FSNode(FSNode::Type::FILE, name, content);
    }

    // Resolve a path string (absolute or relative) to a list of path components
    // Returns empty optional on failure
    std::optional<std::vector<std::string>> resolvePath(const std::string& raw) const {
        std::vector<std::string> result;
        if (raw.empty()) return cwdPath;

        bool absolute = (raw[0] == '/');
        if (!absolute) result = cwdPath;

        // Tokenize by '/'
        std::istringstream ss(raw);
        std::string seg;
        while (std::getline(ss, seg, '/')) {
            if (seg.empty() || seg == ".") continue;
            if (seg == "..") {
                if (!result.empty()) result.pop_back();
            } else {
                result.push_back(seg);
            }
        }
        return result;
    }
};

// ============================================================
//  SECTION 5 — User Management
// ============================================================

enum class UserRole { ADMIN, USER };

struct User {
    std::string username;
    std::string password;
    UserRole    role;
    bool        locked;
    std::time_t createdAt;

    User() : role(UserRole::USER), locked(false), createdAt(std::time(nullptr)) {}
    User(const std::string& u, const std::string& p, UserRole r = UserRole::USER)
        : username(u), password(p), role(r), locked(false), createdAt(std::time(nullptr)) {}

    bool isAdmin() const { return role == UserRole::ADMIN; }
    std::string roleStr() const { return role == UserRole::ADMIN ? "admin" : "user"; }
};

class UserManager {
public:
    std::map<std::string, User> users;
    std::string loggedInUser;

    UserManager() {
        // Default accounts
        users["admin"] = User("admin", "admin123", UserRole::ADMIN);
        users["guest"] = User("guest", "guest",    UserRole::USER);
    }

    User* current() {
        auto it = users.find(loggedInUser);
        return (it != users.end()) ? &it->second : nullptr;
    }

    bool isAdmin() const {
        auto it = users.find(loggedInUser);
        return it != users.end() && it->second.isAdmin();
    }

    // Attempt login; returns error string or empty on success
    std::string tryLogin(const std::string& u, const std::string& p) {
        auto it = users.find(u);
        if (it == users.end()) return "User not found.";
        if (it->second.locked)   return "Account is locked.";
        if (it->second.password != p) return "Incorrect password.";
        loggedInUser = u;
        return "";
    }

    bool addUser(const std::string& u, const std::string& p, UserRole r = UserRole::USER) {
        if (users.count(u)) return false;
        users[u] = User(u, p, r);
        return true;
    }

    bool delUser(const std::string& u) {
        if (u == "admin" || !users.count(u)) return false;
        users.erase(u);
        return true;
    }

    bool changePass(const std::string& u, const std::string& p) {
        auto it = users.find(u);
        if (it == users.end()) return false;
        it->second.password = p;
        return true;
    }

    bool setLock(const std::string& u, bool locked) {
        auto it = users.find(u);
        if (it == users.end() || u == "admin") return false;
        it->second.locked = locked;
        return true;
    }
};

// ============================================================
//  SECTION 6 — Process Management
// ============================================================

struct Process {
    int         pid;
    std::string name;
    std::string status;   // Running / Sleeping / Stopped / Zombie
    int         priority; // 0 (highest) to 19 (lowest)
    int         cpuTime;  // seconds of fake CPU time consumed
    int         memKB;    // fake memory usage
    std::string user;
    std::time_t startTime;

    Process(int p, const std::string& n, const std::string& u, int pr = 10)
        : pid(p), name(n), status("Running"), priority(pr),
          cpuTime(Utils::randInt(0, 3600)), memKB(Utils::randInt(512, 65536)),
          user(u), startTime(std::time(nullptr) - Utils::randInt(0, 7200)) {}
};

class ProcessManager {
    int nextPid = 1000;
public:
    std::vector<Process> procs;

    ProcessManager() {
        // Seed with fake kernel/system processes
        procs.push_back(Process(1,   "init",      "root", 0));
        procs.push_back(Process(2,   "kthreadd",  "root", 0));
        procs.push_back(Process(3,   "ksoftirqd", "root", 1));
        procs.push_back(Process(100, "syslogd",   "root", 5));
        procs.push_back(Process(101, "sshd",      "root", 5));
        procs.push_back(Process(200, "bash",      "admin",10));
        procs.push_back(Process(201, "zsh",       "admin",10));
        nextPid = 1000;
    }

    void spawnUserProcess(const std::string& user) {
        procs.push_back(Process(nextPid++, "zainos-shell", user, 10));
    }

    Process* find(int pid) {
        for (auto& p : procs) if (p.pid == pid) return &p;
        return nullptr;
    }

    bool kill(int pid) {
        auto it = std::remove_if(procs.begin(), procs.end(),
            [pid](const Process& p){ return p.pid == pid && p.pid > 1; });
        if (it == procs.end()) return false;
        procs.erase(it, procs.end());
        return true;
    }

    bool renice(int pid, int prio) {
        Process* p = find(pid);
        if (!p) return false;
        p->priority = std::clamp(prio, 0, 19);
        return true;
    }

    // Simulate CPU fluctuation
    void tick() {
        for (auto& p : procs) {
            p.cpuTime += Utils::randInt(0, 2);
            p.memKB   += Utils::randInt(-64, 128);
            if (p.memKB < 256) p.memKB = 256;
        }
    }
};

// ============================================================
//  SECTION 7 — CPU Scheduling
// ============================================================

struct SchedProcess {
    int id;
    std::string name;
    int arrival;
    int burst;
    int priority;   // lower = higher priority
    int remaining;  // for Round Robin
    int start = -1;
    int finish = 0;
    int waiting = 0;
    int turnaround = 0;
};

struct GanttSlot {
    int pid;
    std::string name;
    int start;
    int end;
};

class Scheduler {
public:
    // FCFS — First Come First Served
    static void fcfs(std::vector<SchedProcess> procs) {
        // Sort by arrival time
        std::sort(procs.begin(), procs.end(),
            [](const SchedProcess& a, const SchedProcess& b){ return a.arrival < b.arrival; });

        int time = 0;
        std::vector<GanttSlot> gantt;
        for (auto& p : procs) {
            if (time < p.arrival) time = p.arrival;
            p.start      = time;
            p.finish     = time + p.burst;
            p.turnaround = p.finish - p.arrival;
            p.waiting    = p.turnaround - p.burst;
            gantt.push_back({p.id, p.name, time, p.finish});
            time = p.finish;
        }
        printResults("FCFS — First Come First Served", procs, gantt);
    }

    // Priority (non-preemptive) — lower number = higher priority
    static void priority(std::vector<SchedProcess> procs) {
        int time = 0;
        std::vector<SchedProcess> done;
        std::vector<GanttSlot> gantt;

        while (!procs.empty()) {
            // Filter processes that have arrived
            std::vector<size_t> available;
            for (size_t i = 0; i < procs.size(); ++i)
                if (procs[i].arrival <= time) available.push_back(i);

            if (available.empty()) { ++time; continue; }

            // Pick highest priority (lowest number)
            size_t sel = *std::min_element(available.begin(), available.end(),
                [&](size_t a, size_t b){ return procs[a].priority < procs[b].priority; });

            auto& p   = procs[sel];
            p.start      = time;
            p.finish     = time + p.burst;
            p.turnaround = p.finish - p.arrival;
            p.waiting    = p.turnaround - p.burst;
            gantt.push_back({p.id, p.name, time, p.finish});
            time = p.finish;
            done.push_back(p);
            procs.erase(procs.begin() + sel);
        }
        printResults("Priority Scheduling (Non-Preemptive)", done, gantt);
    }

    // Round Robin
    static void roundRobin(std::vector<SchedProcess> procs, int quantum) {
        for (auto& p : procs) p.remaining = p.burst;
        std::sort(procs.begin(), procs.end(),
            [](const SchedProcess& a, const SchedProcess& b){ return a.arrival < b.arrival; });

        int time = 0;
        std::vector<GanttSlot> gantt;
        std::vector<int> queue;
        std::vector<bool> inQueue(procs.size(), false);
        std::vector<int>  finishTime(procs.size(), 0);

        // Enqueue processes that arrive at t=0
        for (size_t i = 0; i < procs.size(); ++i)
            if (procs[i].arrival <= time) { queue.push_back(i); inQueue[i] = true; }

        while (!queue.empty()) {
            int idx = queue.front(); queue.erase(queue.begin());
            auto& p = procs[idx];

            if (p.start == -1) p.start = time;
            int exec = std::min(quantum, p.remaining);
            gantt.push_back({p.id, p.name, time, time + exec});
            time         += exec;
            p.remaining  -= exec;

            // Enqueue newly arrived
            for (size_t i = 0; i < procs.size(); ++i) {
                if (!inQueue[i] && procs[i].arrival <= time && procs[i].remaining > 0) {
                    queue.push_back(i); inQueue[i] = true;
                }
            }

            if (p.remaining > 0) {
                queue.push_back(idx);
            } else {
                p.finish     = time;
                p.turnaround = p.finish - p.arrival;
                p.waiting    = p.turnaround - p.burst;
            }
        }
        printResults("Round Robin (Quantum = " + std::to_string(quantum) + ")", procs, gantt);
    }

private:
    // Print Gantt chart and result table
    static void printResults(const std::string& title,
                             const std::vector<SchedProcess>& procs,
                             const std::vector<GanttSlot>& gantt) {
        const auto& T = gTheme.get();
        std::cout << "\n";
        Utils::hline("═");
        Utils::printCentered("[ " + title + " ]", T.header);
        Utils::hline("═");

        // --- Gantt Chart ---
        std::cout << "\n" << CLR_HEAD << ANSI::BOLD << "  Gantt Chart:\n" << ANSI::RESET;
        std::cout << CLR_SEC << "  ";
        for (auto& s : gantt) {
            int width = s.end - s.start;
            if (width < 1) width = 1;
            std::cout << "┌";
            for (int i = 0; i < width * 3; ++i) std::cout << "─";
        }
        std::cout << "┐\n  ";
        for (auto& s : gantt) {
            int width = s.end - s.start;
            if (width < 1) width = 1;
            std::string lbl = "P" + std::to_string(s.pid);
            int pad = (width * 3 - (int)lbl.size()) / 2;
            std::cout << "│" << CLR_PRI << std::string(pad, ' ') << lbl
                      << std::string(width * 3 - pad - lbl.size(), ' ') << CLR_SEC;
        }
        std::cout << "│\n  ";
        for (auto& s : gantt) {
            int width = s.end - s.start;
            if (width < 1) width = 1;
            std::cout << "└";
            for (int i = 0; i < width * 3; ++i) std::cout << "─";
        }
        std::cout << "┘\n  ";
        int lastEnd = -1;
        for (auto& s : gantt) {
            int width = s.end - s.start;
            if (width < 1) width = 1;
            std::string t = std::to_string(s.start);
            std::cout << CLR_INFO << t << std::string(width * 3 + 1 - (int)t.size(), ' ');
            lastEnd = s.end;
        }
        if (lastEnd >= 0) std::cout << CLR_INFO << lastEnd;
        std::cout << ANSI::RESET << "\n\n";

        // --- Results Table ---
        std::cout << CLR_HEAD << ANSI::BOLD
                  << "  ┌─────┬────────────────┬─────────┬───────┬──────────┬─────────────┬─────────────┐\n"
                  << "  │ PID │ Name           │ Arrival │ Burst │ Priority │  Waiting T  │ Turnaround T│\n"
                  << "  ├─────┼────────────────┼─────────┼───────┼──────────┼─────────────┼─────────────┤\n"
                  << ANSI::RESET;

        double totalWT = 0, totalTT = 0;
        for (auto& p : procs) {
            totalWT += p.waiting;
            totalTT += p.turnaround;
            std::cout << CLR_SEC << "  │ " << CLR_PRI
                      << std::setw(3)  << p.id       << CLR_SEC << " │ " << CLR_PRI
                      << std::left << std::setw(14) << p.name << std::right << CLR_SEC << " │ " << CLR_PRI
                      << std::setw(7)  << p.arrival  << CLR_SEC << " │ " << CLR_PRI
                      << std::setw(5)  << p.burst    << CLR_SEC << " │ " << CLR_PRI
                      << std::setw(8)  << p.priority << CLR_SEC << " │ " << CLR_OK
                      << std::setw(11) << p.waiting  << CLR_SEC << " │ " << CLR_WARN
                      << std::setw(11) << p.turnaround << CLR_SEC << " │\n" << ANSI::RESET;
        }
        std::cout << CLR_HEAD << ANSI::BOLD
                  << "  └─────┴────────────────┴─────────┴───────┴──────────┴─────────────┴─────────────┘\n"
                  << ANSI::RESET;

        double n = static_cast<double>(procs.size());
        std::cout << "\n  " << CLR_INFO << "Average Waiting Time    : "
                  << CLR_OK << std::fixed << std::setprecision(2) << totalWT / n << " units\n";
        std::cout << "  " << CLR_INFO << "Average Turnaround Time : "
                  << CLR_WARN << std::fixed << std::setprecision(2) << totalTT / n << " units\n"
                  << ANSI::RESET << "\n";
    }
};

// ============================================================
//  SECTION 8 — Command History
// ============================================================

class CommandHistory {
    std::vector<std::string> history;
public:
    void add(const std::string& cmd) {
        if (!cmd.empty() && (history.empty() || history.back() != cmd))
            history.push_back(cmd);
    }
    void print() const {
        if (history.empty()) {
            std::cout << CLR_INFO << "  No commands in history.\n" << ANSI::RESET;
            return;
        }
        for (size_t i = 0; i < history.size(); ++i)
            std::cout << CLR_INFO << "  " << std::setw(4) << (i + 1)
                      << "  " << CLR_PRI << history[i] << "\n" << ANSI::RESET;
    }
    const std::vector<std::string>& get() const { return history; }
};

// ============================================================
//  SECTION 9 — Boot Sequence
// ============================================================

class BootSequence {
public:
    static void run() {
        ANSI::clearScreen();
        printBanner();
        Utils::sleep_ms(400);
        printFakeKernelLogs();
        Utils::sleep_ms(300);
        printProgressBars();
        Utils::sleep_ms(400);
        printSystemReady();
        Utils::sleep_ms(600);
    }

    private:
    static void printBanner() {
        const char* C = ANSI::FG_BRIGHT_CYAN;
        const char* Y = ANSI::FG_BRIGHT_YELLOW;
        const char* W = ANSI::FG_BRIGHT_WHITE;
        const char* G = ANSI::FG_BRIGHT_GREEN;

        std::cout << "\n\n";
        std::cout << C << "  ╔══════════════════════════════════════════════════════════════════════╗\n";
        std::cout << C << "  ║                                                                      ║\n";

        std::cout << C << "  ║  " << Y << ANSI::BOLD;
        std::cout << " ______     __     __     __   __     ______     ______              ";
        std::cout << C << "║\n";

        std::cout << C << "  ║  " << Y;
        std::cout << "/\\___  \\   /\\ \\   /\\ \\   /\\ '-.\\ \\   /\\  __ \\   /\\  ___\\             ";
        std::cout << C << "║\n";

        std::cout << C << "  ║  " << Y;
        std::cout << "\\/_/  /__  \\ \\ \\  \\ \\ \\  \\ \\ \\-.  \\  \\ \\ \\/\\ \\  \\ \\___  \\            ";
        std::cout << C << "║\n";

        std::cout << C << "  ║  " << Y;
        std::cout << "  /\\_____\\  \\ \\_\\  \\ \\_\\  \\ \\_\\\\'\\_\\  \\ \\_____\\  \\/\\_____\\           ";
        std::cout << C << "║\n";

        std::cout << C << "  ║  " << Y;
        std::cout << "  \\/_____/   \\/_/   \\/_/   \\/_/ \\/_/   \\/_____/   \\/_____/           ";
        std::cout << C << "║\n";

        std::cout << C << "  ║                                                                      ║\n";

        std::cout << C << "  ║  " << G 
                  << "  Advanced Operating System Simulator  v2.0.0 LTS               "
                  << C << "║\n";

        std::cout << C << "  ║  " << W 
                  << "  Kernel: ZainOS Kernel 6.1.0-LTS   Architecture: x86_64        "
                  << C << "║\n";

        std::cout << C << "  ╚══════════════════════════════════════════════════════════════════════╝\n";

        std::cout << ANSI::RESET << "\n";
    }

    static void printFakeKernelLogs() {
        struct LogEntry { std::string level; const char* color; std::string msg; int delay; };
        std::vector<LogEntry> logs = {
            {"OK",   ANSI::FG_BRIGHT_GREEN,  "BIOS POST checks completed",             80},
            {"OK",   ANSI::FG_BRIGHT_GREEN,  "Loading GRUB2 bootloader",               60},
            {"OK",   ANSI::FG_BRIGHT_GREEN,  "Decompressing ZainKernel 6.1.0 ...",     120},
            {"OK",   ANSI::FG_BRIGHT_GREEN,  "Initializing CPU: Intel Core i9 (sim)",  80},
            {"OK",   ANSI::FG_BRIGHT_GREEN,  "Detecting hardware: 16384 MB RAM found", 70},
            {"OK",   ANSI::FG_BRIGHT_GREEN,  "Mounting root file system [ext4]",       90},
            {"INFO", ANSI::FG_BRIGHT_CYAN,   "Starting udev device manager",           70},
            {"OK",   ANSI::FG_BRIGHT_GREEN,  "Loading kernel modules: net/sched/fs",   100},
            {"OK",   ANSI::FG_BRIGHT_GREEN,  "Initializing virtual memory subsystem",  80},
            {"OK",   ANSI::FG_BRIGHT_GREEN,  "Starting process scheduler (CFS)",       70},
            {"INFO", ANSI::FG_BRIGHT_CYAN,   "Loading network interfaces: lo, eth0",   70},
            {"WARN", ANSI::FG_BRIGHT_YELLOW, "eth0: link negotiation 1000Mbps (auto)", 60},
            {"OK",   ANSI::FG_BRIGHT_GREEN,  "Bringing up system services",            80},
            {"OK",   ANSI::FG_BRIGHT_GREEN,  "Starting ZainOS user-space daemons",     90},
            {"OK",   ANSI::FG_BRIGHT_GREEN,  "System initialization complete",         50},
        };

        std::cout << "\n";
        for (auto& log : logs) {
            std::cout << ANSI::FG_BRIGHT_BLACK << "  [" << log.color << std::setw(4) << log.level
                      << ANSI::FG_BRIGHT_BLACK << "] " << ANSI::FG_BRIGHT_WHITE << log.msg
                      << "\n" << ANSI::RESET;
            std::cout.flush();
            Utils::sleep_ms(log.delay);
        }
        std::cout << "\n";
    }

    static void printProgressBars() {
        struct BarEntry { std::string label; int steps; int delay; };
        std::vector<BarEntry> bars = {
            {"Initializing Kernel",    30, 30},
            {"Loading File System",    30, 25},
            {"Loading User Database",  30, 20},
            {"Starting Scheduler",     30, 20},
            {"Loading Shell",          30, 15},
        };
        for (auto& b : bars)
            Utils::progressBar(b.label, b.steps, b.delay);
        std::cout << "\n";
    }

    static void printSystemReady() {
        std::cout << "\n";
        Utils::hline("═");
        Utils::printCentered("★  SYSTEM READY  ★", ANSI::FG_BRIGHT_GREEN);
        Utils::hline("═");
        std::cout << "\n";
        Utils::sleep_ms(400);
    }
};

// ============================================================
//  SECTION 10 — Login System
// ============================================================

class LoginSystem {
public:
    // Returns the authenticated username or empty string on abort
    static std::string run(UserManager& um) {
        ANSI::clearScreen();
        printLoginBanner();

        constexpr int MAX_ATTEMPTS = 3;
        int attempts = 0;

        while (attempts < MAX_ATTEMPTS) {
            std::cout << CLR_SEC << "  ┌─────────────────────────────┐\n";
            std::cout << CLR_SEC << "  │" << CLR_HEAD << ANSI::BOLD
                      << "        ZainOS Login         " << ANSI::RESET << CLR_SEC << "│\n";
            std::cout << CLR_SEC << "  └─────────────────────────────┘\n" << ANSI::RESET;

            std::string username, password;
            std::cout << CLR_PRI << "\n  Username: " << ANSI::RESET;
            std::getline(std::cin, username);
            username = Utils::trim(username);

            std::cout << CLR_PRI << "  Password: " << ANSI::RESET;
            password = Utils::readPassword();
            password = Utils::trim(password);

            std::string err = um.tryLogin(username, password);
            if (err.empty()) {
                std::cout << "\n  " << CLR_OK << ANSI::BOLD << "✔ Login successful. Welcome, "
                          << username << "!" << ANSI::RESET << "\n";
                Utils::sleep_ms(700);
                return username;
            }

            ++attempts;
            std::cout << "\n  " << CLR_ERR << ANSI::BOLD << "✘ Access Denied: " << err;
            if (attempts < MAX_ATTEMPTS)
                std::cout << " (" << (MAX_ATTEMPTS - attempts) << " attempt(s) remaining)";
            std::cout << ANSI::RESET << "\n\n";
            Utils::sleep_ms(600);
        }

        std::cout << CLR_ERR << ANSI::BOLD
                  << "\n  [SECURITY] Too many failed attempts. System locked.\n"
                  << ANSI::RESET;
        return "";
    }

private:
    static void printLoginBanner() {
        std::cout << "\n";
        Utils::hline("═");
        Utils::printCentered("ZainOS 2.0.0 LTS", ANSI::FG_BRIGHT_CYAN);
        Utils::printCentered("Kernel 6.1.0-zainos  |  " + Utils::currentDate(), ANSI::FG_BRIGHT_BLACK);
        Utils::hline("═");
        std::cout << "\n";
    }
};

// ============================================================
//  SECTION 11 — Shell / Command Interpreter
// ============================================================

class Shell {
    UserManager&    um;
    VirtualFileSystem& fs;
    ProcessManager& pm;
    CommandHistory  hist;
    bool            running = true;
    std::chrono::steady_clock::time_point startTime;

    // Fake system stats
    int fakeRamTotal  = 16384; // MB
    int fakeRamUsed   = 4096 + Utils::randInt(0, 2048);
    int fakeCpuPct    = Utils::randInt(5, 35);
    std::string kernelVer = "6.1.0-zainos";

public:
    Shell(UserManager& u, VirtualFileSystem& f, ProcessManager& p)
        : um(u), fs(f), pm(p), startTime(std::chrono::steady_clock::now()) {
        pm.spawnUserProcess(u.loggedInUser);
    }

    void run() {
        ANSI::clearScreen();
        printMotd();
        while (running) {
            std::string line = prompt();
            if (line.empty()) continue;
            hist.add(line);
            dispatch(line);
        }
    }

private:
    // Print Message Of The Day
    void printMotd() {
        const auto& T = gTheme.get();
        std::cout << "\n";
        Utils::hline();
        Utils::printCentered("Welcome to ZainOS 2.0.0 LTS", T.header);
        Utils::printCentered("Type 'help' for a list of commands.", T.info);
        Utils::hline();
        std::cout << "\n  " << CLR_INFO << "Date : " << CLR_PRI << Utils::currentDate() << "\n";
        std::cout << "  " << CLR_INFO << "Time : " << CLR_PRI << Utils::currentTime() << "\n";
        std::cout << "  " << CLR_INFO << "User : " << CLR_PRI << um.loggedInUser
                  << CLR_INFO << "  (" << um.current()->roleStr() << ")\n";
        std::cout << "\n" << ANSI::RESET;
    }

    // Display prompt and read input
    std::string prompt() {
        // Update fake stats periodically
        fakeCpuPct = Utils::randInt(2, 45);
        fakeRamUsed += Utils::randInt(-32, 64);
        fakeRamUsed = std::clamp(fakeRamUsed, 1024, fakeRamTotal - 512);

        const std::string user = um.loggedInUser;
        const std::string path = fs.cwdString();
        const char* roleColor  = um.isAdmin() ? ANSI::FG_BRIGHT_RED : ANSI::FG_BRIGHT_GREEN;

        std::cout << roleColor << ANSI::BOLD << user << ANSI::RESET
                  << CLR_INFO << "@"
                  << CLR_SEC << ANSI::BOLD << "ZainOS" << ANSI::RESET
                  << CLR_INFO << ":"
                  << CLR_HI << ANSI::BOLD << path << ANSI::RESET
                  << CLR_PRI << "$ " << ANSI::RESET;
        std::cout.flush();

        std::string line;
        if (!std::getline(std::cin, line)) {
            running = false;
            return "";
        }
        return Utils::trim(line);
    }

    void dispatch(const std::string& line) {
        auto tokens = Utils::tokenize(line);
        if (tokens.empty()) return;
        const std::string& cmd = tokens[0];

        // Map of command → handler lambda
        using Handler = std::function<void(const std::vector<std::string>&)>;
        std::map<std::string, Handler> cmds;

        // ── System ──────────────────────────────────────
        cmds["help"]       = [&](auto& a){ cmdHelp(a); };
        cmds["clear"]      = [&](auto&  ){ ANSI::clearScreen(); };
        cmds["exit"]       = [&](auto& a){ cmdExit(a); };
        cmds["logout"]     = [&](auto& a){ cmdExit(a); };
        cmds["date"]       = [&](auto&  ){ std::cout << "  " << CLR_PRI << Utils::currentDate() << "\n" << ANSI::RESET; };
        cmds["time"]       = [&](auto&  ){ std::cout << "  " << CLR_PRI << Utils::currentTime() << "\n" << ANSI::RESET; };
        cmds["sysinfo"]    = [&](auto&  ){ cmdSysinfo(); };
        cmds["uptime"]     = [&](auto&  ){ cmdUptime(); };
        cmds["echo"]       = [&](auto& a){ cmdEcho(a); };
        cmds["theme"]      = [&](auto& a){ cmdTheme(a); };
        cmds["history"]    = [&](auto&  ){ hist.print(); };

        // ── File system ──────────────────────────────────
        cmds["ls"]         = [&](auto& a){ cmdLs(a); };
        cmds["pwd"]        = [&](auto&  ){ std::cout << "  " << CLR_PRI << fs.cwdString() << "\n" << ANSI::RESET; };
        cmds["cd"]         = [&](auto& a){ cmdCd(a); };
        cmds["mkdir"]      = [&](auto& a){ cmdMkdir(a); };
        cmds["touch"]      = [&](auto& a){ cmdTouch(a); };
        cmds["rm"]         = [&](auto& a){ cmdRm(a); };
        cmds["cat"]        = [&](auto& a){ cmdCat(a); };
        cmds["cp"]         = [&](auto& a){ cmdCp(a); };
        cmds["mv"]         = [&](auto& a){ cmdMv(a); };

        // ── User management ─────────────────────────────
        cmds["users"]      = [&](auto&  ){ cmdUsers(); };
        cmds["adduser"]    = [&](auto& a){ cmdAdduser(a); };
        cmds["deluser"]    = [&](auto& a){ cmdDeluser(a); };
        cmds["passwd"]     = [&](auto& a){ cmdPasswd(a); };
        cmds["lockuser"]   = [&](auto& a){ cmdLockUser(a, true); };
        cmds["unlockuser"] = [&](auto& a){ cmdLockUser(a, false); };

        // ── Process management ──────────────────────────
        cmds["ps"]         = [&](auto&  ){ cmdPs(); };
        cmds["top"]        = [&](auto&  ){ cmdTop(); };
        cmds["kill"]       = [&](auto& a){ cmdKill(a); };
        cmds["renice"]     = [&](auto& a){ cmdRenice(a); };

        // ── Scheduling ──────────────────────────────────
        cmds["fcfs"]       = [&](auto&  ){ cmdScheduler("fcfs"); };
        cmds["priority"]   = [&](auto&  ){ cmdScheduler("priority"); };
        cmds["roundrobin"] = [&](auto&  ){ cmdScheduler("roundrobin"); };

        auto it = cmds.find(cmd);
        if (it != cmds.end()) {
            it->second(tokens);
        } else {
            std::cout << CLR_ERR << "  zainos: command not found: " << CLR_PRI << cmd
                      << CLR_INFO << "  (type 'help' for commands)\n" << ANSI::RESET;
        }
    }

    // ── Command Implementations ─────────────────────────

    void cmdHelp(const std::vector<std::string>&) {
        const char* H = CLR_HEAD;
        const char* S = CLR_SEC;
        const char* P = CLR_PRI;
        const char* I = CLR_INFO;
        std::cout << "\n";
        Utils::hline("═");
        Utils::printCentered("ZainOS Command Reference", H);
        Utils::hline("═");

        auto section = [&](const std::string& title){
            std::cout << "\n" << H << ANSI::BOLD << "  ── " << title << " ──\n" << ANSI::RESET;
        };
        auto entry = [&](const std::string& cmd, const std::string& desc){
            std::cout << "  " << S << std::left << std::setw(16) << cmd
                      << P << desc << "\n" << ANSI::RESET;
        };

        section("System");
        entry("help",        "Show this help screen");
        entry("clear",       "Clear the terminal");
        entry("exit / logout","Exit the shell");
        entry("date",        "Show current date");
        entry("time",        "Show current time");
        entry("sysinfo",     "Show system information");
        entry("uptime",      "Show system uptime");
        entry("echo <text>", "Print text to terminal");
        entry("theme <name>","Set terminal theme (dark|hacker|blue|red)");
        entry("history",     "Show command history");

        section("File System");
        entry("ls [path]",     "List directory contents");
        entry("pwd",           "Print working directory");
        entry("cd <path>",     "Change directory");
        entry("mkdir <name>",  "Create directory");
        entry("touch <name>",  "Create empty file");
        entry("rm <name>",     "Remove file or directory");
        entry("cat <file>",    "Display file contents");
        entry("cp <src> <dst>","Copy file");
        entry("mv <src> <dst>","Move/rename file");

        section("User Management");
        entry("users",           "List all users");
        entry("adduser <user>",  "Add a new user");
        entry("deluser <user>",  "Delete a user");
        entry("passwd [user]",   "Change password");
        entry("lockuser <user>", "Lock a user account");
        entry("unlockuser <u>",  "Unlock a user account");

        section("Process Management");
        entry("ps",               "List running processes");
        entry("top",              "Live process monitor");
        entry("kill <pid>",       "Terminate a process");
        entry("renice <pid> <n>", "Change process priority");

        section("CPU Scheduling");
        entry("fcfs",       "Run FCFS scheduling simulation");
        entry("priority",   "Run Priority scheduling simulation");
        entry("roundrobin", "Run Round-Robin scheduling simulation");

        Utils::hline();
        std::cout << "  " << I << "Hint: Commands are case-sensitive.\n\n" << ANSI::RESET;
    }

    void cmdExit(const std::vector<std::string>&) {
        std::cout << "\n  " << CLR_OK << "Goodbye, " << um.loggedInUser << ". Session terminated.\n\n" << ANSI::RESET;
        running = false;
    }

    void cmdSysinfo() {
        const auto now = std::chrono::system_clock::now();
        std::time_t t  = std::chrono::system_clock::to_time_t(now);
        char tbuf[64];
        std::strftime(tbuf, sizeof(tbuf), "%Y-%m-%d %H:%M:%S", std::localtime(&t));

        int ramPct = (fakeRamUsed * 100) / fakeRamTotal;

        std::cout << "\n";
        Utils::hline("─");
        Utils::printCentered("System Information", CLR_HEAD);
        Utils::hline("─");

        auto info = [](const std::string& label, const std::string& val) {
            std::cout << "  " << CLR_INFO << std::left << std::setw(22) << label
                      << CLR_PRI << val << "\n" << ANSI::RESET;
        };

        info("OS Name:",       "ZainOS 2.0.0 LTS");
        info("Kernel:",        kernelVer + " (x86_64)");
        info("Hostname:",      "ZainOS-Machine");
        info("Date/Time:",     std::string(tbuf));
        info("Uptime:",        uptimeStr());
        info("User:",          um.loggedInUser + " (" + um.current()->roleStr() + ")");
        info("Shell:",         "/bin/zainos-shell");
        info("CPU:",           "Intel(R) Core(TM) i9-14900K @ 3.20GHz (sim)");
        info("CPU Usage:",     std::to_string(fakeCpuPct) + "%");
        info("Total RAM:",     std::to_string(fakeRamTotal) + " MB");
        info("Used RAM:",      std::to_string(fakeRamUsed) + " MB (" + std::to_string(ramPct) + "%)");
        info("Free RAM:",      std::to_string(fakeRamTotal - fakeRamUsed) + " MB");
        info("Disk (/):",      "128 GB  [used: 42 GB, free: 86 GB]");
        info("Architecture:",  "x86_64");
        info("Terminal:",      "ZainTerm 1.0");
        info("Theme:",         gTheme.current);

        std::cout << "\n";
    }

    std::string uptimeStr() const {
        auto dur = std::chrono::steady_clock::now() - startTime;
        auto sec = std::chrono::duration_cast<std::chrono::seconds>(dur).count();
        long h = sec / 3600; sec %= 3600;
        long m = sec / 60;   sec %= 60;
        return std::to_string(h) + "h " + std::to_string(m) + "m " + std::to_string(sec) + "s";
    }

    void cmdUptime() {
        std::cout << "  " << CLR_INFO << "Uptime: " << CLR_PRI << uptimeStr() << "\n" << ANSI::RESET;
    }

    void cmdEcho(const std::vector<std::string>& args) {
        for (size_t i = 1; i < args.size(); ++i)
            std::cout << args[i] << (i + 1 < args.size() ? " " : "");
        std::cout << "\n";
    }

    void cmdTheme(const std::vector<std::string>& args) {
        if (args.size() < 2) {
            std::cout << CLR_INFO << "  Available themes: dark, hacker, blue, red\n"
                      << "  Current: " << CLR_PRI << gTheme.current << "\n" << ANSI::RESET;
            return;
        }
        if (gTheme.setTheme(args[1]))
            std::cout << CLR_OK << "  Theme set to '" << args[1] << "'\n" << ANSI::RESET;
        else
            std::cout << CLR_ERR << "  Unknown theme: " << args[1] << "\n" << ANSI::RESET;
    }

    // ── File System Commands ─────────────────────────────

    void cmdLs(const std::vector<std::string>& args) {
        FSNode* dir = fs.cwd();
        if (args.size() > 1) {
            auto rp = fs.resolvePath(args[1]);
            if (!rp) { std::cout << CLR_ERR << "  ls: cannot access '" << args[1] << "': No such file or directory\n" << ANSI::RESET; return; }
            dir = fs.navigate(*rp);
            if (!dir) { std::cout << CLR_ERR << "  ls: cannot access '" << args[1] << "': No such file or directory\n" << ANSI::RESET; return; }
        }

        if (dir->children.empty()) {
            std::cout << CLR_INFO << "  (empty directory)\n" << ANSI::RESET;
            return;
        }

        std::cout << "\n";
        int col = 0;
        for (auto& [name, node] : dir->children) {
            if (node.isDir()) {
                std::cout << CLR_HI << ANSI::BOLD << std::left << std::setw(20) << (name + "/");
            } else {
                std::cout << CLR_PRI << std::setw(20) << name;
            }
            std::cout << ANSI::RESET;
            if (++col % 4 == 0) std::cout << "\n";
        }
        if (col % 4 != 0) std::cout << "\n";
        std::cout << "\n  " << CLR_INFO << dir->children.size() << " item(s)\n" << ANSI::RESET;
    }

    void cmdCd(const std::vector<std::string>& args) {
        std::string target = (args.size() > 1) ? args[1] : "/home/" + um.loggedInUser;
        if (target == "~") target = "/home/" + um.loggedInUser;

        auto rp = fs.resolvePath(target);
        if (!rp) { std::cout << CLR_ERR << "  cd: '" << target << "': No such file or directory\n" << ANSI::RESET; return; }
        FSNode* node = fs.navigate(*rp);
        if (!node || !node->isDir()) {
            std::cout << CLR_ERR << "  cd: '" << target << "': Not a directory\n" << ANSI::RESET;
            return;
        }
        fs.cwdPath = *rp;
    }

    void cmdMkdir(const std::vector<std::string>& args) {
        if (args.size() < 2) { std::cout << CLR_ERR << "  Usage: mkdir <name>\n" << ANSI::RESET; return; }
        FSNode* dir = fs.cwd();
        const std::string& name = args[1];
        if (dir->children.count(name)) {
            std::cout << CLR_ERR << "  mkdir: cannot create directory '" << name << "': File exists\n" << ANSI::RESET;
            return;
        }
        dir->children[name] = FSNode(FSNode::Type::DIR, name);
        std::cout << CLR_OK << "  Directory '" << name << "' created.\n" << ANSI::RESET;
    }

    void cmdTouch(const std::vector<std::string>& args) {
        if (args.size() < 2) { std::cout << CLR_ERR << "  Usage: touch <filename>\n" << ANSI::RESET; return; }
        FSNode* dir = fs.cwd();
        const std::string& name = args[1];
        if (!dir->children.count(name))
            dir->children[name] = FSNode(FSNode::Type::FILE, name, "");
        // If it exists, just update timestamp (nothing to do in our model)
        std::cout << CLR_OK << "  File '" << name << "' created/updated.\n" << ANSI::RESET;
    }

    void cmdRm(const std::vector<std::string>& args) {
        if (args.size() < 2) { std::cout << CLR_ERR << "  Usage: rm <name>\n" << ANSI::RESET; return; }
        FSNode* dir = fs.cwd();
        const std::string& name = args[1];
        auto it = dir->children.find(name);
        if (it == dir->children.end()) {
            std::cout << CLR_ERR << "  rm: cannot remove '" << name << "': No such file or directory\n" << ANSI::RESET;
            return;
        }
        dir->children.erase(it);
        std::cout << CLR_OK << "  Removed '" << name << "'.\n" << ANSI::RESET;
    }

    void cmdCat(const std::vector<std::string>& args) {
        if (args.size() < 2) { std::cout << CLR_ERR << "  Usage: cat <file>\n" << ANSI::RESET; return; }
        FSNode* dir = fs.cwd();
        auto it = dir->children.find(args[1]);
        if (it == dir->children.end() || it->second.isDir()) {
            std::cout << CLR_ERR << "  cat: " << args[1] << ": No such file\n" << ANSI::RESET;
            return;
        }
        const std::string& content = it->second.content;
        if (content.empty()) std::cout << CLR_INFO << "  (empty file)\n" << ANSI::RESET;
        else                 std::cout << CLR_PRI << content << ANSI::RESET;
    }

    void cmdCp(const std::vector<std::string>& args) {
        if (args.size() < 3) { std::cout << CLR_ERR << "  Usage: cp <src> <dst>\n" << ANSI::RESET; return; }
        FSNode* dir = fs.cwd();
        auto it = dir->children.find(args[1]);
        if (it == dir->children.end() || it->second.isDir()) {
            std::cout << CLR_ERR << "  cp: '" << args[1] << "': No such file\n" << ANSI::RESET;
            return;
        }
        dir->children[args[2]] = FSNode(FSNode::Type::FILE, args[2], it->second.content);
        std::cout << CLR_OK << "  Copied '" << args[1] << "' → '" << args[2] << "'\n" << ANSI::RESET;
    }

    void cmdMv(const std::vector<std::string>& args) {
        if (args.size() < 3) { std::cout << CLR_ERR << "  Usage: mv <src> <dst>\n" << ANSI::RESET; return; }
        FSNode* dir = fs.cwd();
        auto it = dir->children.find(args[1]);
        if (it == dir->children.end()) {
            std::cout << CLR_ERR << "  mv: '" << args[1] << "': No such file or directory\n" << ANSI::RESET;
            return;
        }
        FSNode moved = it->second;
        moved.name = args[2];
        dir->children.erase(it);
        dir->children[args[2]] = std::move(moved);
        std::cout << CLR_OK << "  Moved/renamed '" << args[1] << "' → '" << args[2] << "'\n" << ANSI::RESET;
    }

    // ── User Management Commands ──────────────────────────

    void cmdUsers() {
        std::cout << "\n";
        Utils::hline("─");
        Utils::printCentered("User Accounts", CLR_HEAD);
        Utils::hline("─");
        std::cout << CLR_SEC << "  " << std::left
                  << std::setw(16) << "Username"
                  << std::setw(10) << "Role"
                  << std::setw(10) << "Status" << "\n";
        Utils::hline("─");
        for (auto& [name, u] : um.users) {
            const char* statColor = u.locked ? CLR_ERR : CLR_OK;
            const char* roleColor = u.isAdmin() ? CLR_WARN : CLR_PRI;
            std::cout << "  " << CLR_PRI << std::left << std::setw(16) << name
                      << roleColor  << std::setw(10) << u.roleStr()
                      << statColor  << (u.locked ? "Locked" : "Active")
                      << "\n" << ANSI::RESET;
        }
        std::cout << "\n";
    }

    void cmdAdduser(const std::vector<std::string>& args) {
        if (!um.isAdmin()) { std::cout << CLR_ERR << "  adduser: Permission denied.\n" << ANSI::RESET; return; }
        if (args.size() < 2) { std::cout << CLR_ERR << "  Usage: adduser <username>\n" << ANSI::RESET; return; }
        const std::string& uname = args[1];
        std::cout << CLR_PRI << "  Password for " << uname << ": " << ANSI::RESET;
        std::string pw = Utils::readPassword();
        if (um.addUser(uname, pw))
            std::cout << CLR_OK << "  User '" << uname << "' created.\n" << ANSI::RESET;
        else
            std::cout << CLR_ERR << "  adduser: user '" << uname << "' already exists.\n" << ANSI::RESET;
    }

    void cmdDeluser(const std::vector<std::string>& args) {
        if (!um.isAdmin()) { std::cout << CLR_ERR << "  deluser: Permission denied.\n" << ANSI::RESET; return; }
        if (args.size() < 2) { std::cout << CLR_ERR << "  Usage: deluser <username>\n" << ANSI::RESET; return; }
        if (um.delUser(args[1]))
            std::cout << CLR_OK << "  User '" << args[1] << "' deleted.\n" << ANSI::RESET;
        else
            std::cout << CLR_ERR << "  deluser: Cannot delete '" << args[1] << "' (not found or protected).\n" << ANSI::RESET;
    }

    void cmdPasswd(const std::vector<std::string>& args) {
        std::string target = (args.size() > 1 && um.isAdmin()) ? args[1] : um.loggedInUser;

        if (args.size() > 1 && !um.isAdmin() && args[1] != um.loggedInUser) {
            std::cout << CLR_ERR << "  passwd: Permission denied.\n" << ANSI::RESET; return;
        }

        std::cout << CLR_PRI << "  New password for " << target << ": " << ANSI::RESET;
        std::string pw1 = Utils::readPassword();
        std::cout << CLR_PRI << "  Confirm password: " << ANSI::RESET;
        std::string pw2 = Utils::readPassword();

        if (pw1 != pw2) {
            std::cout << CLR_ERR << "  passwd: Passwords do not match.\n" << ANSI::RESET;
            return;
        }
        if (um.changePass(target, pw1))
            std::cout << CLR_OK << "  Password updated for '" << target << "'.\n" << ANSI::RESET;
        else
            std::cout << CLR_ERR << "  passwd: User not found.\n" << ANSI::RESET;
    }

    void cmdLockUser(const std::vector<std::string>& args, bool lock) {
        if (!um.isAdmin()) { std::cout << CLR_ERR << "  Permission denied.\n" << ANSI::RESET; return; }
        if (args.size() < 2) { std::cout << CLR_ERR << "  Usage: " << args[0] << " <username>\n" << ANSI::RESET; return; }
        if (um.setLock(args[1], lock))
            std::cout << CLR_OK << "  User '" << args[1] << "' " << (lock ? "locked" : "unlocked") << ".\n" << ANSI::RESET;
        else
            std::cout << CLR_ERR << "  Cannot lock/unlock '" << args[1] << "' (not found or protected).\n" << ANSI::RESET;
    }

    // ── Process Commands ─────────────────────────────────

    void cmdPs() {
        pm.tick();
        std::cout << "\n";
        Utils::hline("─");
        Utils::printCentered("Process List", CLR_HEAD);
        Utils::hline("─");
        std::cout << CLR_SEC << "  " << std::left
                  << std::setw(7)  << "PID"
                  << std::setw(20) << "NAME"
                  << std::setw(10) << "USER"
                  << std::setw(9)  << "STATUS"
                  << std::setw(5)  << "PRI"
                  << std::setw(10) << "CPU(s)"
                  << std::setw(10) << "MEM(KB)"
                  << "\n" << ANSI::RESET;
        Utils::hline("─");
        for (auto& p : pm.procs) {
            const char* sc = (p.status == "Running") ? CLR_OK :
                             (p.status == "Sleeping") ? CLR_INFO : CLR_WARN;
            std::cout << "  " << CLR_PRI << std::setw(7)  << p.pid
                      << CLR_HI         << std::setw(20) << p.name
                      << CLR_INFO       << std::setw(10) << p.user
                      << sc             << std::setw(9)  << p.status
                      << CLR_WARN       << std::setw(5)  << p.priority
                      << CLR_PRI        << std::setw(10) << p.cpuTime
                      << CLR_INFO       << std::setw(10) << p.memKB
                      << "\n" << ANSI::RESET;
        }
        std::cout << "\n  " << CLR_INFO << pm.procs.size() << " process(es) running.\n" << ANSI::RESET;
    }

    void cmdTop() {
        std::cout << "\n  " << CLR_INFO << "[top] Showing live snapshot (press Enter to refresh, type 'q' + Enter to quit)\n\n" << ANSI::RESET;
        std::string input;
        while (true) {
            pm.tick();
            // Simulate CPU
            fakeCpuPct = Utils::randInt(5, 60);
            fakeRamUsed += Utils::randInt(-64, 128);
            fakeRamUsed = std::clamp(fakeRamUsed, 1024, fakeRamTotal - 512);

            ANSI::clearScreen();
            std::cout << CLR_HEAD << ANSI::BOLD;
            Utils::hline("═");
            Utils::printCentered("ZainOS top — " + Utils::currentTime(), CLR_HEAD);

            // Mini stats bar
            int ramPct = (fakeRamUsed * 100) / fakeRamTotal;
            std::cout << "\n  " << CLR_INFO << "CPU: " << CLR_OK;
            int cpuBar = fakeCpuPct / 5;
            std::cout << "[";
            for (int i = 0; i < 20; ++i) std::cout << (i < cpuBar ? "█" : "░");
            std::cout << "] " << fakeCpuPct << "%";

            std::cout << "    " << CLR_INFO << "RAM: " << CLR_WARN;
            int ramBar = ramPct / 5;
            std::cout << "[";
            for (int i = 0; i < 20; ++i) std::cout << (i < ramBar ? "█" : "░");
            std::cout << "] " << ramPct << "% (" << fakeRamUsed << "/" << fakeRamTotal << " MB)\n\n" << ANSI::RESET;

            Utils::hline("─");
            std::cout << CLR_SEC << "  " << std::left
                      << std::setw(7)  << "PID"
                      << std::setw(20) << "NAME"
                      << std::setw(10) << "USER"
                      << std::setw(9)  << "STATUS"
                      << std::setw(5)  << "PRI"
                      << std::setw(10) << "CPU(s)"
                      << std::setw(10) << "MEM(KB)"
                      << "\n" << ANSI::RESET;
            Utils::hline("─");

            // Sort by cpuTime descending
            auto sorted = pm.procs;
            std::sort(sorted.begin(), sorted.end(),
                [](const Process& a, const Process& b){ return a.cpuTime > b.cpuTime; });

            for (auto& p : sorted) {
                const char* sc = (p.status == "Running") ? CLR_OK : CLR_INFO;
                std::cout << "  " << CLR_PRI << std::setw(7)  << p.pid
                          << CLR_HI         << std::setw(20) << p.name
                          << CLR_INFO       << std::setw(10) << p.user
                          << sc             << std::setw(9)  << p.status
                          << CLR_WARN       << std::setw(5)  << p.priority
                          << CLR_PRI        << std::setw(10) << p.cpuTime
                          << CLR_INFO       << std::setw(10) << p.memKB
                          << "\n" << ANSI::RESET;
            }

            Utils::hline("─");
            std::cout << "\n  " << CLR_INFO << "Uptime: " << CLR_PRI << uptimeStr()
                      << CLR_INFO << "   Tasks: " << CLR_PRI << pm.procs.size()
                      << "\n\n  " << CLR_SEC << "[ Enter = refresh | q = quit ] " << ANSI::RESET;
            std::cout.flush();

            std::getline(std::cin, input);
            if (input == "q" || input == "Q") break;
            Utils::sleep_ms(100);
        }
        ANSI::clearScreen();
    }

    void cmdKill(const std::vector<std::string>& args) {
        if (args.size() < 2) { std::cout << CLR_ERR << "  Usage: kill <pid>\n" << ANSI::RESET; return; }
        try {
            int pid = std::stoi(args[1]);
            if (pm.kill(pid))
                std::cout << CLR_OK << "  Process " << pid << " terminated.\n" << ANSI::RESET;
            else
                std::cout << CLR_ERR << "  kill: (" << pid << ") - No such process or cannot kill.\n" << ANSI::RESET;
        } catch (...) {
            std::cout << CLR_ERR << "  kill: invalid PID\n" << ANSI::RESET;
        }
    }

    void cmdRenice(const std::vector<std::string>& args) {
        if (args.size() < 3) { std::cout << CLR_ERR << "  Usage: renice <pid> <priority 0-19>\n" << ANSI::RESET; return; }
        try {
            int pid  = std::stoi(args[1]);
            int prio = std::stoi(args[2]);
            if (pm.renice(pid, prio))
                std::cout << CLR_OK << "  Process " << pid << " priority set to " << prio << ".\n" << ANSI::RESET;
            else
                std::cout << CLR_ERR << "  renice: No such process: " << pid << "\n" << ANSI::RESET;
        } catch (...) {
            std::cout << CLR_ERR << "  renice: invalid arguments\n" << ANSI::RESET;
        }
    }

    // ── Scheduling Commands ───────────────────────────────

    void cmdScheduler(const std::string& algo) {
        std::cout << "\n  " << CLR_HEAD << ANSI::BOLD << "CPU Scheduling Simulator\n" << ANSI::RESET;
        std::cout << "  " << CLR_INFO << "Enter the number of processes: " << ANSI::RESET;
        int n = 0;
        std::string line;
        std::getline(std::cin, line);
        try { n = std::stoi(line); } catch (...) { n = 0; }
        if (n <= 0 || n > 20) {
            std::cout << CLR_ERR << "  Invalid number of processes (1-20).\n" << ANSI::RESET;
            return;
        }

        int quantum = 2;
        if (algo == "roundrobin") {
            std::cout << "  " << CLR_INFO << "Enter time quantum: " << ANSI::RESET;
            std::getline(std::cin, line);
            try { quantum = std::stoi(line); } catch (...) { quantum = 2; }
            if (quantum < 1) quantum = 1;
        }

        std::vector<SchedProcess> procs;
        for (int i = 0; i < n; ++i) {
            SchedProcess p;
            p.id = i + 1;
            std::cout << "\n  " << CLR_SEC << "─── Process P" << p.id << " ───\n" << ANSI::RESET;

            std::cout << "    " << CLR_PRI << "Name       : " << ANSI::RESET;
            std::getline(std::cin, p.name);
            if (p.name.empty()) p.name = "P" + std::to_string(p.id);

            std::cout << "    " << CLR_PRI << "Arrival    : " << ANSI::RESET;
            std::getline(std::cin, line);
            try { p.arrival = std::stoi(line); } catch (...) { p.arrival = 0; }

            std::cout << "    " << CLR_PRI << "Burst Time : " << ANSI::RESET;
            std::getline(std::cin, line);
            try { p.burst = std::stoi(line); } catch (...) { p.burst = 1; }
            if (p.burst < 1) p.burst = 1;

            if (algo == "priority") {
                std::cout << "    " << CLR_PRI << "Priority   : " << ANSI::RESET;
                std::getline(std::cin, line);
                try { p.priority = std::stoi(line); } catch (...) { p.priority = 5; }
            } else {
                p.priority = i + 1;
            }

            p.remaining = p.burst;
            procs.push_back(p);
        }

        if (algo == "fcfs")       Scheduler::fcfs(procs);
        else if (algo == "priority")   Scheduler::priority(procs);
        else if (algo == "roundrobin") Scheduler::roundRobin(procs, quantum);
    }
};

// ============================================================
//  SECTION 12 — Main Entry Point
// ============================================================

int main() {

#ifdef _WIN32
    // Enable UTF-8 and ANSI colors on Windows
    system("chcp 65001 > nul");

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    DWORD dwMode = 0;

    GetConsoleMode(hOut, &dwMode);

    SetConsoleMode(
        hOut,
        dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING
    );
#endif

    // ==============================
    // Boot Sequence
    // ==============================

    BootSequence::run();

    // ==============================
    // Initialize Core Subsystems
    // ==============================

    UserManager userManager;

    VirtualFileSystem vfs;

    ProcessManager procManager;

    // ==============================
    // Main Login Loop
    // ==============================

    while (true) {

        std::string currentUser =
            LoginSystem::run(userManager);

        // Exit if login fails completely
        // or EOF occurs
        if (currentUser.empty()) {

            std::cout
                << ANSI::FG_BRIGHT_RED
                << "\n  Exiting ZainOS...\n"
                << ANSI::RESET;

            break;
        }

        // ==============================
        // Start Shell Session
        // ==============================

        Shell shell(
            userManager,
            vfs,
            procManager
        );

        shell.run();

        // ==============================
        // Return To Login Screen
        // ==============================

        std::cout
            << "\n  "
            << CLR_INFO
            << "Press ENTER to return to login"
            << ANSI::RESET;

        std::string temp;

        if (!std::getline(std::cin, temp)) {
            break;
        }

        // Clear screen before relogin
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
    }

    // ==============================
    // Shutdown Message
    // ==============================

    std::cout
        << ANSI::FG_BRIGHT_CYAN
        << "\n========================================\n"
        << "        ZainOS Shutdown Complete\n"
        << "========================================\n"
        << ANSI::RESET;

    return 0;
}