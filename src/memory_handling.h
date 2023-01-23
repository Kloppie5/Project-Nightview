#ifndef MEMORY_HANDLING_H
#define MEMORY_HANDLING_H

#include <windows.h>

int EnumerateProcesses();
HANDLE FindProcessByExecutable ( char* executable );

int EnumerateModules ( HANDLE hProcess );
HMODULE FindModuleByName ( HANDLE hProcess, char* name );

int EnumerateExportTable ( HANDLE hProcess, LPCVOID address );
DWORD FindExportByName ( HANDLE hProcess, LPCVOID address, char* name );

BYTE Read32BYTE ( HANDLE hProcess, DWORD address );
WORD Read32WORD ( HANDLE hProcess, DWORD address );
DWORD Read32DWORD ( HANDLE hProcess, DWORD address );
int Write32DWORD ( HANDLE hProcess, DWORD address, DWORD value );
char* Read32UTF8String ( HANDLE hProcess, DWORD address );
char* Read32MonoWideString ( HANDLE hProcess, DWORD address );

int HexDump ( HANDLE hProcess, LPCVOID address, SIZE_T size );

#endif // MEMORY_HANDLING_H
