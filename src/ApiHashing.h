#pragma once
#include <windows.h>
#include <winternl.h>
#include "Utils.h"

typedef struct _PEB_LDR_DATA_CUSTOM
{
    ULONG Length;
    BOOLEAN Initialized;
    HANDLE SsHandle;
    LIST_ENTRY InLoadOrderModuleList;
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
    PVOID EntryInProgress;
    BOOLEAN ShutdownInProgress;
    HANDLE ShutdownThreadId;
} PEB_LDR_DATA_CUSTOM, *PPEB_LDR_DATA_CUSTOM;

typedef struct _LDR_DATA_TABLE_ENTRY_CUSTOM {
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderLinks;
    PVOID DllBase;
    PVOID EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
} LDR_DATA_TABLE_ENTRY_CUSTOM, * PLDR_DATA_TABLE_ENTRY_CUSTOM;

typedef int(WINAPI* fnMessageBoxA)(HWND, LPCTSTR, LPCTSTR, UINT);
typedef HMODULE(WINAPI* fnLoadLibraryA)(LPCSTR);

typedef struct _API_TABLE {
    fnLoadLibraryA  pLoadLibraryA;
    fnMessageBoxA    pMessageBoxA;
} API_TABLE, *PAPI_TABLE;

BOOL initApiTable(PAPI_TABLE pApi);

