#include <stdio.h>

#include "memory_handling.h"

int ReadMonoRootDomain ( ) {
    HANDLE hProcess;
    FindProcessByExecutable("C:\\Program Files (x86)\\Steam\\steamapps\\common\\Cultist Simulator\\cultistsimulator.exe", &hProcess);
    
    HMODULE hModule;
    FindModuleByName(hProcess, "mono-2.0-bdwgc.dll", &hModule);
    
    DWORD func_address;
    FindExportByName(hProcess, hModule, "mono_get_root_domain", &func_address);

    // mono-2.0-bdwgc.mono_get_root_domain
    //   A1 ???????? | mov eax, []
    //   C3          | ret

    DWORD rootdomainpointerpointer = 0;
    ReadDWORD(hProcess, (LPVOID)hModule + func_address + 1, &rootdomainpointerpointer);

    DWORD rootdomainpointer = 0;
    ReadDWORD(hProcess, (LPVOID)rootdomainpointerpointer, &rootdomainpointer);
    printf("rootdomainpointer: %08X\n", rootdomainpointer);

    return 0;
}
