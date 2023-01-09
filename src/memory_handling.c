#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <tchar.h>
#include <stdio.h>

#include "memory_handling.h"

int EnumerateProcesses ( ) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if ( snapshot == INVALID_HANDLE_VALUE ) {
        printf("Failed to create snapshot\n");
        return 1;
    }

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(pe);

    if ( !Process32First(snapshot, &pe) ) {
        printf("Failed to enumerate first process\n");
        return 1;
    }

    do {
        printf("%6ld | %s\n", pe.th32ProcessID, pe.szExeFile);
    } while( Process32Next(snapshot, &pe) );

    CloseHandle(snapshot);

    return 0;
}
HANDLE FindProcessByExecutable ( char* executable ) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if ( snapshot == INVALID_HANDLE_VALUE ) {
        printf("Failed to create snapshot\n");
        return NULL;
    }

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(pe);

    if ( !Process32First(snapshot, &pe) ) {
        printf("Failed to enumerate first process\n");
        return NULL;
    }

    HANDLE hProcess;
    do {
        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe.th32ProcessID);
        if ( hProcess == NULL )
            continue;

        TCHAR filename[MAX_PATH];
        if ( !GetModuleFileNameEx(hProcess, NULL, filename, MAX_PATH) ) {
            printf("Failed to get module filename\n");
            return NULL;
        }

        if ( strcmp(filename, executable) == 0 ) {
            CloseHandle(snapshot);
            return hProcess;
        }
        
        CloseHandle(hProcess);
    } while( Process32Next(snapshot, &pe) );

    CloseHandle(snapshot);
    
    return NULL;
}

int EnumerateModules ( HANDLE hProcess ) {
    HMODULE hModules[1024];
    DWORD cbNeeded;
    if ( EnumProcessModulesEx(hProcess, hModules, sizeof(hModules), &cbNeeded, LIST_MODULES_ALL) ) {
        int numModules = cbNeeded / sizeof(HMODULE);

        for ( int i = 0; i < numModules; ++i ) {
            TCHAR lpBaseName[MAX_PATH];
            if ( !GetModuleBaseNameA(hProcess, hModules[i], lpBaseName, sizeof(lpBaseName)) ) {
                printf("Failed to get module base name\n");
                return 1;
            }
            TCHAR lpFileName[MAX_PATH];
            if ( !GetModuleFileNameExA(hProcess, hModules[i], lpFileName, sizeof(lpFileName)) ) {
                printf("Failed to get module file name\n");
                return 1;
            }
            MODULEINFO info;
            if ( !GetModuleInformation(hProcess, hModules[i], &info, sizeof(info)) ) {
                printf("Failed to get module information\n");
                return 1;
            }
            
            // Skip Windows modules
            if ( strstr(lpFileName, "C:\\WINDOWS\\") != NULL || strstr(lpFileName, "C:\\Windows\\") != NULL )
                continue;
            
            // Skip Steam modules
            if ( strstr(lpFileName, "C:\\Program Files (x86)\\Steam\\") != NULL && strstr(lpFileName, "C:\\Program Files (x86)\\Steam\\steamapps\\" ) == NULL )
                continue;
            
            printf("0x%08X-0x%08X | %s (%s)\n", info.lpBaseOfDll, info.lpBaseOfDll + info.SizeOfImage, lpBaseName, lpFileName);
        }
    }
    return 0;
}
HMODULE FindModuleByName ( HANDLE hProcess, char* name ) {
    HMODULE hModules[1024];
    DWORD cbNeeded;
    if ( EnumProcessModulesEx(hProcess, hModules, sizeof(hModules), &cbNeeded, LIST_MODULES_ALL) ) {
        int numModules = cbNeeded / sizeof(HMODULE);

        for ( int i = 0; i < numModules; ++i ) {
            TCHAR lpBaseName[MAX_PATH];
            if ( !GetModuleBaseNameA(hProcess, hModules[i], lpBaseName, sizeof(lpBaseName)) )
                continue;
            
            if ( strcmp(lpBaseName, name) == 0 ) {
                return hModules[i];
            }
        }
    }
    return NULL;
}

