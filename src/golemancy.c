#include <stdio.h>
#include <windows.h>

#include "golemancy.h"
#include "memory_handling.h"

int Something ( ) {
    HANDLE hProcess = FindProcessByExecutable("C:\\Program Files (x86)\\Steam\\steamapps\\common\\Cultist Simulator\\cultistsimulator.exe");
    DWORD rootdomainpointer = ReadRootMonoDomain32(hProcess);
    DWORD assembly = MonoDomain32GetAssemblyByName(hProcess, rootdomainpointer, "SecretHistories.Main");
    DWORD assemblyimage = MonoAssembly32GetImage(hProcess, assembly);

    DWORD watchman = MonoImage32GetClassByName(hProcess, assemblyimage, "SecretHistories.UI", "Watchman");
    
    DWORD hornedaxe = WatchmanGet(hProcess, watchman, "HornedAxe");
    DumpHornedAxe(hProcess, hornedaxe);

    DWORD worktoken = GetFirstTokenByAbsolutePathPrefix(hProcess, hornedaxe, "~/tabletop!work_");
    DWORD worksituation = TokenGetPayload(hProcess, worktoken);

    DWORD jobtoken = GetFirstTokenByAbsolutePathPrefix(hProcess, hornedaxe, "~/tabletop!legacyphysicianjob_");
    
    printf("Work situation: %08X\n", worksituation);
    printf("Job token: %08X\n", jobtoken);

    //SituationOpen(hProcess, worksituation);
    //SituationInteractWithIncomingToken(hProcess, worksituation, jobtoken);
}

/*
    Watchman is a static class responsible for keeping track of a set of registered objects.
    The first and only static field is a dictionary of type Dictionary<Type, object>.
*/
int EnumerateWatchmanRegistered ( HANDLE hProcess, DWORD watchman ) {
    DWORD watchmanstatic = MonoClass32GetStaticFieldData(hProcess, watchman);

    DWORD registered = Read32DWORD(hProcess, watchmanstatic);

    DWORD entries = Read32DWORD(hProcess, registered + 0xC);
    DWORD count = Read32DWORD(hProcess, entries + 0xC);

    for ( int i = 0 ; i < count ; ++i ) {
        // DWORD hashcode = Read32DWORD(hProcess, entries + 0x10 + i * 0x10);
        // DWORD next = Read32DWORD(hProcess, entries + 0x14 + i * 0x10);
        DWORD key = Read32DWORD(hProcess, entries + 0x18 + i * 0x10);
        DWORD value = Read32DWORD(hProcess, entries + 0x1C + i * 0x10);
        if ( key == 0 ) continue;
        DWORD keytype = Read32DWORD(hProcess, key + 0x8);
        DWORD keyclass = Read32DWORD(hProcess, keytype);
        DWORD keyclassname = Read32DWORD(hProcess, keyclass + 0x2C);
        char* keyclassnamestr = Read32UTF8String(hProcess, keyclassname);

        printf("Entry [%s]: %08X\n", keyclassnamestr, value);

        free(keyclassnamestr);
    }

    return 0;
}
// static FucineRoot Get()
//  List<Sphere> _spheres
DWORD WatchmanGet ( HANDLE hProcess, DWORD watchman, char* classname ) {
    DWORD watchmanstatic = MonoClass32GetStaticFieldData(hProcess, watchman);

    DWORD registered = Read32DWORD(hProcess, watchmanstatic);

    DWORD entries = Read32DWORD(hProcess, registered + 0xC);
    DWORD count = Read32DWORD(hProcess, entries + 0xC);

    for ( int i = 0 ; i < count ; ++i ) {
        DWORD key = Read32DWORD(hProcess, entries + 0x18 + i * 0x10);
        DWORD value = Read32DWORD(hProcess, entries + 0x1C + i * 0x10);
        if ( key == 0 ) continue;
        DWORD keytype = Read32DWORD(hProcess, key + 0x8);
        DWORD keyclass = Read32DWORD(hProcess, keytype);
        DWORD keyclassname = Read32DWORD(hProcess, keyclass + 0x2C);
        char* keyclassnamestr = Read32UTF8String(hProcess, keyclassname);

        if ( strcmp(keyclassnamestr, classname) == 0 ) {
            free(keyclassnamestr);
            return value;
        }

        free(keyclassnamestr);
    }

    return 0;
}
DWORD GetFirstTokenByAbsolutePathPrefix ( HANDLE hProcess, DWORD hornedaxe, char* absolutepathprefix ) {
    DWORD _registeredSpheres = Read32DWORD(hProcess, hornedaxe + 0x1C);
    DWORD _registeredSpheresArray = Read32DWORD(hProcess, _registeredSpheres + 0xC);
    DWORD _registeredSpheresCount = Read32DWORD(hProcess, _registeredSpheresArray + 0xC);
    for ( int i = 0 ; i < _registeredSpheresCount ; ++i ) {
        DWORD sphere = Read32DWORD(hProcess, _registeredSpheresArray + 0x10 + i * 0xC + 0x8);
        if ( sphere == 0 ) continue;

        DWORD _tokens = Read32DWORD(hProcess, sphere + 0x28);
        DWORD _tokensArray = Read32DWORD(hProcess, _tokens + 0x8);
        DWORD _tokensCount = Read32DWORD(hProcess, _tokensArray + 0xC);
        for ( int j = 0 ; j < _tokensCount ; ++j ) {
            DWORD token = Read32DWORD(hProcess, _tokensArray + 0x10 + j * 4);
            if ( token == 0 ) continue;

            DWORD fullpath = Read32DWORD(hProcess, token + 0x24);
            char* fullpathstr = Read32MonoWideString(hProcess, fullpath);

            // prefix and no more ! or /
            if ( strncmp(fullpathstr, absolutepathprefix, strlen(absolutepathprefix)) == 0 && strchr(fullpathstr + strlen(absolutepathprefix), '!') == 0 && strchr(fullpathstr + strlen(absolutepathprefix), '/') == 0 ) {
                free(fullpathstr);
                return token;
            }

            free(fullpathstr);
        }
    }
    return 0;
}
DWORD TokenGetPayload ( HANDLE hProcess, DWORD token ) {
    return Read32DWORD(hProcess, token + 0x28);
}

