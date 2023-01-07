#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <tchar.h>
#include <stdio.h>

#include "memory_handling.h"

int EnumerateProcesses ( ) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if ( snapshot == INVALID_HANDLE_VALUE ) {
        printf("Failed to create snapshot\n");
        return 1;
    }

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(pe);

    if ( !Process32First(snapshot, &pe) ) {
        printf("Failed to enumerate first process\n");
        return 1;
    }

    do {
        printf("%6ld | %s\n", pe.th32ProcessID, pe.szExeFile);
    } while( Process32Next(snapshot, &pe) );

    CloseHandle(snapshot);

    return 0;
}
HANDLE FindProcessByExecutable ( char* executable ) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if ( snapshot == INVALID_HANDLE_VALUE ) {
        printf("Failed to create snapshot\n");
        return NULL;
    }

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(pe);

    if ( !Process32First(snapshot, &pe) ) {
        printf("Failed to enumerate first process\n");
        return NULL;
    }

    HANDLE hProcess;
    do {
        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe.th32ProcessID);
        if ( hProcess == NULL )
            continue;

        TCHAR filename[MAX_PATH];
        if ( !GetModuleFileNameEx(hProcess, NULL, filename, MAX_PATH) ) {
            printf("Failed to get module filename\n");
            return NULL;
        }

        if ( strcmp(filename, executable) == 0 ) {
            CloseHandle(snapshot);
            return hProcess;
        }
        
        CloseHandle(hProcess);
    } while( Process32Next(snapshot, &pe) );

    CloseHandle(snapshot);
    
    return NULL;
}

int EnumerateModules ( HANDLE hProcess ) {
    HMODULE hModules[1024];
    DWORD cbNeeded;
    if ( EnumProcessModulesEx(hProcess, hModules, sizeof(hModules), &cbNeeded, LIST_MODULES_ALL) ) {
        int numModules = cbNeeded / sizeof(HMODULE);

        for ( int i = 0; i < numModules; ++i ) {
            TCHAR lpBaseName[MAX_PATH];
            if ( !GetModuleBaseNameA(hProcess, hModules[i], lpBaseName, sizeof(lpBaseName)) ) {
                printf("Failed to get module base name\n");
                return 1;
            }
            TCHAR lpFileName[MAX_PATH];
            if ( !GetModuleFileNameExA(hProcess, hModules[i], lpFileName, sizeof(lpFileName)) ) {
                printf("Failed to get module file name\n");
                return 1;
            }
            MODULEINFO info;
            if ( !GetModuleInformation(hProcess, hModules[i], &info, sizeof(info)) ) {
                printf("Failed to get module information\n");
                return 1;
            }
            
            // Skip Windows modules
            if ( strstr(lpFileName, "C:\\WINDOWS\\") != NULL || strstr(lpFileName, "C:\\Windows\\") != NULL )
                continue;
            
            // Skip Steam modules
            if ( strstr(lpFileName, "C:\\Program Files (x86)\\Steam\\") != NULL && strstr(lpFileName, "C:\\Program Files (x86)\\Steam\\steamapps\\" ) == NULL )
                continue;
            
            printf("0x%08X-0x%08X | %s (%s)\n", info.lpBaseOfDll, info.lpBaseOfDll + info.SizeOfImage, lpBaseName, lpFileName);
        }
    }
    return 0;
}
HMODULE FindModuleByName ( HANDLE hProcess, char* name ) {
    HMODULE hModules[1024];
    DWORD cbNeeded;
    if ( EnumProcessModulesEx(hProcess, hModules, sizeof(hModules), &cbNeeded, LIST_MODULES_ALL) ) {
        int numModules = cbNeeded / sizeof(HMODULE);

        for ( int i = 0; i < numModules; ++i ) {
            TCHAR lpBaseName[MAX_PATH];
            if ( !GetModuleBaseNameA(hProcess, hModules[i], lpBaseName, sizeof(lpBaseName)) )
                continue;
            
            if ( strcmp(lpBaseName, name) == 0 ) {
                return hModules[i];
            }
        }
    }
    return NULL;
}

