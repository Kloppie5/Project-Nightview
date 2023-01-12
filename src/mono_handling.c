#include <stdio.h>

#include "mono_handling.h"
#include "memory_handling.h"

DWORD ReadRootMonoDomain32 ( HANDLE hProcess ) {
    HMODULE hModule = FindModuleByName(hProcess, "mono-2.0-bdwgc.dll");
    DWORD func_address = FindExportByName(hProcess, hModule, "mono_get_root_domain");
    // mono-2.0-bdwgc.mono_get_root_domain
    //   A1 ???????? | mov eax, [????????]
    //   C3          | ret
    DWORD rootdomainpointerpointer = Read32DWORD(hProcess, (DWORD)(intptr_t)hModule + func_address + 1);
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
    BYTE offset = Read32BYTE(hProcess, (DWORD)(intptr_t)hModule + func_address + 0x12);
    
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
    BYTE offset = Read32BYTE(hProcess, (DWORD)(intptr_t)hModule + func_address + 0x8);

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
    BYTE offset = Read32BYTE(hProcess, (DWORD)(intptr_t)hModule + func_address + 0x8);

    DWORD nameinternalpointer = Read32DWORD(hProcess, monoassembly + offset);

    return nameinternalpointer;
}
DWORD MonoAssembly32GetImage ( HANDLE hProcess, DWORD monoassembly ) {
    HMODULE hModule = FindModuleByName(hProcess, "mono-2.0-bdwgc.dll");
    DWORD func_address = FindExportByName(hProcess, hModule, "mono_assembly_get_image");
    // mono-2.0-bdwgc.mono_assembly_get_image
    // +86 | 8B 78 ?? | mov edi,[eax+??]
    BYTE offset = Read32BYTE(hProcess, (DWORD)(intptr_t)hModule + func_address + 0x88);

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
    BYTE offset = Read32BYTE(hProcess, (DWORD)(intptr_t)hModule + func_address + 0x8);

    DWORD namepointer = Read32DWORD(hProcess, monoimage + offset);

    return namepointer;
}
DWORD MonoImage32GetClassCache ( HANDLE hProcess, DWORD monoimage ) {
    HMODULE hModule = FindModuleByName(hProcess, "mono-2.0-bdwgc.dll");
    DWORD func_address = FindExportByName(hProcess, hModule, "mono_image_get_assembly");
    // mono-2.0-bdwgc.mono_image_get_assembly
    // +6 | 8B 80 ???????? |  mov eax,[eax+????????]
    DWORD offset = Read32DWORD(hProcess, (DWORD)(intptr_t)hModule + func_address + 0x8);

    DWORD classcache = monoimage + offset + 0x8;

    return classcache;
}
int EnumerateMonoInternalHashTable ( HANDLE hProcess, DWORD monointernalhashtable ) {
    // DWORD hash_func = Read32DWORD(hProcess, monointernalhashtable + 0x00); // Identity
    // DWORD key_extract = Read32DWORD(hProcess, monointernalhashtable + 0x04); // 8B 40 34 | mov eax, [eax+34] { type_token }
    // DWORD next_value = Read32DWORD(hProcess, monointernalhashtable + 0x08); // 05 A0000000 | add eax, 000000A0 { 160 }
    DWORD size = Read32DWORD(hProcess, monointernalhashtable + 0x0C);
    // DWORD num_entries = Read32DWORD(hProcess, monointernalhashtable + 0x10);
    DWORD table = Read32DWORD(hProcess, monointernalhashtable + 0x14);

    printf("size: %08X\n", size);
    printf("table: %08X\n", table);
    
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
    BYTE offset = Read32BYTE(hProcess, (DWORD)(intptr_t)hModule + func_address + 0x88);

    DWORD namepointer = Read32DWORD(hProcess, monoclass + offset);

    return namepointer;
}
DWORD MonoClass32GetNamespace ( HANDLE hProcess, DWORD monoclass ) {
    HMODULE hModule = FindModuleByName(hProcess, "mono-2.0-bdwgc.dll");
    DWORD func_address = FindExportByName(hProcess, hModule, "mono_class_get_namespace");
    // mono-2.0-bdwgc.mono_class_get_namespace
    // +86 | 8B 78 ?? | mov edi, [eax+??]
    BYTE offset = Read32BYTE(hProcess, (DWORD)(intptr_t)hModule + func_address + 0x88);

    DWORD namespacepointer = Read32DWORD(hProcess, monoclass + offset);

    return namespacepointer;
}
DWORD MonoClass32GetRuntimeInfo ( HANDLE hProcess, DWORD monoclass ) {
    HMODULE hModule = FindModuleByName(hProcess, "mono-2.0-bdwgc.dll");
    DWORD func_address = FindExportByName(hProcess, hModule, "mono_class_vtable");
    // mono-2.0-bdwgc.mono_class_vtable
    // +CF | 8B 4A ?? | mov ecx, [edx+??]
    BYTE offset = Read32BYTE(hProcess, (DWORD)(intptr_t)hModule + func_address + 0xD1);

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
    BYTE offset = Read32BYTE(hProcess, (DWORD)(intptr_t)hModule + func_address + 0x14);

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
    BYTE offset = Read32BYTE(hProcess, (DWORD)(intptr_t)hModule + func_address + 0x18);

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
DWORD MonoClass32GetNumMethods ( HANDLE hProcess, DWORD monoclass ) {
    return 150; // TODO
}
DWORD MonoClass32GetMethods ( HANDLE hProcess, DWORD monoclass ) {
    // mono-2.0-bdwgc.mono_class_get_methods
    // +21 | 8B 5F 64 | mov ebx, [edi+64]
    DWORD methods = Read32DWORD(hProcess, monoclass + 0x64);

    return methods;
}
int MonoClass32EnumerateMonoMethods ( HANDLE hProcess, DWORD monoclass ) {
    DWORD methods = MonoClass32GetMethods(hProcess, monoclass);
    DWORD nummethods = MonoClass32GetNumMethods(hProcess, monoclass);

    for ( int i = 0 ; i < nummethods ; ++i ) {
        DWORD method = Read32DWORD(hProcess, methods + i * 0x4);
        // WORD flags
        // WORD iflags
        // DWORD token
        // DWORD class
        // DWORD MonoMethodSignature*
        DWORD methodname = Read32DWORD(hProcess, method + 0x10);
        char* methodnamestr = Read32UTF8String(hProcess, methodname);
        // WORD more info
        WORD methodslot = Read32WORD(hProcess, method + 0x16);
        printf("Method[%08X] <%d>: %s\n", method, methodslot, methodnamestr);
    }
}
DWORD MonoClass32GetMonoMethodByName ( HANDLE hProcess, DWORD monoclass, char* name ) {
    DWORD methods = MonoClass32GetMethods(hProcess, monoclass);
    DWORD nummethods = MonoClass32GetNumMethods(hProcess, monoclass);

    for ( int i = 0 ; i < nummethods ; ++i ) {
        DWORD method = Read32DWORD(hProcess, methods + i * 0x4);
        // WORD flags
        // WORD iflags
        // DWORD token
        // DWORD class
        // DWORD MonoMethodSignature*
        DWORD methodname = Read32DWORD(hProcess, method + 0x10);
        char* methodnamestr = Read32UTF8String(hProcess, methodname);
        // WORD more info
        // WORD slot

        if ( strcmp(methodnamestr, name) == 0 ) {
            free(methodnamestr);
            return method;
        }
        free(methodnamestr);
    }
    return -1;
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

void MonoInvokeVoid ( HANDLE hProcess, DWORD instance, char* methodname, int argnum, ... ) {
    va_list varargs;
    va_start(varargs, argnum);

    printf("Invoking %s/%d on %08X\n", methodname, argnum, instance);
    DWORD instancevtable = Read32DWORD(hProcess, instance);
    DWORD instanceclass = Read32DWORD(hProcess, instancevtable);

    DWORD rootdomain = ReadRootMonoDomain32(hProcess);
    HMODULE hModule = FindModuleByName(hProcess, "mono-2.0-bdwgc.dll");
    DWORD mono_thread_attach = (DWORD)(intptr_t)hModule + FindExportByName(hProcess, hModule, "mono_thread_attach");
    DWORD mono_runtime_invoke = (DWORD)(intptr_t)hModule + FindExportByName(hProcess, hModule, "mono_runtime_invoke");
    DWORD method = MonoClass32GetMonoMethodByName(hProcess, instanceclass, methodname);

    LPVOID args = VirtualAllocEx(hProcess, NULL, 0x1000, MEM_COMMIT, PAGE_READWRITE);
    LPCVOID argsBuffer = (LPCVOID)malloc(0x1000);
    LPVOID code = VirtualAllocEx(hProcess, NULL, 0x1000, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
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
    for ( int i = 0 ; i < argnum ; ++i ) {
        DWORD arg = (DWORD)va_arg(varargs, DWORD);
        *(DWORD*)(argsBuffer + i * 4) = arg;
    }
    va_end(varargs);

    // mono_runtime_invoke
    //   B8 mono_runtime_invoke | MOV EAX, mono_runtime_invoke
    *(BYTE* )(codeBuffer + 15) = 0xB8;
    *(DWORD*)(codeBuffer + 16) = mono_runtime_invoke;
    //   68 00000000 | PUSH exception handler
    *(BYTE* )(codeBuffer + 20) = 0x68;
    *(DWORD*)(codeBuffer + 21) = 0;
    //   68 args | PUSH args
    *(BYTE* )(codeBuffer + 25) = 0x68;
    *(DWORD*)(codeBuffer + 26) = (DWORD)(intptr_t)args;
    //   68 situation | PUSH situation
    *(BYTE* )(codeBuffer + 30) = 0x68;
    *(DWORD*)(codeBuffer + 31) = instance;
    //   68 method | PUSH method
    *(BYTE* )(codeBuffer + 35) = 0x68;
    *(DWORD*)(codeBuffer + 36) = method;
    //   FF D0 | CALL EAX
    *(WORD* )(codeBuffer + 40) = 0xD0FF;
    //   83 C4 10 | ADD ESP, 16
    *(WORD* )(codeBuffer + 42) = 0xC483;
    *(BYTE* )(codeBuffer + 44) = 0x10;

    // C3 | RET
    *(BYTE* )(codeBuffer + 45) = 0xC3;

    WriteProcessMemory(hProcess, args, argsBuffer, 0x1000, NULL);
    HexDump(hProcess, args, 0x100);
    WriteProcessMemory(hProcess, code, codeBuffer, 0x1000, NULL);
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)code, NULL, 0, NULL);
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);

    free((void*)argsBuffer);
    free((void*)codeBuffer);
    VirtualFreeEx(hProcess, args, 0, MEM_RELEASE);
    VirtualFreeEx(hProcess, code, 0, MEM_RELEASE);
}