void SituationOpen ( HANDLE hProcess, DWORD situation ) {
    printf("Opening situation %08X\n", situation);
    DWORD situationvtable = Read32DWORD(hProcess, situation);
    DWORD situationclass = Read32DWORD(hProcess, situationvtable);

    DWORD rootdomain = ReadRootMonoDomain32(hProcess);
    HMODULE hModule = FindModuleByName(hProcess, "mono-2.0-bdwgc.dll");
    DWORD mono_thread_attach = (DWORD)hModule + FindExportByName(hProcess, hModule, "mono_thread_attach");
    DWORD mono_runtime_invoke = (DWORD)hModule + FindExportByName(hProcess, hModule, "mono_runtime_invoke");
    DWORD Open = MonoClass32GetMonoMethodByName(hProcess, situationclass, "Open");

    DWORD code = VirtualAllocEx(hProcess, NULL, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    LPCVOID codeBuffer = (LPCVOID)malloc(0x1000);    
    
    // Attach mono thread
    //   68 rootdomain | PUSH rootdomain
    *(BYTE* )(codeBuffer +  0) = 0x68;
    *(DWORD*)(codeBuffer +  1) = rootdomain;
    //   B8 mono_thread_attach | MOV EAX, mono_thread_attach
    *(BYTE* )(codeBuffer +  5) = 0xB8;
    *(DWORD*)(codeBuffer +  6) = mono_thread_attach;
    //   FF D0 | CALL EAX
    *(WORD* )(codeBuffer + 10) = 0xD0FF;
    //   83 C4 04 | ADD ESP, 4
    *(WORD* )(codeBuffer + 12) = 0xC483;
    *(BYTE* )(codeBuffer + 14) = 0x04;

    // mono_runtime_invoke
    //   B8 mono_runtime_invoke | MOV EAX, mono_runtime_invoke
    *(BYTE* )(codeBuffer + 15) = 0xB8;
    *(DWORD*)(codeBuffer + 16) = mono_runtime_invoke;
    //   68 00000000 | PUSH exception handler
    *(BYTE* )(codeBuffer + 20) = 0x68;
    *(DWORD*)(codeBuffer + 21) = 0;
    //   68 00000000 | PUSH args
    *(BYTE* )(codeBuffer + 25) = 0x68;
    *(DWORD*)(codeBuffer + 26) = 0;
    //   68 situation | PUSH situation
    *(BYTE* )(codeBuffer + 30) = 0x68;
    *(DWORD*)(codeBuffer + 31) = situation;
    //   68 method | PUSH method
    *(BYTE* )(codeBuffer + 35) = 0x68;
    *(DWORD*)(codeBuffer + 36) = Open;
    //   FF D0 | CALL EAX
    *(WORD* )(codeBuffer + 40) = 0xD0FF;
    //   83 C4 10 | ADD ESP, 16
    *(WORD* )(codeBuffer + 42) = 0xC483;
    *(BYTE* )(codeBuffer + 44) = 0x10;

    // C3 | RET
    *(BYTE* )(codeBuffer + 45) = 0xC3;

    if ( !WriteProcessMemory(hProcess, code, codeBuffer, 0x1000, NULL) ) {
        printf("WriteProcessMemory failed: %08X\n", GetLastError());
        return;
    }

    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)code, NULL, 0, NULL);
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);

    free((void*)codeBuffer);
    VirtualFreeEx(hProcess, code, 0, MEM_RELEASE);
}
void SituationInteractWithIncomingToken ( HANDLE hProcess, DWORD situation, DWORD token ) {
    printf("Forcing situation %08X to interact with token %08X\n", situation, token);
    DWORD situationvtable = Read32DWORD(hProcess, situation);
    DWORD situationclass = Read32DWORD(hProcess, situationvtable);

    DWORD rootdomain = ReadRootMonoDomain32(hProcess);
    HMODULE hModule = FindModuleByName(hProcess, "mono-2.0-bdwgc.dll");
    DWORD mono_thread_attach = (DWORD)hModule + FindExportByName(hProcess, hModule, "mono_thread_attach");
    DWORD mono_runtime_invoke = (DWORD)hModule + FindExportByName(hProcess, hModule, "mono_runtime_invoke");
    DWORD InteractWithIncoming = MonoClass32GetMonoMethodByName(hProcess, situationclass, "InteractWithIncoming");

    DWORD args = VirtualAllocEx(hProcess, NULL, 0x1000, MEM_COMMIT, PAGE_READWRITE);
    LPCVOID argsBuffer = (LPCVOID)malloc(0x1000);
    DWORD code = VirtualAllocEx(hProcess, NULL, 0x1000, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    LPCVOID codeBuffer = (LPCVOID)malloc(0x1000);    
    
    // Attach mono thread
    //   68 rootdomain | PUSH rootdomain
    *(BYTE* )(codeBuffer +  0) = 0x68;
    *(DWORD*)(codeBuffer +  1) = rootdomain;
    //   B8 mono_thread_attach | MOV EAX, mono_thread_attach
    *(BYTE* )(codeBuffer +  5) = 0xB8;
    *(DWORD*)(codeBuffer +  6) = mono_thread_attach;
    //   FF D0 | CALL EAX
    *(WORD* )(codeBuffer + 10) = 0xD0FF;
    //   83 C4 04 | ADD ESP, 4
    *(WORD* )(codeBuffer + 12) = 0xC483;
    *(BYTE* )(codeBuffer + 14) = 0x04;

    // arguments
    *(DWORD*)(argsBuffer + 0) = token; // arg0

    // mono_runtime_invoke
    //   B8 mono_runtime_invoke | MOV EAX, mono_runtime_invoke
    *(BYTE* )(codeBuffer + 15) = 0xB8;
    *(DWORD*)(codeBuffer + 16) = mono_runtime_invoke;
    //   68 00000000 | PUSH exception handler
    *(BYTE* )(codeBuffer + 20) = 0x68;
    *(DWORD*)(codeBuffer + 21) = 0;
    //   68 args | PUSH args
    *(BYTE* )(codeBuffer + 25) = 0x68;
    *(DWORD*)(codeBuffer + 26) = args;
    //   68 situation | PUSH situation
    *(BYTE* )(codeBuffer + 30) = 0x68;
    *(DWORD*)(codeBuffer + 31) = situation;
    //   68 method | PUSH method
    *(BYTE* )(codeBuffer + 35) = 0x68;
    *(DWORD*)(codeBuffer + 36) = InteractWithIncoming;
    //   FF D0 | CALL EAX
    *(WORD* )(codeBuffer + 40) = 0xD0FF;
    //   83 C4 10 | ADD ESP, 16
    *(WORD* )(codeBuffer + 42) = 0xC483;
    *(BYTE* )(codeBuffer + 44) = 0x10;

    // C3 | RET
    *(BYTE* )(codeBuffer + 45) = 0xC3;

    WriteProcessMemory(hProcess, args, argsBuffer, 0x1000, NULL);
    WriteProcessMemory(hProcess, code, codeBuffer, 0x1000, NULL);
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)code, NULL, 0, NULL);
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);

    free((void*)argsBuffer);
    free((void*)codeBuffer);
    VirtualFreeEx(hProcess, args, 0, MEM_RELEASE);
    VirtualFreeEx(hProcess, code, 0, MEM_RELEASE);
}

