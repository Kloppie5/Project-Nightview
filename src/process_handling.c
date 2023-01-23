#include "process_handling.h"
#include "mono_handling.h"
#include "memory_handling.h"

#include <stdio.h>

DWORD GetWatchmanClass ( HANDLE hProcess ) {
    DWORD rootdomainpointer = ReadRootMonoDomain32(hProcess);
    DWORD assembly = MonoDomain32GetAssemblyByName(hProcess, rootdomainpointer, "SecretHistories.Main");
    DWORD assemblyimage = MonoAssembly32GetImage(hProcess, assembly);
    
    DWORD watchman = MonoImage32GetClassByName(hProcess, assemblyimage, "SecretHistories.UI", "Watchman");
    return watchman;
}
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

DWORD HornedAxeGetSphereHashSet ( HANDLE hProcess, DWORD hornedaxe ) {
    return Read32DWORD(hProcess, hornedaxe + 0x1C);
}
DWORD HornedAxeGetSphereById ( HANDLE hProcess, DWORD hornedaxe, char* id ) {
    /*
    limbo | ~/limbo
    windows | ~/windows
    tabletop | ~/tabletop
    enroute | ~/enroute
    
    mansusthresholdwood | ~!mansus/mansusthresholdwood
    wood0 | ~!mansus/wood0
    mansus_wood_draw | ~/mansus_wood_draw
    wood1 | ~!mansus/wood1
    mansus_wood1_draw | ~/mansus_wood1_draw
    wood2 | ~!mansus/wood2
    mansus_wood2_draw | ~/mansus_wood2_draw
    mansusthresholdwhite | ~!mansus/mansusthresholdwhite
    whitedoor0 | ~!mansus/whitedoor0
    mansus_whitedoor_draw | ~/mansus_whitedoor_draw
    whitedoor1 | ~!mansus/whitedoor1
    mansus_whitedoor1_draw | ~/mansus_whitedoor1_draw
    whitedoor2 | ~!mansus/whitedoor2
    mansus_whitedoor2_draw | ~/mansus_whitedoor2_draw
    mansusthresholdstag | ~!mansus/mansusthreshold
    stagstagriddles_draw | ~/stagriddles_draw
    stagdoor0 | ~!mansus/stagdoor0
    mansus_stagdoor_draw | ~/mansus_stagdoor_draw
    stagdoor1 | ~!mansus/stagdoor1
    mansus_stagdoor1_draw | ~/mansus_stagdoor1_draw
    stagdoor2 | ~!mansus/stagdoor2
    mansus_stagdoor2_draw | ~/mansus_stagdoor2_draw
    mansusthresholdspider | ~!mansus/mansusthresholdspider
    spiderdoor0 | ~!mansus/spiderdoor0
    mansus_spiderdoor_draw | ~/mansus_spiderdoor_draw
    spiderdoor1 | ~!mansus/spiderdoor1
    mansus_spiderdoor1_draw | ~/mansus_spiderdoor1_draw
    spiderdoor2 | ~!mansus/spiderdoor2
    mansus_spiderdoor2_draw | ~/mansus_spiderdoor2_draw
    mansusthresholdpeacock | ~!mansus/mansusthresholdpeacock
    peacockdoor0 | ~!mansus/peacockdoor0
    peacockdoor1 | ~!mansus/peacockdoor1
    peacockdoor2 | ~!mansus/peacockdoor2
    mansusthresholdtricuspid | ~!mansus/mansusthresholdtricuspid
    mansusenroute | ~!mansus/mansusenroute
    
    dream | /dream
    study | /study
    talk | /talk
    work | /work
    explore | /explore
    Expenses | /expenses
    Evidence | /evidence
    dropzone_Situationbubble | ~/tabletop!dropzone_situation/dropzone_situationbubble
    dropzone_ElementStackbubble | ~/tabletop!dropzone_elementstack/dropzone_elementstackbubble
    
    seasonevents_draw | ~/seasonevents_draw
    commontomes_draw | ~/commontomes_draw
    exploreevents_draw | ~/exploreevents_draw
    citysecrets_draw | ~/citysecrets_draw
    cabaretevents_draw | ~/cabaretevents_draw
    acquaintances_draw | ~/acquaintances_draw
    hirelings_draw | ~/hirelings_draw
    paintingresultsbasic_draw | ~/paintingresultsbasic_draw
    
    locationscapital_draw | ~/locationscapital_draw
    
    paintingresults_draw | ~/paintingresults_draw
    paintingresultsreputation_draw | ~/paintingresultsreputation_draw
    
    dreamreasonresults_draw | ~/dreamreasonresults_draw
    oneoffrewards_draw | ~/oneoffrewards_draw
    randomvictim_draw | ~/randomvictim_draw

    deck.explorevaultcapital1_success_draw | ~/deck.explorevaultcapital1_success_draw
    deck.explorevaultcapital5_success_draw | ~/deck.explorevaultcapital5_success_draw
    
    vaultsshires_draw | ~/vaultsshires_draw
    auctionedcommontomes_draw | ~/auctionedcommontomes_draw
    vaultslandbeyondforest_draw | ~/vaultslandbeyondforest_draw
    randominfluencelow_draw | ~/randominfluencelow_draw
    */
    DWORD _spheres = HornedAxeGetSphereHashSet(hProcess, hornedaxe);
    DWORD _array = Read32DWORD(hProcess, _spheres + 0xC);
    DWORD _count = Read32DWORD(hProcess, _array + 0xC);
    for ( int i = 0 ; i < _count ; ++i ) {
        DWORD sphere = Read32DWORD(hProcess, _array + 0x10 + i * 0xC + 0x8);
        if ( sphere == 0 ) continue;

        DWORD _editorAbsolutePath = Read32DWORD(hProcess, sphere + 0x1C);
        char* _editorAbsolutePathstr = Read32MonoWideString(hProcess, _editorAbsolutePath);

        // DWORD spherespec = Read32DWORD(hProcess, sphere + 0xC);
        // DWORD sphereid = Read32DWORD(hProcess, spherespec + 0x8);
        // char* sphereidstr = Read32MonoWideString(hProcess, sphereid);
        // free(sphereidstr);

        if ( strcmp(_editorAbsolutePathstr, id) == 0 ) {
            free(_editorAbsolutePathstr);
            return sphere;
        }
        free(_editorAbsolutePathstr);
    }
    return 0;
}
void EnumerateHornedAxeSpheres ( HANDLE hProcess, DWORD hornedaxe ) {
    DWORD _spheres = HornedAxeGetSphereHashSet(hProcess, hornedaxe);
    DWORD _array = Read32DWORD(hProcess, _spheres + 0xC);
    DWORD _count = Read32DWORD(hProcess, _array + 0xC);
    for ( int i = 0 ; i < _count ; ++i ) {
        DWORD sphere = Read32DWORD(hProcess, _array + 0x10 + i * 0xC + 0x8);
        if ( sphere == 0 ) continue;

        char* _editorAbsolutePathstr = SphereGetEditorAbsolutePath(hProcess, sphere);
        printf("%s | ", _editorAbsolutePathstr);
        free(_editorAbsolutePathstr);

        char* _editorWildPathstr = SphereGetEditorWildPath(hProcess, sphere);
        printf("%s\n", _editorWildPathstr);
        free(_editorWildPathstr);
    }
}

