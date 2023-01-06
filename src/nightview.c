#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <tchar.h>
#include <stdio.h>

#include "memory_handling.h"
#include "golemancy.h"

int main ( ) {
    HANDLE hProcess;
    FindProcessByExecutable("C:\\Program Files (x86)\\Steam\\steamapps\\common\\Cultist Simulator\\cultistsimulator.exe", &hProcess);
    printf("hProcess: %d\n", hProcess);

    HMODULE hModule;
    FindModuleByName(hProcess, "mono-2.0-bdwgc.dll", &hModule);
    printf("hModule: %08X\n", hModule);

    DWORD address;
    FindExportByName(hProcess, hModule, "mono_get_root_domain", &address);
    printf("Address: %08X\n", address);
    
    ReadMonoRootDomain();
    
    return 0;
}
