#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <tchar.h>
#include <stdio.h>

#include "memory_handling.h"

int EnumerateProcesses();
int ProcessSummary(HANDLE hProcess);
int FindProcessByExecutable(char* executable, HANDLE* hProcess);
int ReadMemoryTest(HANDLE hProcess, LPCVOID address, SIZE_T size);

int main ( ) {
    EnumerateProcesses();
    HANDLE hProcess;
    FindProcessByExecutable("C:\\Program Files (x86)\\Steam\\steamapps\\common\\Cultist Simulator\\cultistsimulator.exe", &hProcess);
    ProcessSummary(hProcess);
    EnumerateModules(hProcess);
    return 0;
}

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

int ProcessSummary ( HANDLE hProcess ) {
    TCHAR filename[MAX_PATH];
    if ( !GetModuleFileNameEx(hProcess, NULL, filename, MAX_PATH) ) {
        printf("Failed to get module filename\n");
        return 1;
    }

    PROCESS_MEMORY_COUNTERS pmc;
    if ( !GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)) ) {
        printf("Failed to get process memory info\n");
        return 1;
    }

    FILETIME creationTime, exitTime, kernelTime, userTime;
    if ( !GetProcessTimes(hProcess, &creationTime, &exitTime, &kernelTime, &userTime) ) {
        printf("Failed to get process times\n");
        return 1;
    }

    printf("Memory usage: %ld\n", pmc.WorkingSetSize);
    printf("Creation time: %ld\n", creationTime.dwLowDateTime);

    return 0;
}

int FindProcessByExecutable ( char* executable, HANDLE* hProcess ) {
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
        HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe.th32ProcessID);
        if ( process == NULL ) {
            printf("Failed to open process %d (%d); %s\n", pe.th32ProcessID, GetLastError(), pe.szExeFile);
            continue;
        }

        TCHAR filename[MAX_PATH];
        if ( !GetModuleFileNameEx(process, NULL, filename, MAX_PATH) ) {
            printf("Failed to get module filename\n");
            return 1;
        }

        if ( strcmp(filename, executable) == 0 ) {
            printf("Found process %s with pid %d\n", executable, pe.th32ProcessID);
            *hProcess = process;
            break;
        }
        
        CloseHandle(process);
    } while( Process32Next(snapshot, &pe) );

    CloseHandle(snapshot);

    return 0;
}
