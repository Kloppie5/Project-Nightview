#include <stdio.h>

#include "memory_handling.h"

int Something ( ) {
    HANDLE hProcess = FindProcessByExecutable("C:\\Program Files (x86)\\Steam\\steamapps\\common\\Cultist Simulator\\cultistsimulator.exe");
    DWORD rootdomainpointer = ReadRootMonoDomain32(hProcess);

    MonoDomain32EnumerateAssemblies(hProcess, rootdomainpointer);
//SecretHistories.Main
    DWORD assembly = MonoDomain32GetAssemblyByName(hProcess, rootdomainpointer, "SecretHistories.Main");
    DWORD assemblyname = MonoAssembly32GetNameInternal(hProcess, assembly);
    char* assemblynamestring = Read32UTF8String(hProcess, assemblyname);
    printf("Assembly [%08X]: %s\n", assembly, assemblynamestring);

    DWORD assemblyimage = MonoAssembly32GetImage(hProcess, assembly);
    DWORD assemblyimagename = MonoImage32GetName(hProcess, assemblyimage);
    char* assemblyimagenamestring = Read32UTF8String(hProcess, assemblyimagename);
    printf("Assembly Image [%08X]: %s\n", assemblyimage, assemblyimagenamestring);

    DWORD assemblyimagehashtable = MonoImage32GetClassCache(hProcess, assemblyimage);

    EnumerateMonoInternalHashTable(hProcess, assemblyimagehashtable);

    HexDump(hProcess, (LPVOID)rootdomainpointer, 0x100);
}