int EnumerateExportTable ( HANDLE hProcess, LPCVOID lpBaseAddress ) {
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)malloc(sizeof(IMAGE_DOS_HEADER));
    ReadProcessMemory(hProcess, lpBaseAddress, dosHeader, sizeof(IMAGE_DOS_HEADER), NULL);
    
    PIMAGE_NT_HEADERS32 ntHeader32 = (PIMAGE_NT_HEADERS32)malloc(sizeof(IMAGE_NT_HEADERS32));
    ReadProcessMemory(hProcess, (LPCVOID)(lpBaseAddress + dosHeader->e_lfanew), ntHeader32, sizeof(IMAGE_NT_HEADERS32), NULL);
    
    PIMAGE_EXPORT_DIRECTORY exportDirectory = (PIMAGE_EXPORT_DIRECTORY)malloc(sizeof(IMAGE_EXPORT_DIRECTORY));
    ReadProcessMemory(hProcess, (LPCVOID)(lpBaseAddress + ntHeader32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress), exportDirectory, sizeof(IMAGE_EXPORT_DIRECTORY), NULL);
    
    DWORD* names = (DWORD*)malloc(exportDirectory->NumberOfNames * sizeof(DWORD));
    ReadProcessMemory(hProcess, (LPCVOID)(lpBaseAddress + exportDirectory->AddressOfNames), names, exportDirectory->NumberOfNames * sizeof(DWORD), NULL);
    WORD* ordinals = (WORD*)malloc(exportDirectory->NumberOfNames * sizeof(WORD));
    ReadProcessMemory(hProcess, (LPCVOID)(lpBaseAddress + exportDirectory->AddressOfNameOrdinals), ordinals, exportDirectory->NumberOfNames * sizeof(WORD), NULL);
    DWORD* addresses = (DWORD*)malloc(exportDirectory->NumberOfNames * sizeof(DWORD));
    ReadProcessMemory(hProcess, (LPCVOID)(lpBaseAddress + exportDirectory->AddressOfFunctions), addresses, exportDirectory->NumberOfNames * sizeof(DWORD), NULL);

    for ( int i = 0; i < exportDirectory->NumberOfNames; ++i ) {
        char* name = (char*)malloc(256);
        ReadProcessMemory(hProcess, (LPCVOID)(lpBaseAddress + names[i]), name, 256, NULL);
        printf("%04X | %08X | %s\n", ordinals[i], addresses[ordinals[i]], name);
        free(name);
    }

    free(names);
    free(exportDirectory);
    free(ntHeader32);
    free(dosHeader);

    return 0;
}
DWORD FindExportByName ( HANDLE hProcess, LPCVOID lpBaseAddress, char* name ) {
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)malloc(sizeof(IMAGE_DOS_HEADER));
    ReadProcessMemory(hProcess, lpBaseAddress, dosHeader, sizeof(IMAGE_DOS_HEADER), NULL);
    
    PIMAGE_NT_HEADERS32 ntHeader32 = (PIMAGE_NT_HEADERS32)malloc(sizeof(IMAGE_NT_HEADERS32));
    ReadProcessMemory(hProcess, (LPCVOID)(lpBaseAddress + dosHeader->e_lfanew), ntHeader32, sizeof(IMAGE_NT_HEADERS32), NULL);
    
    PIMAGE_EXPORT_DIRECTORY exportDirectory = (PIMAGE_EXPORT_DIRECTORY)malloc(sizeof(IMAGE_EXPORT_DIRECTORY));
    ReadProcessMemory(hProcess, (LPCVOID)(lpBaseAddress + ntHeader32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress), exportDirectory, sizeof(IMAGE_EXPORT_DIRECTORY), NULL);
    
    DWORD* names = (DWORD*)malloc(exportDirectory->NumberOfNames * sizeof(DWORD));
    ReadProcessMemory(hProcess, (LPCVOID)(lpBaseAddress + exportDirectory->AddressOfNames), names, exportDirectory->NumberOfNames * sizeof(DWORD), NULL);
    WORD* ordinals = (WORD*)malloc(exportDirectory->NumberOfNames * sizeof(WORD));
    ReadProcessMemory(hProcess, (LPCVOID)(lpBaseAddress + exportDirectory->AddressOfNameOrdinals), ordinals, exportDirectory->NumberOfNames * sizeof(WORD), NULL);
    DWORD* addresses = (DWORD*)malloc(exportDirectory->NumberOfNames * sizeof(DWORD));
    ReadProcessMemory(hProcess, (LPCVOID)(lpBaseAddress + exportDirectory->AddressOfFunctions), addresses, exportDirectory->NumberOfNames * sizeof(DWORD), NULL);

    DWORD address = 0;
    for ( int i = 0; i < exportDirectory->NumberOfNames; ++i ) {
        char* exportName = (char*)malloc(256);
        ReadProcessMemory(hProcess, (LPCVOID)(lpBaseAddress + names[i]), exportName, 256, NULL);
        if ( strcmp(exportName, name) == 0 ) {
            address = addresses[ordinals[i]];
            free(exportName);
            break;
        }
        free(exportName);
    }

    free(names);
    free(exportDirectory);
    free(ntHeader32);
    free(dosHeader);

    return address;
}

