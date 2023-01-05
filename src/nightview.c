#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <tchar.h>
#include <stdio.h>

#include "memory_handling.h"

int main ( ) {
    EnumerateProcesses();
    HANDLE hProcess;
    FindProcessByExecutable("C:\\Program Files (x86)\\Steam\\steamapps\\common\\Cultist Simulator\\cultistsimulator.exe", &hProcess);
    
    EnumerateModules(hProcess);
    HMODULE hModule;
    FindModuleByName(hProcess, "mono-2.0-bdwgc.dll", &hModule);
    printf("mono-2.0-bdwgc.dll: 0x%08X\n", hModule);

    HexDump(hProcess, hModule, 0x1000);
    EnumerateExportTable(hProcess, hModule);
    return 0;
}
