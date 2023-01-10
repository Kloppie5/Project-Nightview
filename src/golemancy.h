#ifndef GOLEMANCY_H
#define GOLEMANCY_H

#include <windows.h>

int Something();
int EnumerateWatchmanRegistered ( HANDLE hProcess, DWORD watchman );
DWORD WatchmanGet ( HANDLE hProcess, DWORD watchman, char* classname );
DWORD GetFirstTokenByAbsolutePathPrefix ( HANDLE hProcess, DWORD hornedaxe, char* absolutepathprefix );
DWORD TokenGetPayload ( HANDLE hProcess, DWORD token );
void SituationInteractWithIncomingToken ( HANDLE hProcess, DWORD situation, DWORD token );

#endif // GOLEMANCY_H