BYTE Read32BYTE ( HANDLE hProcess, DWORD address ) {
    BYTE result = 0;
    SIZE_T lpNumberOfBytesRead;
    if ( !ReadProcessMemory(hProcess, (LPCVOID)address, &result, sizeof(BYTE), &lpNumberOfBytesRead) ) {
        printf("Failed to read process memory; %d\n", GetLastError());
        return 0;
    }
    return result;
}
DWORD Read32DWORD ( HANDLE hProcess, DWORD address ) {
    DWORD result = 0;
    SIZE_T lpNumberOfBytesRead;
    if ( !ReadProcessMemory(hProcess, (LPCVOID)address, &result, sizeof(DWORD), &lpNumberOfBytesRead) ) {
        printf("Failed to read process memory; %d\n", GetLastError());
        return 0;
    }
    return result;
}
char* Read32UTF8String ( HANDLE hProcess, DWORD address ) {
    char* result = (char*)malloc(256);
    SIZE_T lpNumberOfBytesRead;
    if ( !ReadProcessMemory(hProcess, (LPCVOID)address, result, 256, &lpNumberOfBytesRead) ) {
        printf("Failed to read process memory; %d\n", GetLastError());
        return NULL;
    }
    return result;
}
char* Read32MonoWideString ( HANDLE hProcess, DWORD address ) {
    DWORD length = Read32DWORD(hProcess, address + 0x8);
    SIZE_T lpNumberOfBytesRead;
    char* result = (char*)malloc(length + 1);
    for ( int i = 0; i < length; ++i ) {
        result[i] = Read32BYTE(hProcess, address + 0xC + i*2);
    }
    result[length] = 0;
    return result;
}

int HexDump ( HANDLE hProcess, LPCVOID lpBaseAddress, SIZE_T nSize ) {
    LPVOID lpBuffer = (LPVOID)malloc(nSize);
    if ( lpBuffer == NULL ) {
        printf("Failed to allocate buffer\n");
        return 1;
    }

    SIZE_T lpNumberOfBytesRead;
    if (!ReadProcessMemory(hProcess, lpBaseAddress, lpBuffer, nSize, &lpNumberOfBytesRead)) {
        printf("Failed to read process memory\n");
        return 1;
    }

    for ( int line = 0; line < lpNumberOfBytesRead; line += 32 ) {
        printf("%08X: ", lpBaseAddress+line);
        for ( int i = 0; i < 32; ++i ) {
            if ( line + i < lpNumberOfBytesRead )
                printf("%02X ", ((unsigned char*)lpBuffer)[line + i]);
            else
                printf("   ");
        }
        printf(" | ");
        for ( int i = 0; i < 32; ++i ) {
            if ( line + i < lpNumberOfBytesRead ) {
                if ( ((unsigned char*)lpBuffer)[line + i] >= 32 && ((unsigned char*)lpBuffer)[line + i] <= 126 )
                    printf("%c", ((unsigned char*)lpBuffer)[line + i]);
                else
                    printf(".");
            }
            else
                printf(" ");
        }
        printf("\n");
    }

    free(lpBuffer);
}