int EnumerateExportTable ( HANDLE hProcess, LPCVOID lpBaseAddress ) {
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)malloc(sizeof(IMAGE_DOS_HEADER));
    ReadProcessMemory(hProcess, lpBaseAddress, dosHeader, sizeof(IMAGE_DOS_HEADER), NULL);
    
    PIMAGE_NT_HEADERS32 ntHeader32 = (PIMAGE_NT_HEADERS32)malloc(sizeof(IMAGE_NT_HEADERS32));
    ReadProcessMemory(hProcess, (LPCVOID)(lpBaseAddress + dosHeader->e_lfanew), ntHeader32, sizeof(IMAGE_NT_HEADERS32), NULL);
    
    PIMAGE_EXPORT_DIRECTORY exportDirectory = (PIMAGE_EXPORT_DIRECTORY)malloc(sizeof(IMAGE_EXPORT_DIRECTORY));
    ReadProcessMemory(hProcess, (LPCVOID)(lpBaseAddress + ntHeader32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress), exportDirectory, sizeof(IMAGE_EXPORT_DIRECTORY), NULL);
    
    DWORD* names = (DWORD*)malloc(exportDirectory->NumberOfNames * sizeof(DWORD));
    ReadProcessMemory(hProcess, (LPCVOID)(lpBaseAddress + exportDirectory->AddressOfNames), names, exportDirectory->NumberOfNames * sizeof(DWORD), NULL);
    WORD* ordinals = (WORD*)malloc(exportDirectory->NumberOfNames * sizeof(WORD));
    ReadProcessMemory(hProcess, (LPCVOID)(lpBaseAddress + exportDirectory->AddressOfNameOrdinals), ordinals, exportDirectory->NumberOfNames * sizeof(WORD), NULL);
    DWORD* addresses = (DWORD*)malloc(exportDirectory->NumberOfNames * sizeof(DWORD));
    ReadProcessMemory(hProcess, (LPCVOID)(lpBaseAddress + exportDirectory->AddressOfFunctions), addresses, exportDirectory->NumberOfNames * sizeof(DWORD), NULL);

    for ( int i = 0; i < exportDirectory->NumberOfNames; ++i ) {
        char* name = (char*)malloc(256);
        ReadProcessMemory(hProcess, (LPCVOID)(lpBaseAddress + names[i]), name, 256, NULL);
        printf("%04X | %08X | %s\n", ordinals[i], addresses[ordinals[i]], name);
        free(name);
    }

    free(names);
    free(exportDirectory);
    free(ntHeader32);
    free(dosHeader);

    return 0;
}
DWORD FindExportByName ( HANDLE hProcess, LPCVOID lpBaseAddress, char* name ) {
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)malloc(sizeof(IMAGE_DOS_HEADER));
    ReadProcessMemory(hProcess, lpBaseAddress, dosHeader, sizeof(IMAGE_DOS_HEADER), NULL);
    
    PIMAGE_NT_HEADERS32 ntHeader32 = (PIMAGE_NT_HEADERS32)malloc(sizeof(IMAGE_NT_HEADERS32));
    ReadProcessMemory(hProcess, (LPCVOID)(lpBaseAddress + dosHeader->e_lfanew), ntHeader32, sizeof(IMAGE_NT_HEADERS32), NULL);
    
    PIMAGE_EXPORT_DIRECTORY exportDirectory = (PIMAGE_EXPORT_DIRECTORY)malloc(sizeof(IMAGE_EXPORT_DIRECTORY));
    ReadProcessMemory(hProcess, (LPCVOID)(lpBaseAddress + ntHeader32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress), exportDirectory, sizeof(IMAGE_EXPORT_DIRECTORY), NULL);
    
    DWORD* names = (DWORD*)malloc(exportDirectory->NumberOfNames * sizeof(DWORD));
    ReadProcessMemory(hProcess, (LPCVOID)(lpBaseAddress + exportDirectory->AddressOfNames), names, exportDirectory->NumberOfNames * sizeof(DWORD), NULL);
    WORD* ordinals = (WORD*)malloc(exportDirectory->NumberOfNames * sizeof(WORD));
    ReadProcessMemory(hProcess, (LPCVOID)(lpBaseAddress + exportDirectory->AddressOfNameOrdinals), ordinals, exportDirectory->NumberOfNames * sizeof(WORD), NULL);
    DWORD* addresses = (DWORD*)malloc(exportDirectory->NumberOfNames * sizeof(DWORD));
    ReadProcessMemory(hProcess, (LPCVOID)(lpBaseAddress + exportDirectory->AddressOfFunctions), addresses, exportDirectory->NumberOfNames * sizeof(DWORD), NULL);

    DWORD address = NULL;
    for ( int i = 0; i < exportDirectory->NumberOfNames; ++i ) {
        char* exportName = (char*)malloc(256);
        ReadProcessMemory(hProcess, (LPCVOID)(lpBaseAddress + names[i]), exportName, 256, NULL);
        if ( strcmp(exportName, name) == 0 ) {
            address = addresses[ordinals[i]];
            free(exportName);
            break;
        }
        free(exportName);
    }

    free(names);
    free(exportDirectory);
    free(ntHeader32);
    free(dosHeader);

    return address;
}

BYTE Read32BYTE ( HANDLE hProcess, DWORD address ) {
    BYTE result = 0;
    SIZE_T lpNumberOfBytesRead;
    if ( !ReadProcessMemory(hProcess, (LPCVOID)address, &result, sizeof(BYTE), &lpNumberOfBytesRead) ) {
        printf("Failed to read process memory; %d\n", GetLastError());
        return 0;
    }
    return result;
}
DWORD Read32DWORD ( HANDLE hProcess, DWORD address ) {
    DWORD result = 0;
    SIZE_T lpNumberOfBytesRead;
    if ( !ReadProcessMemory(hProcess, (LPCVOID)address, &result, sizeof(DWORD), &lpNumberOfBytesRead) ) {
        printf("Failed to read process memory; %d\n", GetLastError());
        return 0;
    }
    return result;
}
char* Read32UTF8String ( HANDLE hProcess, DWORD address ) {
    char* result = (char*)malloc(256);
    SIZE_T lpNumberOfBytesRead;
    if ( !ReadProcessMemory(hProcess, (LPCVOID)address, result, 256, &lpNumberOfBytesRead) ) {
        printf("Failed to read process memory; %d\n", GetLastError());
        return;
    }
    return result;
}

