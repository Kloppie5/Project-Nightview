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

int HexDump ( HANDLE hProcess, LPCVOID lpBaseAddress, SIZE_T nSize ) {
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

    for ( int line = 0; line < lpNumberOfBytesRead; line += 32 ) {
        printf("%08X: ", line);
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

    VirtualFree(lpBuffer, 0, MEM_RELEASE);
}

int EnumerateExportTable ( HANDLE hProcess, LPCVOID lpBaseAddress ) {
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)malloc(sizeof(IMAGE_DOS_HEADER));
    ReadProcessMemory(hProcess, lpBaseAddress, dosHeader, sizeof(IMAGE_DOS_HEADER), NULL);
    printf("e_lfanew: %08X\n", dosHeader->e_lfanew);

    PIMAGE_NT_HEADERS32 ntHeader32 = (PIMAGE_NT_HEADERS32)malloc(sizeof(IMAGE_NT_HEADERS32));
    ReadProcessMemory(hProcess, (LPCVOID)(lpBaseAddress + dosHeader->e_lfanew), ntHeader32, sizeof(IMAGE_NT_HEADERS32), NULL);
    printf("Magic: %04X\n", ntHeader32->OptionalHeader.Magic);

    PIMAGE_EXPORT_DIRECTORY exportDirectory = (PIMAGE_EXPORT_DIRECTORY)malloc(sizeof(IMAGE_EXPORT_DIRECTORY));
    ReadProcessMemory(hProcess, (LPCVOID)(lpBaseAddress + ntHeader32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress), exportDirectory, sizeof(IMAGE_EXPORT_DIRECTORY), NULL);
    printf("NumberOfNames: %08X\n", exportDirectory->NumberOfNames);

    DWORD* names = (DWORD*)malloc(exportDirectory->NumberOfNames * sizeof(DWORD));
    ReadProcessMemory(hProcess, (LPCVOID)(lpBaseAddress + exportDirectory->AddressOfNames), names, exportDirectory->NumberOfNames * sizeof(DWORD), NULL);

    for ( int i = 0; i < exportDirectory->NumberOfNames; ++i ) {
        char* name = (char*)malloc(256);
        ReadProcessMemory(hProcess, (LPCVOID)(lpBaseAddress + names[i]), name, 256, NULL);
        printf("%s\n", name);
        free(name);
    }

    free(names);
    free(exportDirectory);
    free(ntHeader32);
    free(dosHeader);

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
}