int DumpHornedAxe ( HANDLE hProcess, DWORD hornedaxe ) {
    printf("HornedAxe: %08X\n", hornedaxe);
    
    DWORD _currentSituations = Read32DWORD(hProcess, hornedaxe + 0x14); // List<Situation>
    DWORD _registeredWays = Read32DWORD(hProcess, hornedaxe + 0x18);
    DWORD _registeredSpheres = Read32DWORD(hProcess, hornedaxe + 0x1C);
    
    printf("  _currentSituations: %08X\n", _currentSituations);
    // List<Situation>
    DWORD _currentSituationsArray = Read32DWORD(hProcess, _currentSituations + 0x8);
    DWORD _currentSituationsCount = Read32DWORD(hProcess, _currentSituationsArray + 0xC);
    for ( int i = 0 ; i < _currentSituationsCount ; ++i ) {
        DWORD situation = Read32DWORD(hProcess, _currentSituationsArray + 0x10 + i * 4);
        DWORD id = Read32DWORD(hProcess, situation + 0x20);
        char* idstr = Read32MonoWideString(hProcess, id);
        printf("    Situation %s\n", idstr);
        free(idstr);
    }

    printf("  _registeredWays: %08X\n", _registeredWays);
    // HashSet<Way>

    printf("  _registeredSpheres: %08X\n", _registeredSpheres);
    // HashSet<Sphere>
    /*
    public HashSet<Int32> ReadHashSetObject ( Int32 pointer ) {
      HashSet<Int32> set = new HashSet<Int32>();
      Int32 slotArray = MemoryHelper.ReadAbsolute<Int32>(_manager, pointer + 0xC);
      Int32 entries = MemoryHelper.ReadAbsolute<Int32>(_manager, slotArray + 0xC);
      for ( Int32 i = 0 ; i < entries ; ++i ) {
        Int32 hashcode = MemoryHelper.ReadAbsolute<Int32>(_manager, slotArray + 0x10 + i * 0xC);
        Int32 next = MemoryHelper.ReadAbsolute<Int32>(_manager, slotArray + 0x14 + i * 0xC);
        Int32 value = MemoryHelper.ReadAbsolute<Int32>(_manager, slotArray + 0x18 + i * 0xC);
        if ( value == 0 )
          continue;
        set.Add(value);
      }
      return set;
    }
    */
    DWORD _registeredSpheresArray = Read32DWORD(hProcess, _registeredSpheres + 0xC);
    DWORD _registeredSpheresCount = Read32DWORD(hProcess, _registeredSpheresArray + 0xC);
    for ( int i = 0 ; i < _registeredSpheresCount ; ++i ) {
        // DWORD hashcode = Read32DWORD(hProcess, _registeredSpheresArray + 0x10 + i * 0xC);
        // DWORD next = Read32DWORD(hProcess, _registeredSpheresArray + 0x10 + i * 0xC + 0x4);
        DWORD entry = Read32DWORD(hProcess, _registeredSpheresArray + 0x10 + i * 0xC + 0x8);
        if ( entry == 0 ) continue;

        DWORD _editorAbsolutePath = Read32DWORD(hProcess, entry + 0x18);
        char* _editorAbsolutePathstr = Read32MonoWideString(hProcess, _editorAbsolutePath);

        printf("Sphere[%08X] %s\n", entry, _editorAbsolutePathstr);

        DWORD _tokens = Read32DWORD(hProcess, entry + 0x28);
        DWORD _tokensArray = Read32DWORD(hProcess, _tokens + 0x8);
        DWORD _tokensCount = Read32DWORD(hProcess, _tokensArray + 0xC);
        for ( int j = 0 ; j < _tokensCount ; ++j ) {
            DWORD token = Read32DWORD(hProcess, _tokensArray + 0x10 + j * 4);
            if ( token == 0 ) continue;

            DWORD fullpath = Read32DWORD(hProcess, token + 0x24);
            char* fullpathstr = Read32MonoWideString(hProcess, fullpath);
            DWORD payload = Read32DWORD(hProcess, token + 0x28);
            printf("  Token[%08X] %s\n", token, fullpathstr);

            DWORD payloadvtable = Read32DWORD(hProcess, payload);
            DWORD payloadmonoclass = Read32DWORD(hProcess, payloadvtable);
            DWORD payloadmonoclassname = Read32DWORD(hProcess, payloadmonoclass + 0x2C);
            char* payloadmonoclassnamestr = Read32UTF8String(hProcess, payloadmonoclassname);

            if ( strcmp(payloadmonoclassnamestr, "ElementStack") == 0 ) {
                DumpElementStack(hProcess, payload);
            } else if ( strcmp(payloadmonoclassnamestr, "Situation") == 0 ) {
                DumpSituation(hProcess, payload);
            } else if ( strcmp(payloadmonoclassnamestr, "Way") == 0 ) {
                printf("        %s ?\n", payloadmonoclassnamestr);
            }

            free(payloadmonoclassnamestr);
            free(fullpathstr);
        }

        free(_editorAbsolutePathstr);
    }
}
int DumpStageHand ( HANDLE hProcess, DWORD stagehand ) {
    printf("StageHand: %08X\n", stagehand);
    
    DWORD gamePersistenceProvider = Read32DWORD(hProcess, stagehand + 0x10);

    printf("  gamePersistenceProvider: %08X\n", gamePersistenceProvider);
}
int DumpCompendium ( HANDLE hProcess, DWORD compendium ) {
    printf("Compendium: %08X\n", compendium);

    DWORD entityStores = Read32DWORD(hProcess, compendium + 0x8);
    DWORD _pastLevers = Read32DWORD(hProcess, compendium + 0xC);

    printf("  entityStores: %08X\n", entityStores);
    printf("  _pastLevers: %08X\n", _pastLevers);
}
int DumpStable ( HANDLE hProcess, DWORD stable ) {
    printf("Stable: %08X\n", stable);

    DWORD _protagonist = Read32DWORD(hProcess, stable + 0xC);

    printf("  _protagonist: %08X\n", _protagonist);
}
int DumpSecretHistory ( HANDLE hProcess, DWORD secrethistory ) {
    printf("SecretHistory: %08X\n", secrethistory);

    DWORD entries = Read32DWORD(hProcess, secrethistory + 0x18);

    printf("  entries: %08X\n", entries);
}
int DumpMeniscate ( HANDLE hProcess, DWORD meniscate ) {
    printf("Meniscate: %08X\n", meniscate);
  
    DWORD cardHoverDetail = Read32DWORD(hProcess, meniscate + 0xC);
    DWORD _currentlyDraggedToken = Read32DWORD(hProcess, meniscate + 0x14);

    printf("  cardHoverDetail: %08X\n", cardHoverDetail);
    printf("  _currentlyDraggedToken: %08X\n", _currentlyDraggedToken);
}
int DumpXamanek ( HANDLE hProcess, DWORD xamanek ) {
    printf("Xamanek: %08X\n", xamanek);

    DWORD _itineraries = Read32DWORD(hProcess, xamanek + 0xC);
    DWORD _sphereBlocks = Read32DWORD(hProcess, xamanek + 0x10);

    printf("  _itineraries: %08X\n", _itineraries);
    printf("  _sphereBlocks: %08X\n", _sphereBlocks);
}
int DumpNuma ( HANDLE hProcess, DWORD numa ) {
    printf("Numa: %08X\n", numa);

    DWORD Otherworlds = Read32DWORD(hProcess, numa + 0xC);
    DWORD _currentOtherworld = Read32DWORD(hProcess, numa + 0x10);

    printf("  Otherworlds: %08X\n", Otherworlds);
    printf("  _currentOtherworld: %08X\n", _currentOtherworld);
}

