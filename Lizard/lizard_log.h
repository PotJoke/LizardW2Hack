#pragma once

#include <Windows.h>
#include <cstdarg>
#include <string>
#include <vector>

// Populated by dllmain after AllocConsole (used by lizard_log.cpp for colors).
extern HANDLE g_console;
extern WORD g_colorInfo;

// Console/UI only (menus, banner text, redraws). Does not touch the session log.
void LizardLogf(WORD color, const char* fmt, ...);

// Session log: ring buffer + console + optional file (see lizard_log.cpp). Use for resolver, hooks, detour lines.
void LizardSessionLogf(WORD color, const char* fmt, ...);
void LizardResolverLogf(const char* fmt, ...);
void LizardLogRawUtf8(const char* utf8);

void LizardGetLogSnapshot(std::vector<std::string>& out);
