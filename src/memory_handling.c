#include <windows.h>
#include <psapi.h>
#include <stdio.h>

#include "memory_handling.h"

int EnumerateModules ( HANDLE hProcess ) {
    HMODULE hModules[1024];
    DWORD cbNeeded;
    if ( EnumProcessModulesEx(hProcess, hModules, sizeof(hModules), &cbNeeded, LIST_MODULES_ALL) ) {
        int numModules = cbNeeded / sizeof(HMODULE);

        for ( int i = 0; i < numModules; ++i ) {
            char path[MAX_PATH];
            if ( GetModuleBaseNameA(hProcess, hModules[i], path, sizeof(path)) ) {
                printf("Module %d: %s\n", i + 1, path);
            }
            if ( GetModuleFileNameExA(hProcess, hModules[i], path, sizeof(path)) ) {
                printf("  Path: %s\n", path);
            }
            printf("  Base address: 0x%08X\n", hModules[i]);
            MODULEINFO info;
            if ( GetModuleInformation(hProcess, hModules[i], &info, sizeof(info)) ) {
                printf("  Size: %d bytes\n", info.SizeOfImage);
            }
        }
    }
    return 0;
}

int ReadMemoryTest ( HANDLE hProcess, LPCVOID lpBaseAddress, SIZE_T nSize ) {
    LPVOID lpBuffer = VirtualAlloc(NULL, nSize, MEM_COMMIT, PAGE_READWRITE);
    if ( lpBuffer == NULL ) {
        printf("Failed to allocate buffer\n");
        return 1;
    }

    SIZE_T lpNumberOfBytesRead;
    if (!ReadProcessMemory(hProcess, lpBaseAddress, lpBuffer, nSize, &lpNumberOfBytesRead)) {
        printf("Failed to read process memory\n");
        return 1;
    }

    printf("Read %d bytes from process memory;\n", lpNumberOfBytesRead);
    printf("Buffer contents: %s\n", (char*)lpBuffer);

    VirtualFree(lpBuffer, 0, MEM_RELEASE);
    CloseHandle(hProcess);
}
