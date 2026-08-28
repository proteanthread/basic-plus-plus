# BASIC++ v6.5.2 Binary Formats

## 1. OVERVIEW

BASIC++ works with several binary file formats for program storage, data exchange, and legacy compatibility. This document describes each format and the tools that read and write them.

## 2. ASCII PROGRAM FILES (.BAS, .BPP)

The default program file format is plain ASCII text. Each line begins with a line number followed by a space and the statement text. Lines are terminated by CR+LF (Windows) or LF (Unix). The file encoding is UTF-8 on modern builds and ASCII on FreeDOS builds.

SAVE "PROGRAM.BAS" writes the current program in ASCII format. SAVE "PROGRAM.BAS", A explicitly requests ASCII format. LOAD "PROGRAM.BAS" reads it back.

The .bpp extension is used for BASIC++ extended programs that use features beyond GW-BASIC compatibility. The file format is identical to .bas.

## 3. PROTECTED PROGRAM FILES

SAVE "PROGRAM.BAS", P saves the program in a protected format. Protected programs are obfuscated: the source text is XOR-encrypted with a simple key. LIST and EDIT are disabled for protected programs. This provides casual copy protection, not cryptographic security.

LOAD reads protected files transparently. The interpreter decrypts the source internally.

## 4. TOKENIZED PROGRAM FILES (GW-BASIC BINARY FORMAT)

GW-BASIC stored programs in a binary tokenized format where keywords were replaced by single-byte or two-byte token codes. This format is compact but not human-readable.

BASIC++ can read GW-BASIC tokenized files through the detok (detokenizer) tool:

```bash
detok GWPROG.BAS > PROGRAM.TXT
```

The detok executable (engine build target) reads the binary token stream, maps each token back to its keyword text, and outputs a plain text program file. The output can then be loaded by BASIC++.

## 5. BYTECODE FORMAT

The bppc compiler generates bytecode files that contain a compact binary representation of the program's operations. The bytecode format consists of:

1. A file header with magic number, version, and metadata.
2. A constant pool containing string literals and numeric constants.
3. A bytecode section containing the opcode stream (BppOpcode values from engine/include/types/types.h).

Bytecode files are loaded by the VM stub and executed through the bytecode execution loop, bypassing the tokenizer and parser for faster startup.

## 6. TRANSPILED C17 OUTPUT

The bppc compiler can also produce C17 source code that, when compiled with a C17 compiler, produces a standalone native executable. The generated C includes:

- A VM stub with the execution loop.
- Embedded string constants.
- Compiled statement representations.
- Required runtime library functions.

The generated C file compiles with any C17-compliant compiler (GCC, Clang, MSVC).

## 7. STATE SAVE/RESTORE FORMAT

STATESAVE "filename" writes the complete VM state to a binary file. STATELOAD "filename" restores it. The state file contains:

- All variable values and types.
- All array contents.
- The string heap.
- The program text.
- Stack states (FOR, GOSUB, DO, WHILE, SELECT, SUB, TRY).
- The current execution position.
- Open file channel states.
- Event trap configurations.

State save files are version-specific and may not be compatible across BASIC++ versions.

## 8. RANDOM ACCESS DATA FILES

Programs that use OPEN FOR RANDOM create binary data files with fixed-length records. The record structure is defined by FIELD statements or by TYPE variable sizes. These files can be read by any program that knows the record layout.

## 9. BINARY DATA FILES

Programs that use OPEN FOR BINARY and BGET/BPUT create raw binary data files with no record structure. The program is responsible for interpreting the byte layout.

## 10. FILE FORMAT DETECTION

When LOAD encounters a file, it examines the first byte to determine the format:

- 0xFF: GW-BASIC tokenized format → passed to the detokenizer.
- 0xFE: GW-BASIC protected format → passed to the decryptor.
- 0x42 ('B'): BASIC++ bytecode format → loaded by the bytecode loader.
- Other: Treated as ASCII text → loaded line by line.
