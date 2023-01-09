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
DWORD Read32DWORD ( HANDLE hProcess, DWORD address );
char* Read32UTF8String ( HANDLE hProcess, DWORD address );
char* Read32MonoWideString ( HANDLE hProcess, DWORD address );

int HexDump ( HANDLE hProcess, LPCVOID address, SIZE_T size );

DWORD ReadRootMonoDomain32 ( HANDLE hProcess );

DWORD MonoDomain32GetAssemblyList ( HANDLE hProcess, DWORD address );
int MonoDomain32EnumerateAssemblies ( HANDLE hProcess, DWORD address );
DWORD MonoDomain32GetAssemblyByName ( HANDLE hProcess, DWORD address, char* name );
DWORD MonoDomain32GetFriendlyName ( HANDLE hProcess, DWORD address );

DWORD MonoAssembly32GetNameInternal ( HANDLE hProcess, DWORD address );
DWORD MonoAssembly32GetImage ( HANDLE hProcess, DWORD monoassembly );

DWORD MonoImage32GetName ( HANDLE hProcess, DWORD monoimage );
DWORD MonoImage32GetClassCache ( HANDLE hProcess, DWORD monoimage );
int EnumerateMonoInternalHashTable ( HANDLE hProcess, DWORD ghashtable );
DWORD MonoImage32GetClassByName ( HANDLE hProcess, DWORD monoimage, char* namespace, char* name );

DWORD MonoClass32GetName ( HANDLE hProcess, DWORD monoclass );
DWORD MonoClass32GetNamespace ( HANDLE hProcess, DWORD monoclass );
DWORD MonoClass32GetRuntimeInfo ( HANDLE hProcess, DWORD monoclass );
DWORD MonoClass32GetVTable ( HANDLE hProcess, DWORD monoclass );
DWORD MonoClass32GetVTableSize ( HANDLE hProcess, DWORD monoclass );
DWORD MonoClass32GetStaticFieldData ( HANDLE hProcess, DWORD monoclass );
DWORD MonoClass32GetFields ( HANDLE hProcess, DWORD monoclass );
int MonoClass32EnumerateMonoClassFields ( HANDLE hProcess, DWORD monoclass );

DWORD MonoVTable32GetClass ( HANDLE hProcess, DWORD monovtable );
DWORD MonoVTable32GetDomain ( HANDLE hProcess, DWORD monovtable );

#endif // MEMORY_HANDLING_H