int HexDump ( HANDLE hProcess, LPCVOID lpBaseAddress, SIZE_T nSize ) {
    LPVOID lpBuffer = (LPVOID)malloc(nSize);
    if ( lpBuffer == NULL ) {
        printf("Failed to allocate buffer\n");
        return 1;
    }

    SIZE_T lpNumberOfBytesRead;
    if (!ReadProcessMemory(hProcess, lpBaseAddress, lpBuffer, nSize, &lpNumberOfBytesRead)) {
        printf("Failed to read process memory\n");
        return 1;
    }

    for ( int line = 0; line < lpNumberOfBytesRead; line += 32 ) {
        printf("%08X: ", lpBaseAddress+line);
        for ( int i = 0; i < 32; ++i ) {
            if ( line + i < lpNumberOfBytesRead )
                printf("%02X ", ((unsigned char*)lpBuffer)[line + i]);
            else
                printf("   ");
        }
        printf(" | ");
        for ( int i = 0; i < 32; ++i ) {
            if ( line + i < lpNumberOfBytesRead ) {
                if ( ((unsigned char*)lpBuffer)[line + i] >= 32 && ((unsigned char*)lpBuffer)[line + i] <= 126 )
                    printf("%c", ((unsigned char*)lpBuffer)[line + i]);
                else
                    printf(".");
            }
            else
                printf(" ");
        }
        printf("\n");
    }

    free(lpBuffer);
}

/* Mono */
DWORD ReadRootMonoDomain32 ( HANDLE hProcess ) {
    HMODULE hModule = FindModuleByName(hProcess, "mono-2.0-bdwgc.dll");
    DWORD func_address = FindExportByName(hProcess, hModule, "mono_get_root_domain");
    // mono-2.0-bdwgc.mono_get_root_domain
    //   A1 ???????? | mov eax, [????????]
    //   C3          | ret
    DWORD rootdomainpointerpointer = Read32DWORD(hProcess, (DWORD)hModule + func_address + 1);
    DWORD rootdomainpointer = Read32DWORD(hProcess, rootdomainpointerpointer);

    return rootdomainpointer;
}
DWORD MonoDomain32GetAssemblyList ( HANDLE hProcess, DWORD monodomain ) {
    HMODULE hModule = FindModuleByName(hProcess, "mono-2.0-bdwgc.dll");
    DWORD func_address = FindExportByName(hProcess, hModule, "mono_domain_assembly_foreach");
    // mono-2.0-bdwgc.mono_domain_assembly_foreach
    //   55 8B EC 53 8B 5D 08 8B CB 56 57 | stack management
    //   E8 ???????? | call ????????
    //   8B 73 ??    | mov esi, [ebx+??]
    //   85 F6       | test esi, esi
    BYTE offset = Read32BYTE(hProcess, (DWORD)hModule + func_address + 0x12);
    
    DWORD assemblylistpointer = Read32DWORD(hProcess, monodomain + offset);

    return assemblylistpointer;
}
DWORD MonoDomain32GetFriendlyName ( HANDLE hProcess, DWORD monodomain ) {
    HMODULE hModule = FindModuleByName(hProcess, "mono-2.0-bdwgc.dll");
    DWORD func_address = FindExportByName(hProcess, hModule, "mono_domain_get_friendly_name");
    // mono-2.0-bdwgc.mono_domain_get_friendly_name
    //   55 8B EC 8B 45 08 | stack management
    //   8B 40 ??          | mov eax, [eax+??]
    //   5D C3             | stack management
    BYTE offset = Read32BYTE(hProcess, (DWORD)hModule + func_address + 0x8);

    DWORD friendlynamepointer = Read32DWORD(hProcess, monodomain + offset);

    return friendlynamepointer;
}
DWORD MonoAssembly32GetNameInternal ( HANDLE hProcess, DWORD monoassembly ) {
    HMODULE hModule = FindModuleByName(hProcess, "mono-2.0-bdwgc.dll");
    DWORD func_address = FindExportByName(hProcess, hModule, "mono_assembly_get_name_internal");
    // mono-2.0-bdwgc.mono_assembly_get_name_internal
    //   55 8B EC 8B 45 08 | stack management
    //   83 C0 ??          | add eax, ??
    //   5D C3             | stack management
    BYTE offset = Read32BYTE(hProcess, (DWORD)hModule + func_address + 0x8);

    DWORD nameinternalpointer = Read32DWORD(hProcess, monoassembly + offset);

    return nameinternalpointer;
}
