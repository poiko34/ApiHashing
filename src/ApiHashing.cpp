#include "ApiHashing.h"

HMODULE GetModuleByHash(DWORD dwTargetHash) {
#if defined(_WIN64)
    PPEB pPeb = (PPEB)__readgsqword(0x60);
#else
    PPEB pPeb = (PPEB)__readfsdword(0x30);
#endif

    PPEB_LDR_DATA_CUSTOM pLdr = (PPEB_LDR_DATA_CUSTOM)pPeb->Ldr;
    
    LIST_ENTRY* pListHead = &pLdr->InLoadOrderModuleList;
    LIST_ENTRY* pListEntry = pListHead->Flink;

    while (pListEntry != pListHead) {
        LDR_DATA_TABLE_ENTRY_CUSTOM* pEntry = CONTAINING_RECORD(pListEntry, LDR_DATA_TABLE_ENTRY_CUSTOM, InLoadOrderLinks);

        if (pEntry->BaseDllName.Buffer != NULL) {
            if (runtime_fnv1a_w(pEntry->BaseDllName.Buffer) == dwTargetHash) {
                return (HMODULE)pEntry->DllBase;
            }
        }
        pListEntry = pListEntry->Flink;
    }
    return NULL;
}

FARPROC GetProcAddressByHash(HMODULE hModule, DWORD64 dwTargetHash) {
    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hModule;
    if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) return NULL;

    PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((BYTE*)hModule + pDosHeader->e_lfanew);
    if (pNtHeaders->Signature != IMAGE_NT_SIGNATURE) return NULL;

    IMAGE_OPTIONAL_HEADER optionalHeader = pNtHeaders->OptionalHeader;
    DWORD exportDirRVA = optionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    if (exportDirRVA == 0) return NULL;

    PIMAGE_EXPORT_DIRECTORY pExportDir = (PIMAGE_EXPORT_DIRECTORY)((BYTE*)hModule + exportDirRVA);

    DWORD* pNames = (DWORD*)((BYTE*)hModule + pExportDir->AddressOfNames);
    DWORD* pFunctions = (DWORD*)((BYTE*)hModule + pExportDir->AddressOfFunctions);
    WORD* pOrdinals = (WORD*)((BYTE*)hModule + pExportDir->AddressOfNameOrdinals);

    for (DWORD i = 0; i < pExportDir->NumberOfNames; i++) {
        char* szFunctionName = (char*)((BYTE*)hModule + pNames[i]);
        if (runtime_fnv1a(szFunctionName) == dwTargetHash) {
            WORD wOrdinal = pOrdinals[i];
            DWORD dwFuncRVA = pFunctions[wOrdinal];
            return (FARPROC)((BYTE*)hModule + dwFuncRVA);
        }
    }
    return NULL;
}

BOOL initApiTable(PAPI_TABLE pApi) {
    HMODULE hKernel32 = GetModuleByHash(HASHW(L"kernel32.dll"));
    if(!hKernel32) hKernel32 = GetModuleByHash(HASHW(L"KERNEL32.DLL"));
    if(!hKernel32) return FALSE;

    pApi->pLoadLibraryA = (fnLoadLibraryA)GetProcAddressByHash(hKernel32, HASH("LoadLibraryA"));
    if(!pApi->pLoadLibraryA) return FALSE;
    
    HMODULE hUser32 = pApi->pLoadLibraryA("user32.dll");
    if(!hUser32) hUser32 = pApi->pLoadLibraryA("USER32.DLL");
    if(!hUser32) return FALSE;

    pApi->pMessageBoxA = (fnMessageBoxA)GetProcAddressByHash(hUser32, HASH("MessageBoxA"));
    if(!pApi->pMessageBoxA) return FALSE;

    return TRUE;
}



