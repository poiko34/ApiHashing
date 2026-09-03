# ApiHashing

A small Windows C++ Proof of Concept demonstrating **API hashing** and runtime API resolution without relying on the normal `GetModuleHandle` / `GetProcAddress` workflow.

The project manually walks the Process Environment Block (**PEB**) to locate loaded modules and parses the **Export Address Table (EAT)** to resolve exported functions by their hash.

> **Note:** This project is intended for educational and security research purposes. It demonstrates low-level Windows internals and PE parsing concepts.

## Overview

Normally, a Windows application can resolve an exported function using APIs such as:

```cpp
GetModuleHandleA("kernel32.dll");
GetProcAddress(module, "LoadLibraryA");
```

This PoC demonstrates an alternative approach:

```text
PEB
 │
 └── PEB_LDR_DATA
      │
      └── InLoadOrderModuleList
           │
           ├── kernel32.dll
           ├── user32.dll
           └── ...
                │
                ▼
        Hash module name
                │
                ▼
          Find module base
                │
                ▼
          Parse PE headers
                │
                ▼
       Export Address Table
                │
                ▼
        Hash exported names
                │
                ▼
       Resolve function address
                │
                ▼
          Function pointer
```

The example ultimately resolves and calls `MessageBoxA` without directly resolving it through the standard Windows API.

## Features

* PEB-based module enumeration
* 32-bit FNV-1a hashing
* Compile-time hashing for string literals
* Runtime hashing for module and export names
* Manual PE header parsing
* Manual Export Address Table (EAT) traversal
* Runtime API table initialization
* x86-64 Windows support
* CMake-based build system
* LLVM/MinGW cross-compilation support

## Project Structure

```text
ApiHashing/
├── src/
│   ├── ApiHashing.cpp
│   ├── ApiHashing.h
│   ├── Utils.h
│   └── main.cpp
│
├── CMakeLists.txt
├── llvm-mingw-toolchain.cmake
├── .gitignore
└── README.md
```

### `src/ApiHashing.cpp`

Contains the main API hashing implementation:

* `GetModuleByHash()` — searches loaded modules through the PEB loader structures.
* `GetProcAddressByHash()` — parses a module's PE export directory and searches for an exported function by hash.
* `initApiTable()` — resolves the APIs required by the demonstration.

### `src/ApiHashing.h`

Contains the Windows-specific structures and function pointer definitions used by the implementation.

The project defines a small custom representation of loader structures so that it can access the required PEB/LDR fields directly.

### `src/Utils.h`

Contains the FNV-1a hashing implementation.

Both narrow and wide strings are supported:

```cpp
HASH("MessageBoxA")
HASHW(L"kernel32.dll")
```

The project provides both compile-time and runtime hashing functions.

### `src/main.cpp`

Contains the demonstration program.

It initializes the API table and calls the resolved `MessageBoxA` function:

```text
Initialize API table
        │
        ├── Resolve kernel32.dll
        ├── Resolve LoadLibraryA
        ├── Load user32.dll
        └── Resolve MessageBoxA
                │
                ▼
          Call MessageBoxA
```

## How API Hashing Works

Instead of storing an API name directly at the point where it is resolved, the resolver compares hashes.

For example:

```cpp
HASH("MessageBoxA")
```

produces the hash at compile time.

At runtime, the resolver walks the module's export names and calculates the same hash:

```text
"MessageBoxA"
      │
      ▼
   FNV-1a
      │
      ▼
   32-bit hash
```

When the hashes match, the corresponding entry in the Export Address Table is used to obtain the function address.

This allows the PoC to resolve an exported function without passing its name to the standard `GetProcAddress` API.

## Module Resolution

`GetModuleByHash()` obtains the PEB using the architecture-specific segment register:

* x64: `GS:[0x60]`
* x86: `FS:[0x30]`

It then walks the loader's `InLoadOrderModuleList` and hashes each module's base name.

Conceptually:

```text
PEB
 │
 └── Ldr
      │
      └── InLoadOrderModuleList
           │
           ├── module #1
           ├── module #2
           ├── kernel32.dll  ──► hash match
           └── ...
```

