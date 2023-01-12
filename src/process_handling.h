#ifndef PROCESS_HANDLING_H
#define PROCESS_HANDLING_H

#include <windows.h>

/*
    Watchman is a static class responsible for keeping track of a set of registered objects.
    The first and only static field is a dictionary of type Dictionary<Type, object>.
*/
DWORD GetWatchmanClass ( HANDLE hProcess );
int EnumerateWatchmanRegistered ( HANDLE hProcess, DWORD watchman );
DWORD WatchmanGet ( HANDLE hProcess, DWORD watchman, char* classname );

/*
    HornedAxe is the main Sphere container.
*/
DWORD HornedAxeGetSphereHashSet ( HANDLE hProcess, DWORD hornedaxe );
DWORD HornedAxeGetSphereById ( HANDLE hProcess, DWORD hornedaxe, char* id );

/*
    Sphere
*/
DWORD SphereGetTokenList ( HANDLE hProcess, DWORD sphere );
int SphereGetTokenCountById ( HANDLE hProcess, DWORD sphere, char* id );
DWORD SphereGetTokenById ( HANDLE hProcess, DWORD sphere, char* id, int index );

/*
    Token
*/
DWORD TokenGetPayload ( HANDLE hProcess, DWORD token );

#endif // PROCESS_HANDLING_H