// --------------------
// Mono
// --------------------

// Domain
DWORD ReadRootMonoDomain32 ( HANDLE hProcess ) {
    HMODULE hModule = FindModuleByName(hProcess, "mono-2.0-bdwgc.dll");
    DWORD func_address = FindExportByName(hProcess, hModule, "mono_get_root_domain");
    // mono-2.0-bdwgc.mono_get_root_domain
    //   A1 ???????? | mov eax, [????????]
    //   C3          | ret
    DWORD rootdomainpointerpointer = Read32DWORD(hProcess, (DWORD)hModule + func_address + 1);
    DWORD rootdomainpointer = Read32DWORD(hProcess, rootdomainpointerpointer);

    return rootdomainpointer;
}
DWORD MonoDomain32GetAssemblyList ( HANDLE hProcess, DWORD monodomain ) {
    HMODULE hModule = FindModuleByName(hProcess, "mono-2.0-bdwgc.dll");
    DWORD func_address = FindExportByName(hProcess, hModule, "mono_domain_assembly_foreach");
    // mono-2.0-bdwgc.mono_domain_assembly_foreach
    //   55 8B EC 53 8B 5D 08 8B CB 56 57 | stack management
    //   E8 ???????? | call ????????
    //   8B 73 ??    | mov esi, [ebx+??]
    //   85 F6       | test esi, esi
    BYTE offset = Read32BYTE(hProcess, (DWORD)hModule + func_address + 0x12);
    
    DWORD assemblylistpointer = Read32DWORD(hProcess, monodomain + offset);

    return assemblylistpointer;
}
int MonoDomain32EnumerateAssemblies ( HANDLE hProcess, DWORD address ) {
    DWORD assemblylistpointer = MonoDomain32GetAssemblyList(hProcess, address);
    while ( assemblylistpointer != 0 ) {
        DWORD assemblypointer = Read32DWORD(hProcess, assemblylistpointer);
        DWORD assemblynamepointer = MonoAssembly32GetNameInternal(hProcess, assemblypointer);
        char* assemblyname = Read32UTF8String(hProcess, assemblynamepointer);
        printf("Assembly [%08X]: %s\n", assemblypointer, assemblyname);
        free(assemblyname);
        assemblylistpointer = Read32DWORD(hProcess, assemblylistpointer + 4);
    }
    return 0;
}
DWORD MonoDomain32GetAssemblyByName ( HANDLE hProcess, DWORD monodomain, char* name ) {
    DWORD assemblylistpointer = MonoDomain32GetAssemblyList(hProcess, monodomain);
    while ( assemblylistpointer != 0 ) {
        DWORD assemblypointer = Read32DWORD(hProcess, assemblylistpointer);
        DWORD assemblynamepointer = MonoAssembly32GetNameInternal(hProcess, assemblypointer);
        char* assemblyname = Read32UTF8String(hProcess, assemblynamepointer);
        if ( strcmp(assemblyname, name) == 0 ) {
            free(assemblyname);
            return assemblypointer;
        }
        free(assemblyname);
        assemblylistpointer = Read32DWORD(hProcess, assemblylistpointer + 4);
    }
    return 0;
}
DWORD MonoDomain32GetFriendlyName ( HANDLE hProcess, DWORD monodomain ) {
    HMODULE hModule = FindModuleByName(hProcess, "mono-2.0-bdwgc.dll");
    DWORD func_address = FindExportByName(hProcess, hModule, "mono_domain_get_friendly_name");
    // mono-2.0-bdwgc.mono_domain_get_friendly_name
    //   55 8B EC 8B 45 08 | stack management
    //   8B 40 ??          | mov eax, [eax+??]
    //   5D C3             | stack management
    BYTE offset = Read32BYTE(hProcess, (DWORD)hModule + func_address + 0x8);

    DWORD friendlynamepointer = Read32DWORD(hProcess, monodomain + offset);

    return friendlynamepointer;
}