char* DominionGetIdentifier ( HANDLE hProcess, DWORD dominion ) {
    DWORD identifier = Read32DWORD(hProcess, dominion + 0x10);
    return Read32MonoWideString(hProcess, identifier);
}
DWORD DominionGetSphereList ( HANDLE hProcess, DWORD dominion ) {
    return Read32DWORD(hProcess, dominion + 0x14);
}
char* SituationDominionGetEditableIdentifier ( HANDLE hProcess, DWORD situationdominion ) {
    DWORD editableidentifier = Read32DWORD(hProcess, situationdominion + 0x34);
    char* editableidentifierstr = Read32MonoWideString(hProcess, editableidentifier);
    return editableidentifierstr;
}

DWORD SphereGetContainer ( HANDLE hProcess, DWORD sphere ) {
    return Read32DWORD(hProcess, sphere + 0x18);
}
char* SphereGetEditorAbsolutePath ( HANDLE hProcess, DWORD sphere ) {
    DWORD _editorAbsolutePath = Read32DWORD(hProcess, sphere + 0x1C);
    char* _editorAbsolutePathstr = Read32MonoWideString(hProcess, _editorAbsolutePath);
    return _editorAbsolutePathstr;
}
char* SphereGetEditorWildPath ( HANDLE hProcess, DWORD sphere ) {
    DWORD _editorWildPath = Read32DWORD(hProcess, sphere + 0x20);
    char* _editorWildPathstr = Read32MonoWideString(hProcess, _editorWildPath);
    return _editorWildPathstr;
}
DWORD SphereGetTokenList ( HANDLE hProcess, DWORD sphere ) {
    return Read32DWORD(hProcess, sphere + 0x2C);
}
void SphereListPrintTokens ( HANDLE hProcess, DWORD spherelist ) {
    DWORD _array = Read32DWORD(hProcess, spherelist + 0x8);
    DWORD _count = Read32DWORD(hProcess, _array + 0xC);
    for ( int i = 0 ; i < _count ; ++i ) {
        DWORD sphere = Read32DWORD(hProcess, _array + 0x10 + i * 0x4);
        if ( sphere == 0 ) continue;
        printf("[%08X]", sphere);

        char* classnamestr = MonoInstanceGetClassName(hProcess, sphere);
        printf(" %s", classnamestr);
        free(classnamestr);

        char* _editorAbsolutePathstr = SphereGetEditorAbsolutePath(hProcess, sphere);
        printf(" %s\n", _editorAbsolutePathstr);
        free(_editorAbsolutePathstr);

        SpherePrintTokens(hProcess, sphere);
    }
}
void SpherePrintTokens ( HANDLE hProcess, DWORD sphere ) {
    DWORD _tokens = SphereGetTokenList(hProcess, sphere);
    DWORD _array = Read32DWORD(hProcess, _tokens + 0x8);
    DWORD _count = Read32DWORD(hProcess, _array + 0xC);
    for ( int i = 0 ; i < _count ; ++i ) {
        DWORD token = Read32DWORD(hProcess, _array + 0x10 + i * 0x4);
        if ( token == 0 ) continue;
        printf("[%08X]", token);

        char* classnamestr = MonoInstanceGetClassName(hProcess, token);
        printf(" %s", classnamestr);
        free(classnamestr);

        char* fullpath = TokenGetFullPath(hProcess, token);
        printf(" %s\n", fullpath);
        free(fullpath);
    }
}
int SphereGetTokenCountById ( HANDLE hProcess, DWORD sphere, char* id ) {
    DWORD _tokens = SphereGetTokenList(hProcess, sphere);
    DWORD _array = Read32DWORD(hProcess, _tokens + 0x8);
    DWORD _count = Read32DWORD(hProcess, _array + 0xC);
    int count = 0;
    for ( int i = 0 ; i < _count ; ++i ) {
        DWORD token = Read32DWORD(hProcess, _array + 0x10 + i * 0x4);
        if ( token == 0 ) continue;

        DWORD fullpathasstring = Read32DWORD(hProcess, token + 0x24);
        char* fullpathasstringstr = Read32MonoWideString(hProcess, fullpathasstring);

        char* lastunderscore = strrchr(fullpathasstringstr, '_');
        if ( strncmp(fullpathasstringstr, id, lastunderscore - fullpathasstringstr) == 0 ) {
            ++count;
        }

        free(fullpathasstringstr);
    }
    return count;
}
DWORD SphereGetTokenById ( HANDLE hProcess, DWORD sphere, char* id, int index ) {
    DWORD _tokens = SphereGetTokenList(hProcess, sphere);
    DWORD _array = Read32DWORD(hProcess, _tokens + 0x8);
    DWORD _count = Read32DWORD(hProcess, _array + 0xC);
    int count = 0;
    for ( int i = 0 ; i < _count ; ++i ) {
        DWORD token = Read32DWORD(hProcess, _array + 0x10 + i * 0x4);
        if ( token == 0 ) continue;

        char* fullpathasstringstr = TokenGetFullPath(hProcess, token);

        char* lastunderscore = strrchr(fullpathasstringstr, '_');
        if ( strncmp(fullpathasstringstr, id, lastunderscore - fullpathasstringstr) == 0 ) {
            if ( count == index ) {
                free(fullpathasstringstr);
                return token;
            }
            ++count;
        }
        free(fullpathasstringstr);
    }
    return 0;
}

