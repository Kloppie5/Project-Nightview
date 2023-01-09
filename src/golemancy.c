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
    DWORD StageHand = WatchmanGet(hProcess, watchman, "StageHand");
    DumpStageHand(hProcess, StageHand);
        // Scene manager
    DWORD Compendium = WatchmanGet(hProcess, watchman, "Compendium");
    DumpCompendium(hProcess, Compendium);
        // Container for elements, verbs, recipes, etc.
    DWORD Stable = WatchmanGet(hProcess, watchman, "Stable");
    DumpStable(hProcess, Stable);
        // Character selection
    DWORD SecretHistory = WatchmanGet(hProcess, watchman, "SecretHistory");
    DumpSecretHistory(hProcess, SecretHistory);
        // Secret console messages
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
    
    DWORD _subscribers = Read32DWORD(hProcess, hornedaxe + 0x8);
    DWORD _tabletopAspects = Read32DWORD(hProcess, hornedaxe + 0xC);
    DWORD _allAspectsExtant = Read32DWORD(hProcess, hornedaxe + 0x10);
    DWORD _currentSituations = Read32DWORD(hProcess, hornedaxe + 0x14);
    DWORD _registeredWays = Read32DWORD(hProcess, hornedaxe + 0x18);
    DWORD _registeredSpheres = Read32DWORD(hProcess, hornedaxe + 0x1C);
    DWORD _defaultSpherePath = Read32DWORD(hProcess, hornedaxe + 0x20);
    DWORD _alternativeDefaultWorldSpherePaths = Read32DWORD(hProcess, hornedaxe + 0x24);
    BYTE EnableAspectCaching = Read32BYTE(hProcess, hornedaxe + 0x28);
    BYTE _tabletopAspectsDirty = Read32BYTE(hProcess, hornedaxe + 0x29);
    BYTE _allAspectsExtantDirty = Read32BYTE(hProcess, hornedaxe + 0x2A);

    printf("_subscribers: %08X\n", _subscribers);
    printf("_tabletopAspects: %08X\n", _tabletopAspects);
    printf("_allAspectsExtant: %08X\n", _allAspectsExtant);
    printf("_currentSituations: %08X\n", _currentSituations);
    printf("_registeredWays: %08X\n", _registeredWays);
    printf("_registeredSpheres: %08X\n", _registeredSpheres);
    printf("_defaultSpherePath: %08X\n", _defaultSpherePath);
    printf("_alternativeDefaultWorldSpherePaths: %08X\n", _alternativeDefaultWorldSpherePaths);
    printf("EnableAspectCaching: %02X\n", EnableAspectCaching);
    printf("_tabletopAspectsDirty: %02X\n", _tabletopAspectsDirty);
    printf("_allAspectsExtantDirty: %02X\n", _allAspectsExtantDirty);
}
int DumpStageHand ( HANDLE hProcess, DWORD stagehand ) {
    printf("StageHand: %08X\n", stagehand);
    
    DWORD fadeOverlay = Read32DWORD(hProcess, stagehand + 0xC);
    DWORD gamePersistenceProvider = Read32DWORD(hProcess, stagehand + 0x10);
    DWORD fadeDuration = Read32DWORD(hProcess, stagehand + 0x14);
    DWORD loadedInfoScene = Read32DWORD(hProcess, stagehand + 0x18);

    printf("fadeOverlay: %08X\n", fadeOverlay);
    printf("gamePersistenceProvider: %08X\n", gamePersistenceProvider);
    printf("fadeDuration: %08X\n", fadeDuration);
    printf("loadedInfoScene: %08X\n", loadedInfoScene);
}
int DumpCompendium ( HANDLE hProcess, DWORD compendium ) {
    printf("Compendium: %08X\n", compendium);

    DWORD entityStores = Read32DWORD(hProcess, compendium + 0x8);
    DWORD _pastLevers = Read32DWORD(hProcess, compendium + 0xC);
    DWORD entityIdsToValidate = Read32DWORD(hProcess, compendium + 0x10);
    DWORD _compendiumNullObjectStore = Read32DWORD(hProcess, compendium + 0x14);
    DWORD elementsInUniquenessGroups = Read32DWORD(hProcess, compendium + 0x18);

    printf("entityStores: %08X\n", entityStores);
    printf("_pastLevers: %08X\n", _pastLevers);
    printf("entityIdsToValidate: %08X\n", entityIdsToValidate);
    printf("_compendiumNullObjectStore: %08X\n", _compendiumNullObjectStore);
    printf("elementsInUniquenessGroups: %08X\n", elementsInUniquenessGroups);
}
int DumpStable ( HANDLE hProcess, DWORD stable ) {
    printf("Stable: %08X\n", stable);

    DWORD _protagonist = Read32DWORD(hProcess, stable + 0xC);

    printf("_protagonist: %08X\n", _protagonist);
}
int DumpSecretHistory ( HANDLE hProcess, DWORD secrethistory ) {
    printf("SecretHistory: %08X\n", secrethistory);

    DWORD LogMessageEntryPrefab = Read32DWORD(hProcess, secrethistory + 0xC);
    DWORD logMessageEntriesHere = Read32DWORD(hProcess, secrethistory + 0x10);
    DWORD canvas = Read32DWORD(hProcess, secrethistory + 0x14);
    DWORD entries = Read32DWORD(hProcess, secrethistory + 0x18);

    printf("LogMessageEntryPrefab: %08X\n", LogMessageEntryPrefab);
    printf("logMessageEntriesHere: %08X\n", logMessageEntriesHere);
    printf("canvas: %08X\n", canvas);
    printf("entries: %08X\n", entries);
}
int DumpMeniscate ( HANDLE hProcess, DWORD meniscate ) {
    printf("Meniscate: %08X\n", meniscate);
  
    DWORD cardHoverDetail = Read32DWORD(hProcess, meniscate + 0xC);
    DWORD _multiSelectedTokens = Read32DWORD(hProcess, meniscate + 0x10);
    DWORD _currentlyDraggedToken = Read32DWORD(hProcess, meniscate + 0x14);
    DWORD GRID_WIDTH = Read32DWORD(hProcess, meniscate + 0x18);
    DWORD GRID_HEIGHT = Read32DWORD(hProcess, meniscate + 0x1C);

    printf("cardHoverDetail: %08X\n", cardHoverDetail);
    printf("_multiSelectedTokens: %08X\n", _multiSelectedTokens);
    printf("_currentlyDraggedToken: %08X\n", _currentlyDraggedToken);
    printf("GRID_WIDTH: %08X\n", GRID_WIDTH);
    printf("GRID_HEIGHT: %08X\n", GRID_HEIGHT);
}
int DumpXamanek ( HANDLE hProcess, DWORD xamanek ) {
    printf("Xamanek: %08X\n", xamanek);

    DWORD _itineraries = Read32DWORD(hProcess, xamanek + 0xC);
    DWORD _sphereBlocks = Read32DWORD(hProcess, xamanek + 0x10);
    DWORD ItinerariesDisplayHolder = Read32DWORD(hProcess, xamanek + 0x14);
    DWORD SphereBlocksDisplayHolder = Read32DWORD(hProcess, xamanek + 0x18);
    DWORD MetapauseWhenItineraryStarted = Read32DWORD(hProcess, xamanek + 0x1C);

    printf("_itineraries: %08X\n", _itineraries);
    printf("_sphereBlocks: %08X\n", _sphereBlocks);
    printf("ItinerariesDisplayHolder: %08X\n", ItinerariesDisplayHolder);
    printf("SphereBlocksDisplayHolder: %08X\n", SphereBlocksDisplayHolder);
    printf("MetapauseWhenItineraryStarted: %08X\n", MetapauseWhenItineraryStarted);
}
int DumpNuma ( HANDLE hProcess, DWORD numa ) {
    printf("Numa: %08X\n", numa);

    DWORD Otherworlds = Read32DWORD(hProcess, numa + 0xC);
    DWORD _currentOtherworld = Read32DWORD(hProcess, numa + 0x10);

    printf("Otherworlds: %08X\n", Otherworlds);
    printf("_currentOtherworld: %08X\n", _currentOtherworld);
}