int DumpSituation ( HANDLE hProcess, DWORD situation ) {
    DWORD currentRecipe = Read32DWORD(hProcess, situation + 0x14); // Recipe
    DWORD verb = Read32DWORD(hProcess, situation + 0x1C); // Verb
    DWORD id = Read32DWORD(hProcess, situation + 0x20); // String
    char* idstr = Read32MonoWideString(hProcess, id);
    DWORD state = Read32DWORD(hProcess, situation + 0x28); // SituationState
    // DWORD _token = Read32DWORD(hProcess, situation + 0x40); // Token

    printf("    Situation[%08X]:\n", situation);
    printf("      currentRecipe: %08X\n", currentRecipe);
    printf("      verb[%08X]\n", verb);
    printf("      id[%08X]: %s\n", id, idstr);
    printf("      state[%08X]\n", state);

    free(idstr);
}
int DumpElementStack ( HANDLE hProcess, DWORD elementstack ) {
    DWORD elementstackid = Read32DWORD(hProcess, elementstack + 0x10);
    char* elementstackidstr = Read32MonoWideString(hProcess, elementstackid);
    DWORD elementstackelement = Read32DWORD(hProcess, elementstack + 0x14);
    DWORD elementstackquantity = Read32DWORD(hProcess, elementstack + 0x40);
    printf("    ElementStack[%08X]: %dx %s\n", elementstack, elementstackquantity, elementstackidstr);
    free(elementstackidstr);
}

