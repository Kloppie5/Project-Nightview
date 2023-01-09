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
    
    DWORD HornedAxe = WatchmanGet(hProcess, watchman, "HornedAxe");
    DumpHornedAxe(hProcess, HornedAxe);
        // Sphere manager
    DWORD StageHand = WatchmanGet(hProcess, watchman, "StageHand"); // Scene manager
    DWORD Compendium = WatchmanGet(hProcess, watchman, "Compendium"); // Container for elements, verbs, recipes, etc.
    DWORD Stable = WatchmanGet(hProcess, watchman, "Stable");
    DumpStable(hProcess, Stable);
        // Character selection
    DWORD SecretHistory = WatchmanGet(hProcess, watchman, "SecretHistory"); // Secret console messages
    DWORD Meniscate = WatchmanGet(hProcess, watchman, "Meniscate");
    DumpMeniscate(hProcess, Meniscate);
        // 
    DWORD Xamanek = WatchmanGet(hProcess, watchman, "Xamanek");
    DumpXamanek(hProcess, Xamanek);
        // Itinerary and SphereBlock
    DWORD Numa = WatchmanGet(hProcess, watchman, "Numa");
    DumpNuma(hProcess, Numa);
        // Otherworld manager
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

        printf("    Sphere[%08X] %s\n", entry, _editorAbsolutePathstr);

        DWORD _tokens = Read32DWORD(hProcess, entry + 0x28);
        DWORD _tokensArray = Read32DWORD(hProcess, _tokens + 0x8);
        DWORD _tokensCount = Read32DWORD(hProcess, _tokensArray + 0xC);
        for ( int j = 0 ; j < _tokensCount ; ++j ) {
            DWORD token = Read32DWORD(hProcess, _tokensArray + 0x10 + j * 4);
            if ( token == 0 ) continue;

            DWORD fullpath = Read32DWORD(hProcess, token + 0x24);
            char* fullpathstr = Read32MonoWideString(hProcess, fullpath);
            DWORD payload = Read32DWORD(hProcess, token + 0x28);
            printf("      Token[%08X] %s: %08X\n", token, fullpathstr);

            DWORD payloadvtable = Read32DWORD(hProcess, payload);
            DWORD payloadmonoclass = Read32DWORD(hProcess, payloadvtable);
            DWORD payloadmonoclassname = Read32DWORD(hProcess, payloadmonoclass + 0x2C);
            char* payloadmonoclassnamestr = Read32UTF8String(hProcess, payloadmonoclassname);

            if ( strcmp(payloadmonoclassnamestr, "ElementStack") == 0 ) {
                DWORD elementstackid = Read32DWORD(hProcess, payload + 0x10);
                char* elementstackidstr = Read32MonoWideString(hProcess, elementstackid);
                DWORD elementstackelement = Read32DWORD(hProcess, payload + 0x14);
                DWORD elementstackquantity = Read32DWORD(hProcess, payload + 0x40);
                printf("        ElementStack[%08X] %dx %s [%08X]\n", payload, elementstackquantity, elementstackidstr, elementstackelement);
                free(elementstackidstr);
            } else {
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
    DWORD state = Read32DWORD(hProcess, situation + 0x28); // SituationState
    DWORD _token = Read32DWORD(hProcess, situation + 0x40); // Token

    printf("    currentRecipe: %08X\n", currentRecipe);
    printf("    verb: %08X\n", verb);
    printf("    id: %08X\n", id);
    printf("    state: %08X\n", state);
    printf("    _token: %08X\n", _token);
}
