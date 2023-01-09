#ifndef GOLEMANCY_H
#define GOLEMANCY_H

#include <windows.h>

int Something();
int EnumerateWatchmanRegistered ( HANDLE hProcess, DWORD watchman );
DWORD WatchmanGet ( HANDLE hProcess, DWORD watchman, char* classname );

#endif // GOLEMANCY_H