// Assembly
DWORD MonoAssembly32GetNameInternal ( HANDLE hProcess, DWORD monoassembly ) {
    HMODULE hModule = FindModuleByName(hProcess, "mono-2.0-bdwgc.dll");
    DWORD func_address = FindExportByName(hProcess, hModule, "mono_assembly_get_name_internal");
    // mono-2.0-bdwgc.mono_assembly_get_name_internal
    //   55 8B EC 8B 45 08 | stack management
    //   83 C0 ??          | add eax, ??
    //   5D C3             | stack management
    BYTE offset = Read32BYTE(hProcess, (DWORD)hModule + func_address + 0x8);

    DWORD nameinternalpointer = Read32DWORD(hProcess, monoassembly + offset);

    return nameinternalpointer;
}
DWORD MonoAssembly32GetImage ( HANDLE hProcess, DWORD monoassembly ) {
    HMODULE hModule = FindModuleByName(hProcess, "mono-2.0-bdwgc.dll");
    DWORD func_address = FindExportByName(hProcess, hModule, "mono_assembly_get_image");
    // mono-2.0-bdwgc.mono_assembly_get_image
    // +86 | 8B 78 ?? | mov edi,[eax+??]
    BYTE offset = Read32BYTE(hProcess, (DWORD)hModule + func_address + 0x88);

    DWORD imagepointer = Read32DWORD(hProcess, monoassembly + offset);

    return imagepointer;
}

// Image
DWORD MonoImage32GetName ( HANDLE hProcess, DWORD monoimage ) {
    HMODULE hModule = FindModuleByName(hProcess, "mono-2.0-bdwgc.dll");
    DWORD func_address = FindExportByName(hProcess, hModule, "mono_image_get_name");
    // mono-2.0-bdwgc.mono_image_get_name
    //   55 8B EC 8B 45 08 | stack management
    //   8B 40 ??          | mov eax, [eax+??]
    //   5D C3             | stack management
    BYTE offset = Read32BYTE(hProcess, (DWORD)hModule + func_address + 0x8);

    DWORD namepointer = Read32DWORD(hProcess, monoimage + offset);

    return namepointer;
}
DWORD MonoImage32GetClassCache ( HANDLE hProcess, DWORD monoimage ) {
    HMODULE hModule = FindModuleByName(hProcess, "mono-2.0-bdwgc.dll");
    DWORD func_address = FindExportByName(hProcess, hModule, "mono_image_get_assembly");
    // mono-2.0-bdwgc.mono_image_get_assembly
    // +6 | 8B 80 ???????? |  mov eax,[eax+????????]
    DWORD offset = Read32DWORD(hProcess, (DWORD)hModule + func_address + 0x8);

    DWORD classcache = monoimage + offset + 0x8;

    return classcache;
}
int EnumerateMonoInternalHashTable ( HANDLE hProcess, DWORD monointernalhashtable ) {
    HexDump(hProcess, monointernalhashtable, 0x100);

    // DWORD hash_func = Read32DWORD(hProcess, monointernalhashtable + 0x00); // Identity
    // DWORD key_extract = Read32DWORD(hProcess, monointernalhashtable + 0x04); // 8B 40 34 | mov eax, [eax+34] { type_token }
    // DWORD next_value = Read32DWORD(hProcess, monointernalhashtable + 0x08); // 05 A0000000 | add eax, 000000A0 { 160 }
    DWORD size = Read32DWORD(hProcess, monointernalhashtable + 0x0C);
    // DWORD num_entries = Read32DWORD(hProcess, monointernalhashtable + 0x10);
    DWORD table = Read32DWORD(hProcess, monointernalhashtable + 0x14);

    for ( int i = 0 ; i < size ; ++i ) {
        DWORD entry = Read32DWORD(hProcess, table + i * 4);
        DWORD namepointer = MonoClass32GetName(hProcess, entry);
        char* name = Read32UTF8String(hProcess, namepointer);
        DWORD namespacepointer = MonoClass32GetNamespace(hProcess, entry);
        char* namespace = Read32UTF8String(hProcess, namespacepointer);
        printf("Entry [%08X]: %s . %s\n", entry, namespace, name);
        free(name);
        free(namespace);
    }

    return 0;
}
DWORD MonoImage32GetClassByName ( HANDLE hProcess, DWORD monoimage, char* namespace, char* name ) {
    DWORD classcache = MonoImage32GetClassCache(hProcess, monoimage);

    DWORD size = Read32DWORD(hProcess, classcache + 0x0C);
    DWORD table = Read32DWORD(hProcess, classcache + 0x14);

    for ( int i = 0 ; i < size ; ++i ) {
        DWORD entry = Read32DWORD(hProcess, table + i * 4);
        DWORD entrynamepointer = MonoClass32GetName(hProcess, entry);
        char* entryname = Read32UTF8String(hProcess, entrynamepointer);
        DWORD entrynamespacepointer = MonoClass32GetNamespace(hProcess, entry);
        char* entrynamespace = Read32UTF8String(hProcess, entrynamespacepointer);
        if ( strcmp(entrynamespace, namespace) == 0 && strcmp(entryname, name) == 0 ) {
            free(entryname);
            free(entrynamespace);
            return entry;
        }
        free(entryname);
        free(entrynamespace);
    }
    
    return 0;
}

