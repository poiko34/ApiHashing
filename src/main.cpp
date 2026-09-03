#include <windows.h>
#include <stdio.h>
#include "Utils.h"
#include "ApiHashing.h"

int main() {
    API_TABLE Api;
    if(!initApiTable(&Api)) {
        printf("[-] Init Api Hashing failed\n");
        return 1;
    } else {
        printf("[+] Init Api Hashing successful\n");
    }

    Api.pMessageBoxA(NULL, 
        "Hello, World!", 
        "test", 
        MB_OK | MB_ICONINFORMATION);

    return 0;
}
