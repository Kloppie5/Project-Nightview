#ifndef MEMORY_HANDLING_H
#define MEMORY_HANDLING_H

#include <windows.h>

int EnumerateModules ( HANDLE hProcess );
int FindProcessByExecutable ( char* executable, HANDLE* hProcess );
int HexDump ( HANDLE hProcess, LPCVOID address, SIZE_T size );
int EnumerateExportTable ( HANDLE hProcess, LPCVOID address );
int ReadMemoryTest ( HANDLE hProcess, LPCVOID address, SIZE_T size );

#endif // MEMORY_HANDLING_H