// Class
DWORD MonoClass32GetName ( HANDLE hProcess, DWORD monoclass ) {
    HMODULE hModule = FindModuleByName(hProcess, "mono-2.0-bdwgc.dll");
    DWORD func_address = FindExportByName(hProcess, hModule, "mono_class_get_name");
    // mono-2.0-bdwgc.mono_assembly_get_image
    // +86 | 8B 78 ?? | mov edi, [eax+??]
    BYTE offset = Read32BYTE(hProcess, (DWORD)hModule + func_address + 0x88);

    DWORD namepointer = Read32DWORD(hProcess, monoclass + offset);

    return namepointer;
}
DWORD MonoClass32GetNamespace ( HANDLE hProcess, DWORD monoclass ) {
    HMODULE hModule = FindModuleByName(hProcess, "mono-2.0-bdwgc.dll");
    DWORD func_address = FindExportByName(hProcess, hModule, "mono_class_get_namespace");
    // mono-2.0-bdwgc.mono_class_get_namespace
    // +86 | 8B 78 ?? | mov edi, [eax+??]
    BYTE offset = Read32BYTE(hProcess, (DWORD)hModule + func_address + 0x88);

    DWORD namespacepointer = Read32DWORD(hProcess, monoclass + offset);

    return namespacepointer;
}
DWORD MonoClass32GetRuntimeInfo ( HANDLE hProcess, DWORD monoclass ) {
    HMODULE hModule = FindModuleByName(hProcess, "mono-2.0-bdwgc.dll");
    DWORD func_address = FindExportByName(hProcess, hModule, "mono_class_vtable");
    // mono-2.0-bdwgc.mono_class_vtable
    // +CF | 8B 4A ?? | mov ecx, [edx+??]
    BYTE offset = Read32BYTE(hProcess, (DWORD)hModule + func_address + 0xD1);

    DWORD runtimeinfopointer = Read32DWORD(hProcess, monoclass + offset);

    return runtimeinfopointer;
}
DWORD MonoClass32GetVTable ( HANDLE hProcess, DWORD monoclass ) {
    DWORD runtimeinfopointer = MonoClass32GetRuntimeInfo(hProcess, monoclass);

    DWORD vtable = Read32DWORD(hProcess, runtimeinfopointer + 0x4);

    return vtable;
}
DWORD MonoClass32GetVTableSize ( HANDLE hProcess, DWORD monoclass ) {
    HMODULE hModule = FindModuleByName(hProcess, "mono-2.0-bdwgc.dll");
    DWORD func_address = FindExportByName(hProcess, hModule, "mono_vtable_get_static_field_data");
    // mono-2.0-bdwgc.mono_vtable_get_static_field_data
    // +12 | 8B 41 ?? | mov eax, [ecx+??]
    BYTE offset = Read32BYTE(hProcess, (DWORD)hModule + func_address + 0x14);

    DWORD vtable_size = Read32DWORD(hProcess, monoclass + offset);

    return vtable_size;
}
DWORD MonoClass32GetStaticFieldData ( HANDLE hProcess, DWORD monoclass ) {
    DWORD vtable = MonoClass32GetVTable(hProcess, monoclass);
    DWORD vtable_size = MonoClass32GetVTableSize(hProcess, monoclass);

    HMODULE hModule = FindModuleByName(hProcess, "mono-2.0-bdwgc.dll");
    DWORD func_address = FindExportByName(hProcess, hModule, "mono_vtable_get_static_field_data");
    // mono-2.0-bdwgc.mono_vtable_get_static_field_data
    // +15 | 8B 44 82 ?? | mov eax, [edx+eax*4+??]
    BYTE offset = Read32BYTE(hProcess, (DWORD)hModule + func_address + 0x18);

    DWORD staticfielddatapointer = Read32DWORD(hProcess, vtable + vtable_size * 4 + offset);

    return staticfielddatapointer;
}
DWORD MonoClass32GetNumFields ( HANDLE hProcess, DWORD monoclass ) {
    return 100; // TODO
}
DWORD MonoClass32GetFields ( HANDLE hProcess, DWORD monoclass ) {
    // mono-2.0-bdwgc.mono_class_get_field
    // +61 | 8B 57 60 | mov edx, [edi+60]
    DWORD fields = Read32DWORD(hProcess, monoclass + 0x60);

    return fields;
}
int MonoClass32EnumerateMonoClassFields ( HANDLE hProcess, DWORD monoclass ) {
    DWORD fields = MonoClass32GetFields(hProcess, monoclass);
    DWORD numfields = MonoClass32GetNumFields(hProcess, monoclass);

    for ( int i = 0 ; i < numfields ; ++i ) {
        DWORD fieldtype = Read32DWORD(hProcess, fields + i * 0x10);
        if ( fieldtype == 0 ) break;
        DWORD fieldname = Read32DWORD(hProcess, fields + i * 0x10 + 0x4);
        char* fieldnamestr = Read32UTF8String(hProcess, fieldname);
        DWORD fieldparent = Read32DWORD(hProcess, fields + i * 0x10 + 0x8);
        DWORD fieldoffset = Read32DWORD(hProcess, fields + i * 0x10 + 0xC);
        
        printf("Field: %s %08X\n", fieldnamestr, fieldoffset);
        free(fieldnamestr);
    }
}

// VTable
DWORD MonoVTable32GetClass ( HANDLE hProcess, DWORD monovtable ) {
    // mono-2.0-bdwgc.mono_vtable_class
    // +6 | 8B 00 | mov eax, [eax]
    DWORD monoclass = Read32DWORD(hProcess, monovtable);

    return monoclass;
}
DWORD MonoVTable32GetDomain ( HANDLE hProcess, DWORD monovtable ) {
    // mono-2.0-bdwgc.mono_vtable_get_domain
    // +6 | 8B 40 08 | mov eax, [eax+08]
    DWORD domain = Read32DWORD(hProcess, monovtable + 0x8);

    return domain;
}