char* TokenGetFullPath ( HANDLE hProcess, DWORD token ) {
    DWORD fullpathasstring = Read32DWORD(hProcess, token + 0x28);
    return Read32MonoWideString(hProcess, fullpathasstring);
}
DWORD TokenGetPayload ( HANDLE hProcess, DWORD token ) {
    return Read32DWORD(hProcess, token + 0x2C);
}

DWORD SituationGetDominionList ( HANDLE hProcess, DWORD situation ) {
    return Read32DWORD(hProcess, situation + 0x34);
}
void SituationPrintDominions ( HANDLE hProcess, DWORD situation ) {
    DWORD _registeredDominions = SituationGetDominionList(hProcess, situation);
    DWORD _array = Read32DWORD(hProcess, _registeredDominions + 0x8);
    DWORD _count = Read32DWORD(hProcess, _array + 0xC);
    printf("Registered dominions: %d\n", _count);
    printf("Dominions: %08X\n", _array);
    for ( int i = 0 ; i < _count ; ++i ) {
        DWORD dominion = Read32DWORD(hProcess, _array + 0x10 + i * 0x4);
        if ( dominion == 0 ) continue;
        printf("[%08X]", dominion);

        char* classnamestr = MonoInstanceGetClassName(hProcess, dominion);
        printf(" %s", classnamestr);

        char* identifierstr = DominionGetIdentifier(hProcess, dominion);
        printf(" %s", identifierstr);
        free(identifierstr);

        char* editableidentifierstr = SituationDominionGetEditableIdentifier(hProcess, dominion);
        printf(" %s\n", editableidentifierstr);
        free(editableidentifierstr);

        DWORD _spheres = DominionGetSphereList(hProcess, dominion);
        SphereListPrintTokens(hProcess, _spheres);
    }
}
DWORD SituationGetTimeShadow ( HANDLE hProcess, DWORD situation ) {
    return Read32DWORD(hProcess, situation + 0x3C);
}
void SituationSetTime ( HANDLE hProcess, DWORD situation, float time ) {
    DWORD timeshadow = SituationGetTimeShadow(hProcess, situation);
    Write32FLOAT(hProcess, timeshadow + 0x8, time);
}
