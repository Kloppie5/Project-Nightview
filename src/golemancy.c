#include <stdio.h>

#include "memory_handling.h"

int Something ( ) {
    HANDLE hProcess = FindProcessByExecutable("C:\\Program Files (x86)\\Steam\\steamapps\\common\\Cultist Simulator\\cultistsimulator.exe");
    DWORD rootdomainpointer = ReadRootMonoDomain32(hProcess);
    DWORD rootdomainnamepointer = MonoDomain32GetFriendlyName(hProcess, rootdomainpointer);
    char* rootdomainname = Read32UTF8String(hProcess, rootdomainnamepointer);
    printf("Root domain [%08X]: %s\n", rootdomainpointer, rootdomainname);
    free(rootdomainname);

    DWORD assemblylist = MonoDomain32GetAssemblyList(hProcess, rootdomainpointer);

    while (assemblylist != 0) {
        DWORD assembly = Read32DWORD(hProcess, assemblylist);
        DWORD assemblynamepointer = MonoAssembly32GetNameInternal(hProcess, assembly);
        char* assemblyname = Read32UTF8String(hProcess, assemblynamepointer);
        printf("Assembly [%08X]: %s\n", assembly, assemblyname);
        free(assemblyname);
        
        assemblylist = Read32DWORD(hProcess, assemblylist + 0x4);
    }

    HexDump(hProcess, (LPVOID)rootdomainpointer, 0x100);
}
