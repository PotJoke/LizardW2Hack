#include "lizard_log.h"

#include <cstdio>
#include <cstring>
#include <deque>
#include <mutex>
#include <string>

static std::mutex g_sessionMutex;
static std::deque<std::string> g_sessionLines;
static constexpr size_t kMaxSessionLines = 4000;

static FILE* g_sessionFile = nullptr;

HANDLE g_console = nullptr;
WORD g_colorInfo = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY;

static void FormatSessionTimestampPrefix(char* out, size_t outSize) {
    if (!out || outSize < 28) return;
    SYSTEMTIME st{};
    GetLocalTime(&st);
    snprintf(out, outSize, "[%04u-%02u-%02u %02u:%02u:%02u] ",
        (unsigned)st.wYear,
        (unsigned)st.wMonth,
        (unsigned)st.wDay,
        (unsigned)st.wHour,
        (unsigned)st.wMinute,
        (unsigned)st.wSecond);
}

static void AppendSessionFileRaw(const char* data, size_t len) {
    if (!data || len == 0) return;
    if (!g_sessionFile) {
        char path[MAX_PATH] = { 0 };
        if (GetModuleFileNameA(NULL, path, MAX_PATH) == 0) return;
        char* slash = strrchr(path, '\\');
        if (slash) slash[1] = '\0';
        else path[0] = '\0';
        if (strcat_s(path, sizeof(path), "LizardSession.log") != 0) return;
        if (fopen_s(&g_sessionFile, path, "ab") != 0 || !g_sessionFile) return;
    }
    fwrite(data, 1, len, g_sessionFile);
    fflush(g_sessionFile);
}

// One timestamp per call: all lines from the same vsnprintf / buffer share it.
static void SessionAppendLinesFromBuffer(const char* buf, bool echoStdout, const WORD* echoColor) {
    if (!buf || !buf[0]) return;
    char ts[48];
    FormatSessionTimestampPrefix(ts, sizeof(ts));

    std::lock_guard<std::mutex> lock(g_sessionMutex);
    const char* p = buf;
    for (;;) {
        const char* nl = strchr(p, '\n');
        std::string lineContent;
        if (nl) {
            lineContent.assign(p, nl - p);
            while (!lineContent.empty() && lineContent.back() == '\r') lineContent.pop_back();
        } else {
            lineContent.assign(p);
        }

        std::string full = std::string(ts) + lineContent;
        g_sessionLines.push_back(full);
        while (g_sessionLines.size() > kMaxSessionLines) g_sessionLines.pop_front();
        AppendSessionFileRaw(full.c_str(), full.size());
        AppendSessionFileRaw("\n", 1);

        if (echoStdout) {
            if (echoColor && g_console && g_console != INVALID_HANDLE_VALUE) {
                SetConsoleTextAttribute(g_console, *echoColor);
            }
            fputs(full.c_str(), stdout);
            fputc('\n', stdout);
            fflush(stdout);
            if (echoColor && g_console && g_console != INVALID_HANDLE_VALUE) {
                SetConsoleTextAttribute(g_console, g_colorInfo);
            }
        }

        if (!nl) break;
        p = nl + 1;
    }
}

void LizardGetLogSnapshot(std::vector<std::string>& out) {
    std::lock_guard<std::mutex> lock(g_sessionMutex);
    out.assign(g_sessionLines.begin(), g_sessionLines.end());
}

void LizardLogf(WORD color, const char* fmt, ...) {
    char buf[16384];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    if (g_console && g_console != INVALID_HANDLE_VALUE) SetConsoleTextAttribute(g_console, color);
    fputs(buf, stdout);
    fflush(stdout);
    if (g_console && g_console != INVALID_HANDLE_VALUE) SetConsoleTextAttribute(g_console, g_colorInfo);
}

void LizardSessionLogf(WORD color, const char* fmt, ...) {
    char buf[16384];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    SessionAppendLinesFromBuffer(buf, true, &color);
}

void LizardResolverLogf(const char* fmt, ...) {
    char buf[16384];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    SessionAppendLinesFromBuffer(buf, true, nullptr);
}

void LizardLogRawUtf8(const char* utf8) {
    if (!utf8) return;
    SessionAppendLinesFromBuffer(utf8, true, nullptr);
}
