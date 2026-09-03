#include <windows.h>
#include <stdio.h>
#include "Utils.h"

int main() {
    char string[] = "Hello World!";
    DWORD runtimeHash = runtime_fnv1a(string);
    DWORD constexprHash = HASH("This is test");

    printf("String: %s, Hash: %lu\n", string, runtimeHash);
    printf("Constexpr hash: %lu\n", constexprHash);

    return 0;
}
