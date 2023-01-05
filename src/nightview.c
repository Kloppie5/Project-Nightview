#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <tchar.h>
#include <stdio.h>

int EnumerateProcesses();
int ProcessSummary(DWORD pid);
int FindProcessByExecutable(char* executable, DWORD* pid);
int ReadMemoryTest(DWORD pid, LPCVOID lpBaseAddress, SIZE_T nSize);

int main ( ) {
    EnumerateProcesses();
    DWORD pid = 0;
    FindProcessByExecutable("C:\\Program Files (x86)\\Steam\\steamapps\\common\\Cultist Simulator\\cultistsimulator.exe", &pid);
    ProcessSummary(pid);
    ReadMemoryTest(pid, (LPCVOID)0x344018, 0x1000);
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

int ProcessSummary ( DWORD pid ) {
    HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if ( process == NULL ) {
        printf("Failed to open process\n");
        return 1;
    }

    TCHAR filename[MAX_PATH];
    if ( !GetModuleFileNameEx(process, NULL, filename, MAX_PATH) ) {
        printf("Failed to get module filename\n");
        return 1;
    }

    printf("Process %d: %s\n", pid, filename);

    PROCESS_MEMORY_COUNTERS pmc;
    if ( !GetProcessMemoryInfo(process, &pmc, sizeof(pmc)) ) {
        printf("Failed to get process memory info\n");
        return 1;
    }

    FILETIME creationTime, exitTime, kernelTime, userTime;
    if ( !GetProcessTimes(process, &creationTime, &exitTime, &kernelTime, &userTime) ) {
        printf("Failed to get process times\n");
        return 1;
    }

    printf("Memory usage: %ld\n", pmc.WorkingSetSize);
    printf("Creation time: %ld\n", creationTime.dwLowDateTime);

    CloseHandle(process);

    return 0;
}

int FindProcessByExecutable ( char* executable, DWORD* pid ) {
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

        CloseHandle(process);

        if ( strcmp(filename, executable) == 0 ) {
            printf("Found process %s with pid %d\n", executable, pe.th32ProcessID);
            *pid = pe.th32ProcessID;
            break;
        }
    } while( Process32Next(snapshot, &pe) );

    CloseHandle(snapshot);

    return 0;
}

int ReadMemoryTest ( DWORD pid, LPCVOID lpBaseAddress, SIZE_T nSize ) {
    HANDLE hProcess = OpenProcess(PROCESS_VM_READ, FALSE, pid);
    if ( hProcess == NULL ) {
        printf("Failed to open process\n");
        return 1;
    }

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