Once the requested hash is found, the module's base address is returned.

## Export Resolution

After obtaining a module base address, `GetProcAddressByHash()` performs basic PE parsing:

```text
DOS Header
    │
    └── e_lfanew
          │
          ▼
      NT Headers
          │
          └── Optional Header
                │
                └── Export Directory
                      │
                      ├── AddressOfNames
                      ├── AddressOfFunctions
                      └── AddressOfNameOrdinals
```

The resolver iterates through the exported function names, hashes each name and compares it with the requested hash.

If a match is found, the corresponding function RVA is converted into an address relative to the module base.

## Hash Function

The PoC uses **FNV-1a** with the standard 32-bit FNV parameters:

```text
FNV offset basis = 0x811c9dc5
FNV prime        = 0x01000193
```

The hash is used as an identifier rather than as a cryptographic primitive.

FNV-1a should **not** be considered a cryptographic hash function. Hash collisions are theoretically possible, so a production implementation would need to consider collision handling and the required security properties.

## Building

### Requirements

* Windows or a Windows cross-compilation environment
* C++17-compatible compiler
* CMake 3.15 or newer

The project is configured as a C++17 CMake project.

### Build with CMake

```bash
cmake -S . -B build
cmake --build build --config Release
```

The resulting executable is named:

```text
app
```

### LLVM/MinGW

The repository also contains an LLVM/MinGW CMake toolchain file targeting:

```text
x86_64-w64-mingw32
```

The provided configuration uses Clang/LLVM-MinGW for C and C++ compilation.

Example:

```bash
cmake -S . -B build \
    -DCMAKE_TOOLCHAIN_FILE=llvm-mingw-toolchain.cmake

cmake --build build --config Release
```

The exact path to the LLVM-MinGW installation may need to be adjusted in `llvm-mingw-toolchain.cmake`.

## Example

When the program starts successfully, it prints:

```text
[+] Init Api Hashing successful
```

The resolved `MessageBoxA` function is then called, producing the demonstration message box.

If API resolution fails:

```text
[-] Init Api Hashing failed
```

## Limitations

This is intentionally a **small Proof of Concept**, not a production-ready API resolver.

Some important limitations include:

* No explicit handling of forwarded exports.
* No comprehensive validation of PE structures and RVA boundaries.
* Hash collisions are not handled.
* The implementation relies on Windows internal structures.
* Loader structure layouts and undocumented internals should not be treated as a stable public API.
* The current example resolves only the APIs required by the demonstration.
* The implementation is primarily intended for x86-64 Windows environments.
* FNV-1a provides no cryptographic security.

These limitations are intentional so that the implementation remains focused on demonstrating the underlying concept.

## Why API Hashing?

API hashing is a technique commonly discussed in:

* malware analysis
* reverse engineering
* Windows internals research
* PE format research
* defensive security research
* malware detection and threat hunting

It can make static identification of API names more difficult, but **API hashing is not encryption and does not provide strong secrecy**.

A resolver can still be identified through behavioral analysis, dynamic analysis, memory inspection, control-flow analysis, or by reproducing the hashing algorithm.

## Learning Goals

This project was created as a practical exercise for understanding:

1. Windows PEB internals
2. Windows loader structures
3. PE file structure
4. Export Address Tables
5. RVA-to-address calculations
6. Function pointers
7. Compile-time C++ evaluation
8. FNV-1a hashing
9. Runtime API resolution

## References

* [Microsoft PE/COFF specification](https://learn.microsoft.com/en-us/windows/win32/debug/pe-format)
* [Microsoft Win32 documentation](https://learn.microsoft.com/en-us/windows/win32/)
* [FNV hash function](http://www.isthe.com/chongo/tech/comp/fnv/)

## Disclaimer

This repository is provided for **educational and security research purposes**.

The code demonstrates Windows internals and API resolution techniques in a controlled Proof-of-Concept environment. The author does not encourage the use of these techniques for unauthorized access, evasion, or deployment of malicious software.
