#include <stdarg.h>
#include <stdio.h>
#include <windows.h>

#include "golemancy.h"
#include "process_handling.h"
#include "mono_handling.h"
#include "memory_handling.h"

int Something ( ) {
    HANDLE hProcess = FindProcessByExecutable("C:\\Program Files (x86)\\Steam\\steamapps\\common\\Cultist Simulator\\cultistsimulator.exe");
    
    DWORD watchman = GetWatchmanClass(hProcess);
    DWORD hornedaxe = WatchmanGet(hProcess, watchman, "HornedAxe");

    DWORD tabletop = HornedAxeGetSphereById(hProcess, hornedaxe, "~/tabletop");
    /*
    ~/tabletop!ascensionenlightenmentc
    ~/tabletop!cultlantern_1

    ~/tabletop!dream
    ~/tabletop!study
    ~/tabletop!talk
    ~/tabletop!work
    ~/tabletop!explore
   
    ~/tabletop!time
    ~/tabletop!suspicion

    ~/tabletop!health
    ~/tabletop!skillhealtha
    ~/tabletop!passion
    ~/tabletop!skillpassiona
    ~/tabletop!reason
    ~/tabletop!skillreasona
    ~/tabletop!erudition
    ~/tabletop!funds

    ~/tabletop!fragmentsecrethistories
    ~/tabletop!fragmentsecrethistoriesb
    ~/tabletop!fragmentsecrethistoriesc
    ~/tabletop!fragmentsecrethistoriesd
    ~/tabletop!fragmentlantern
    ~/tabletop!fragmentlanternd
    ~/tabletop!fragmentmoth
    ~/tabletop!fragmentmothb
    ~/tabletop!fragmentgrail
    ~/tabletop!fragmentgrailb
    ~/tabletop!fragmentwinter
    ~/tabletop!fragmentwinterb
    ~/tabletop!fragmentedge
    ~/tabletop!fragmentedgeb
    ~/tabletop!fragmentknock
    ~/tabletop!fragmentknockb
    ~/tabletop!fragmentheart
    ~/tabletop!fragmentheartc
    ~/tabletop!fragmentforge
    ~/tabletop!fragmentforgeb

    ~/tabletop!textbookpassion
    ~/tabletop!textbookreason

    ~/tabletop!locationcabaret
    ~/tabletop!locationbookdealer
    ~/tabletop!locationauctionhouse
    
    ~/tabletop!gloverandgloverjuniorjob
    
    ~/tabletop!generic_a_heart
    ~/tabletop!generic_a_grail
    ~/tabletop!generic_a_lantern
    ~/tabletop!clovette_a
    ~/tabletop!dorothy_a
    ~/tabletop!enid_a
    ~/tabletop!sylvia_a
    ~/tabletop!rose_a
    ~/tabletop!renira_a
    ~/tabletop!saliba_a
    ~/tabletop!clifton_a

    ~/tabletop!waywood
    ~/tabletop!waywhite
    ~/tabletop!waystag_after
    ~/tabletop!wayspider
    ~/tabletop!waypeacock
    
    ~/tabletop!ritetoolfollowerconsumelore
    ~/tabletop!ritetoolconsumefollower

    ~/tabletop!dropzone_situation
    ~/tabletop!dropzone_elementstack
    
    ~/tabletop!vaultcapital2
    ~/tabletop!vaultcapital3
    ~/tabletop!vaultcapital4
    ~/tabletop!vaultlandbeyondforest2
    ~/tabletop!vaultlandbeyondforest3
    ~/tabletop!vaultlandbeyondforest4
    ~/tabletop!vaultshires1
    ~/tabletop!vaultshires2
    ~/tabletop!vaultshires3
    
    ~/tabletop!scholarlatin
    ~/tabletop!scholargreek
    ~/tabletop!scholarsanskrit
    ~/tabletop!scholarfucine
    */
    printf("Tabletop: %08X\n", tabletop);
    int health = SphereGetTokenCountById(hProcess, tabletop, "~/tabletop!health");
    DWORD health_1 = SphereGetTokenById(hProcess, tabletop, "~/tabletop!health", 0);

    printf("Health: %d\n", health);
    
    //MonoInvokeVoid(hProcess, heart, "Beat", 2, (DWORD)10.0f, (DWORD)10.0f);
    //MonoInvokeVoid(hProcess, worksituation, "Open", 0); // Works
    //MonoInvokeVoid(hProcess, worksituation, "TryStart", 0); // Crashes
    //MonoInvokeVoid(hProcess, worksituation, "Conclude", 0); // Initially works but then crashes
    //MonoInvokeVoid(hProcess, worksituation, "Close", 0); // Works
}

// static FucineRoot Get()
//  List<Sphere> _spheres

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

/*
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
int DumpHeart( HANDLE hProcess, DWORD heart ) {
    DWORD vtable = Read32DWORD(hProcess, heart);
    DWORD monoclass = Read32DWORD(hProcess, vtable);

    DWORD gameSpeedState = Read32DWORD(hProcess, heart + 0x10);
    DWORD timerBetweenBeats = Read32DWORD(hProcess, heart + 0x2C);

    float timerBetweenBeatsf = (float)timerBetweenBeats;

    printf("Heart: %08X\n", heart);
    printf("  gameSpeedState: %08X\n", gameSpeedState);
    printf("  timerBetweenBeats: %08X\n", timerBetweenBeats);
    printf("  timerBetweenBeatsf: %f\n", timerBetweenBeatsf);
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
*/

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
