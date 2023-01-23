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
void EnumerateHornedAxeSpheres ( HANDLE hProcess, DWORD hornedaxe );

/*
    Dominion
*/
char* DominionGetIdentifier ( HANDLE hProcess, DWORD dominion );
DWORD DominionGetSphereList ( HANDLE hProcess, DWORD dominion );
char* SituationDominionGetEditableIdentifier ( HANDLE hProcess, DWORD dominion );

/*
    Sphere
*/
char* SphereGetEditorAbsolutePath ( HANDLE hProcess, DWORD sphere );
char* SphereGetEditorWildPath ( HANDLE hProcess, DWORD sphere );
DWORD SphereGetTokenList ( HANDLE hProcess, DWORD sphere );
void SpherePrintTokens ( HANDLE hProcess, DWORD sphere );
int SphereGetTokenCountById ( HANDLE hProcess, DWORD sphere, char* id );
DWORD SphereGetTokenById ( HANDLE hProcess, DWORD sphere, char* id, int index );
void SphereListPrintTokens ( HANDLE hProcess, DWORD spherelist );

/*
    Token
*/
char* TokenGetFullPath ( HANDLE hProcess, DWORD token );
DWORD TokenGetPayload ( HANDLE hProcess, DWORD token );

/*
    Situation
*/

DWORD SituationGetDominionList ( HANDLE hProcess, DWORD situation );
void SituationPrintDominions ( HANDLE hProcess, DWORD situation );
DWORD SituationGetTimeShadow ( HANDLE hProcess, DWORD situation );
void SituationSetTime ( HANDLE hProcess, DWORD situation, float time );

#endif // PROCESS_HANDLING_H