// Note to self; dont arrest EVERYONE

/*
    mystique; reputation 1
    notoriety; reputation 2

    painthasinfluence: insight +1
    painthaslore: insight +1 notoriety +1
    
    paintaddpigment

    paintluck 30% repeating: insight +1

    paintfourpassion: insight +4
    paintthreepassion: insight +3
    painttwopassion: insight +2
    paintonepassion: insight +1
    paintnopassion: insight -1

    painthasfunds: insight +1

    paintresults: restlessness -1

    {
    paintingqualityglorious: <insight 8> paintingresults 6 fascination 1 stale -1
    paintingqualitymagnificent: <insight 6> paintingresults 5 fascination 1 stale -1
    paintingqualityexceptional: <insight 4> paintingresults 4 stale -1
    paintingqualitygood: <insight 2> paintingresults 3
    paintingqualitycompetent: <insight 1> paintingresults 2
    paintingqualityfailure: <insight -1>
    }
    [ 7 glimmering, 5 contentment, 7 mystique]

    {
    paintingreputationcheck: insight -1
    paintingreputation_celebrated: <reputation 9, stale -1> paintingresultsreputation 7
    paintingreputation_famed: <reputation 7, stale -2> paintingresultsreputation 6
    paintingreputation_notable: <reputation 5, stale -3> paintingresultsreputation 4
    paintingreputation_significant: <reputation 3, stale -4> paintingresultsreputation 3
    paintingreputation_known: <reputation 2, stale -5> paintingresultsreputation 2, overexposure decays
    paintingreputation_noticed: <reputation 1> paintingresultsreputation 1, overexposure decays
    paintingreputation_unknown: <reputation -1>
    }
    [ 4 overexposure, 12 funds, 4 mystique ]

    painting.reputation.overexposure: <overexposure 1> overexposure -1 stale 1

    => Paint with 3 Notoriety, 1 Mystique, 1 Lore, 1 Funds
    paintingresult 2
    paintingresultsreputation 7

    => Paint with 4 passion
    paintingresult 4
    stale -1
    paintingresultsreputation if we get mystique
*/
