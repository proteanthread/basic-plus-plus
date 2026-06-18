# =====================================================================
# BASIC++ Interpreter — Makefile
# =====================================================================
#
# Multi-platform build system.
#
# TARGETS:
#   make          — Build "baspp" with gcc (Linux/MinGW), C17
#   make debug    — Debug build with gcc
#   make msvc     — Build "basicpp.exe" with MSVC (Windows 11)
#   make watcom   — Build "bpp.exe" with OpenWatcom 16-bit (FreeDOS)
#   make watcom386— Build "bpp.exe" with OpenWatcom 32-bit (DOS/4GW)
#   make clean    — Remove build artifacts
#
# EXECUTABLE NAMES:
#   baspp         — Linux (gcc/MinGW)
#   basicpp.exe   — Windows 11 (MSVC)
#   bpp.exe       — FreeDOS (OpenWatcom)
#
# DIRECTORY LAYOUT:
#   All .c implementation files are organized into domain subdirectories.
#   All .h header files remain in source/ root (the public interface).
#   The -I. flag ensures subdirectory files can find root headers.
#   All executables are placed in the project root (..).
#
# =====================================================================

# Compiler and flags (gcc default)
CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -O2 -I. -D_POSIX_C_SOURCE=200809L
LDFLAGS =

# Output directory — project root (parent of source/)
OUTDIR = ..

# Default target name for gcc/Linux
TARGET = $(OUTDIR)/baspp

# =====================================================================
# Source files — organized by domain subdirectory
# =====================================================================

# Core infrastructure
CORE_SOURCES = \
    core/main.c core/memory.c core/errors.c core/value.c \
    core/stringpool.c core/platform.c core/security.c core/ldisdbl.c \
    core/config_file.c core/stdlib_core.c core/stdlib_dialect.c \
    core/error_registry.c core/pcode_compiler.c core/pcode_emit.c \
    core/vm_exec.c \
    core/boot.c core/rpn.c

# Lexer / tokenizer
LEXER_SOURCES = \
    lexer/lexer.c lexer/keyword_props.c lexer/alias_lang.c

# Parser dispatcher & expression evaluator
PARSER_SOURCES = \
    parser/parser.c parser/parser_expr.c spec.c

# Control flow (IF/ELSE/FOR/WHILE/DO/GOSUB)
FLOW_SOURCES = \
    flow/parser_flow.c flow/parser_loops.c

# Input/Output (by device)
IO_SOURCES = \
    io/parser_io.c io/parser_fileio.c io/parser_blockio.c \
    io/parser_streamio.c io/fileio.c io/builtins_fileio.c io/vdev_net.c \
    io/format_using.c io/format_input.c io/device_alias.c \
    io/builtins_sio.c io/builtins_bio.c io/builtins_txn.c \
    io/builtins_net.c \
    io/txn.c io/parser_txn.c io/vfs.c

# File management & directories
FILEMGMT_SOURCES = \
    filemgmt/parser_filemgmt.c

# Graphics & drawing
GRAPHICS_SOURCES = \
    graphics/parser_graphics.c graphics/gfxbuf.c \
    graphics/builtins_graphics.c

# Display & console attributes
DISPLAY_SOURCES = \
    display/parser_display.c

# Sound & music
SOUND_SOURCES = \
    sound/parser_sound.c

# Variables, constants & types
VARIABLES_SOURCES = \
    variables/parser_vars.c variables/parser_assign.c

# Arrays & matrices
ARRAYS_SOURCES = \
    arrays/parser_mat.c

# String operations
STRINGS_SOURCES = \
    strings/builtins_string.c

# Math (integer & floating point)
MATH_SOURCES = \
    math/builtins_math.c

# Registered functions & user-defined functions
FUNCTIONS_SOURCES = \
    functions/funcreg.c functions/parser_deffn.c functions/builtins.c

# Structured programming (SELECT/CASE, SUB/FUNCTION)
STRUCT_SOURCES = \
    struct/parser_struct.c

# Error handling (ERROR, RESUME, WHEN, etc.)
ERRHAND_SOURCES = \
    errhand/parser_errhand.c

# Shell & external execution
SHELL_SOURCES = \
    shell/parser_shell.c

# Debugging & testing
DEBUG_SOURCES = \
    debug/parser_debug.c debug/selftest.c debug/check.c

# Help system
HELP_SOURCES = \
    help/parser_help.c help/help.c

# Configuration & meta commands
CONFIG_SOURCES = \
    config/parser_config.c config/override.c config/scope.c \
    config/scope_stack.c

# Program management & editing
PROGMGMT_SOURCES = \
    progmgmt/parser_progmgmt.c progmgmt/parser_cmds.c \
    progmgmt/compiler.c

# Virtual infrastructure
VIRTUAL_SOURCES = \
    virtual/parser_virtual.c virtual/vdev.c virtual/vm.c

# Memory introspection
MEMORY_SOURCES = \
    memory/memmap.c memory/builtins_memory.c

# System & environment
SYSTEM_SOURCES = \
    system/builtins_system.c system/builtins_io.c

# Dialect profiles (one file per dialect)
DIALECT_SOURCES = \
    dialect/dialect.c \
    dialect/dialect_patb.c dialect/dialect_trs1.c dialect/dialect_trs2.c \
    dialect/dialect_gwbs.c dialect/dialect_ecma55.c \
    dialect/dialect_ecma116.c dialect/dialect_qbasic.c \
    dialect/dialect_aint.c dialect/dialect_asft.c \
    dialect/dialect_atari.c dialect/dialect_c64.c dialect/dialect_coco.c \
    dialect/dialect_mbasic.c dialect/dialect_sinclair.c \
    dialect/dialect_superbasic.c dialect/dialect_sbasic.c

# Code generation & bytecode
CODEGEN_SOURCES = \
    codegen/ast.c codegen/codegen.c codegen/bytecode.c codegen/detok.c

# External modules
MODULES_SOURCES = \
    modules/module.c modules/mod_stdlib.c \
    modules/mod_usb.c modules/mod_fujinet.c \
    modules/mod_upnp.c modules/mod_jit.c \
    modules/ext_lib.c modules/ext_func.c \
    modules/ext_feature.c modules/ext_plugin.c \
    modules/lib_space.c modules/bpl_format.c \
    modules/parser_block.c

# Runtime state & execution
RUNTIME_SOURCES = \
    runtime/runtime.c runtime/exec.c

# Miscellaneous utility commands
MISC_SOURCES = \
    misc/parser_misc.c

# =====================================================================
# Combined source list
# =====================================================================
SOURCES = $(CORE_SOURCES) $(LEXER_SOURCES) $(PARSER_SOURCES) \
          $(FLOW_SOURCES) $(IO_SOURCES) $(FILEMGMT_SOURCES) \
          $(GRAPHICS_SOURCES) $(DISPLAY_SOURCES) $(SOUND_SOURCES) \
          $(VARIABLES_SOURCES) $(ARRAYS_SOURCES) $(STRINGS_SOURCES) \
          $(MATH_SOURCES) $(FUNCTIONS_SOURCES) $(STRUCT_SOURCES) \
          $(ERRHAND_SOURCES) $(SHELL_SOURCES) $(DEBUG_SOURCES) \
          $(HELP_SOURCES) $(CONFIG_SOURCES) $(PROGMGMT_SOURCES) \
          $(VIRTUAL_SOURCES) $(MEMORY_SOURCES) $(SYSTEM_SOURCES) \
          $(DIALECT_SOURCES) $(CODEGEN_SOURCES) $(MODULES_SOURCES) \
          $(RUNTIME_SOURCES) $(MISC_SOURCES)

# Object files (derived from sources)
OBJECTS = $(SOURCES:.c=.o)

# Header files (all in source/ root — the public interface surface)
HEADERS = config.h memory.h lexer.h parser.h parser_internal.h \
          parser_internal_additions.h \
          exec.h runtime.h \
          dialect.h fileio.h detok.h errors.h value.h stringpool.h \
          ast.h codegen.h compiler.h \
          vdev.h funcreg.h builtins.h vm.h module.h security.h \
          platform.h help.h selftest.h memmap.h gfxbuf.h bytecode.h \
          keyword_props.h alias_lang.h override.h scope.h scope_stack.h \
          config_file.h boot.h rpn.h check.h device_alias.h txn.h \
          mod_stdlib.h mod_usb.h mod_fujinet.h mod_upnp.h mod_jit.h \
          io/vfs.h compat.h

# =====================================================================
# Default target: gcc release build
# =====================================================================
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECTS) -lm

# Generic rule: compile .c to .o
# All object files depend on all headers (conservative but correct)
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# =====================================================================
# Debug build: gcc with debug symbols, no optimization
# =====================================================================
# debug target invokes make with debug flags
debug:
	$(MAKE) CFLAGS="-std=c17 -Wall -Wextra -g -O0 -DDEBUG -I." clean $(TARGET)

# =====================================================================
# MSVC build — produces "basicpp.exe" in project root
# =====================================================================
# Usage: nmake /f Makefile msvc
#   (from Visual Studio Developer Command Prompt)
#
msvc:
	cl /TC /std:c17 /W3 /O2 /I. /D_CRT_SECURE_NO_WARNINGS /c $(SOURCES)
	link /OUT:basicpp.exe /SUBSYSTEM:CONSOLE *.obj advapi32.lib ws2_32.lib
	@if exist basicpp.exe move /Y basicpp.exe $(OUTDIR)\basicpp.exe >nul
	@del /q *.obj 2>nul

# =====================================================================
# OpenWatcom build — produces "bpp.exe" in project root (FreeDOS)
# =====================================================================
# Usage (16-bit real mode, large model):
#   make watcom
# Usage (32-bit protected mode, DOS/4GW extender):
#   make watcom386
#
# FREEDOS SLIM BUILD:
#   Both targets define BPP_FREEDOS, which activates:
#   - Reduced memory pools (fits within 512K conventional memory)
#   - Only GW-BASIC + ECMA-116 dialects (14 dialects excluded)
#   - No optional modules (USB, FujiNet, UPnP excluded)
#
#   To add a dialect back to the FreeDOS build:
#   1. Add its compile line back to the watcom/watcom386 target
#   2. Add its .obj to WATCOM_OBJS
#   3. Remove its #ifndef BPP_FREEDOS guard in dialect.h + dialect.c
#   4. Verify the binary still fits in 512K
#
# OpenWatcom compiler flags:
#   -ml         = large memory model (16-bit: far code + far data)
#   -mf         = flat memory model (32-bit: DPMI)
#   -0          = 8086 instruction set (broadest compatibility)
#   -za         = strict ANSI C (equivalent to -std=c90)
#   -wx         = maximum warnings
#   -ox         = maximum optimization
#   -i=.        = include path (source/ root for headers)
#   -dBPP_FREEDOS = enable FreeDOS slim build (512K memory, 2 dialects)
#
WCC = wcc
WCC386 = wcc386
WCFLAGS = -ml -0 -za -wx -ox -i=. -dBPP_FREEDOS
WCFLAGS386 = -mf -za -wx -ox -i=. -dBPP_FREEDOS

# --- Object file list (FreeDOS slim build) ---
# Only GW-BASIC + ECMA-116 dialects.  No USB/FujiNet/UPnP modules.
# To restore a dialect: add its .obj here AND its compile line below.
WATCOM_OBJS = \
	main.obj memory.obj errors.obj value.obj stringpool.obj \
	platform.obj security.obj ldisdbl.obj config_file.obj \
	stdlib_core.obj stdlib_dialect.obj \
	error_registry.obj pcode_compiler.obj pcode_emit.obj vm_exec.obj \
	boot.obj rpn.obj \
	lexer.obj keyword_props.obj alias_lang.obj \
	parser.obj parser_expr.obj \
	parser_flow.obj parser_loops.obj \
	parser_io.obj parser_fileio.obj parser_blockio.obj \
	parser_streamio.obj fileio.obj builtins_fileio.obj \
	parser_filemgmt.obj \
	parser_graphics.obj gfxbuf.obj builtins_graphics.obj \
	parser_display.obj \
	parser_sound.obj \
	parser_vars.obj parser_assign.obj \
	parser_mat.obj \
	builtins_string.obj \
	builtins_math.obj \
	funcreg.obj parser_deffn.obj builtins.obj \
	parser_struct.obj \
	parser_errhand.obj \
	parser_shell.obj \
	parser_debug.obj selftest.obj check.obj \
	parser_help.obj help.obj \
	parser_config.obj override.obj scope.obj scope_stack.obj \
	parser_progmgmt.obj parser_cmds.obj compiler.obj \
	parser_virtual.obj vdev.obj vm.obj \
	memmap.obj builtins_memory.obj \
	builtins_system.obj builtins_io.obj \
	dialect.obj \
	dialect_patb.obj \
	ast.obj codegen.obj bytecode.obj detok.obj \
	module.obj mod_stdlib.obj mod_jit.obj \
	format_using.obj format_input.obj device_alias.obj \
	builtins_sio.obj builtins_bio.obj builtins_txn.obj \
	builtins_net.obj txn.obj parser_txn.obj \
	vdev_net.obj spec.obj vfs.obj \
	runtime.obj exec.obj \
	parser_misc.obj

watcom:
	@echo === OpenWatcom 16-bit build (FreeDOS slim: PATB only) ===
	$(WCC) $(WCFLAGS) -fo=main.obj core/main.c
	$(WCC) $(WCFLAGS) -fo=memory.obj core/memory.c
	$(WCC) $(WCFLAGS) -fo=errors.obj core/errors.c
	$(WCC) $(WCFLAGS) -fo=value.obj core/value.c
	$(WCC) $(WCFLAGS) -fo=stringpool.obj core/stringpool.c
	$(WCC) $(WCFLAGS) -fo=platform.obj core/platform.c
	$(WCC) $(WCFLAGS) -fo=security.obj core/security.c
	$(WCC) $(WCFLAGS) -fo=ldisdbl.obj core/ldisdbl.c
	$(WCC) $(WCFLAGS) -fo=config_file.obj core/config_file.c
	$(WCC) $(WCFLAGS) -fo=stdlib_core.obj core/stdlib_core.c
	$(WCC) $(WCFLAGS) -fo=stdlib_dialect.obj core/stdlib_dialect.c
	$(WCC) $(WCFLAGS) -fo=error_registry.obj core/error_registry.c
	$(WCC) $(WCFLAGS) -fo=pcode_compiler.obj core/pcode_compiler.c
	$(WCC) $(WCFLAGS) -fo=pcode_emit.obj core/pcode_emit.c
	$(WCC) $(WCFLAGS) -fo=vm_exec.obj core/vm_exec.c
	$(WCC) $(WCFLAGS) -fo=boot.obj core/boot.c
	$(WCC) $(WCFLAGS) -fo=rpn.obj core/rpn.c
	$(WCC) $(WCFLAGS) -fo=lexer.obj lexer/lexer.c
	$(WCC) $(WCFLAGS) -fo=keyword_props.obj lexer/keyword_props.c
	$(WCC) $(WCFLAGS) -fo=alias_lang.obj lexer/alias_lang.c
	$(WCC) $(WCFLAGS) -fo=parser.obj parser/parser.c
	$(WCC) $(WCFLAGS) -fo=parser_expr.obj parser/parser_expr.c
	$(WCC) $(WCFLAGS) -fo=spec.obj spec.c
	$(WCC) $(WCFLAGS) -fo=parser_flow.obj flow/parser_flow.c
	$(WCC) $(WCFLAGS) -fo=parser_loops.obj flow/parser_loops.c
	$(WCC) $(WCFLAGS) -fo=parser_io.obj io/parser_io.c
	$(WCC) $(WCFLAGS) -fo=parser_fileio.obj io/parser_fileio.c
	$(WCC) $(WCFLAGS) -fo=parser_blockio.obj io/parser_blockio.c
	$(WCC) $(WCFLAGS) -fo=parser_streamio.obj io/parser_streamio.c
	$(WCC) $(WCFLAGS) -fo=fileio.obj io/fileio.c
	$(WCC) $(WCFLAGS) -fo=builtins_fileio.obj io/builtins_fileio.c
	$(WCC) $(WCFLAGS) -fo=format_using.obj io/format_using.c
	$(WCC) $(WCFLAGS) -fo=format_input.obj io/format_input.c
	$(WCC) $(WCFLAGS) -fo=device_alias.obj io/device_alias.c
	$(WCC) $(WCFLAGS) -fo=builtins_sio.obj io/builtins_sio.c
	$(WCC) $(WCFLAGS) -fo=builtins_bio.obj io/builtins_bio.c
	$(WCC) $(WCFLAGS) -fo=builtins_txn.obj io/builtins_txn.c
	$(WCC) $(WCFLAGS) -fo=builtins_net.obj io/builtins_net.c
	$(WCC) $(WCFLAGS) -fo=txn.obj io/txn.c
	$(WCC) $(WCFLAGS) -fo=parser_txn.obj io/parser_txn.c
	$(WCC) $(WCFLAGS) -fo=vdev_net.obj io/vdev_net.c
	$(WCC) $(WCFLAGS) -fo=parser_filemgmt.obj filemgmt/parser_filemgmt.c
	$(WCC) $(WCFLAGS) -fo=parser_graphics.obj graphics/parser_graphics.c
	$(WCC) $(WCFLAGS) -fo=gfxbuf.obj graphics/gfxbuf.c
	$(WCC) $(WCFLAGS) -fo=builtins_graphics.obj graphics/builtins_graphics.c
	$(WCC) $(WCFLAGS) -fo=parser_display.obj display/parser_display.c
	$(WCC) $(WCFLAGS) -fo=parser_sound.obj sound/parser_sound.c
	$(WCC) $(WCFLAGS) -fo=parser_vars.obj variables/parser_vars.c
	$(WCC) $(WCFLAGS) -fo=parser_assign.obj variables/parser_assign.c
	$(WCC) $(WCFLAGS) -fo=parser_mat.obj arrays/parser_mat.c
	$(WCC) $(WCFLAGS) -fo=builtins_string.obj strings/builtins_string.c
	$(WCC) $(WCFLAGS) -fo=builtins_math.obj math/builtins_math.c
	$(WCC) $(WCFLAGS) -fo=funcreg.obj functions/funcreg.c
	$(WCC) $(WCFLAGS) -fo=parser_deffn.obj functions/parser_deffn.c
	$(WCC) $(WCFLAGS) -fo=builtins.obj functions/builtins.c
	$(WCC) $(WCFLAGS) -fo=parser_struct.obj struct/parser_struct.c
	$(WCC) $(WCFLAGS) -fo=parser_errhand.obj errhand/parser_errhand.c
	$(WCC) $(WCFLAGS) -fo=parser_shell.obj shell/parser_shell.c
	$(WCC) $(WCFLAGS) -fo=parser_debug.obj debug/parser_debug.c
	$(WCC) $(WCFLAGS) -fo=selftest.obj debug/selftest.c
	$(WCC) $(WCFLAGS) -fo=check.obj debug/check.c
	$(WCC) $(WCFLAGS) -fo=parser_help.obj help/parser_help.c
	$(WCC) $(WCFLAGS) -fo=help.obj help/help.c
	$(WCC) $(WCFLAGS) -fo=parser_config.obj config/parser_config.c
	$(WCC) $(WCFLAGS) -fo=override.obj config/override.c
	$(WCC) $(WCFLAGS) -fo=scope.obj config/scope.c
	$(WCC) $(WCFLAGS) -fo=scope_stack.obj config/scope_stack.c
	$(WCC) $(WCFLAGS) -fo=parser_progmgmt.obj progmgmt/parser_progmgmt.c
	$(WCC) $(WCFLAGS) -fo=parser_cmds.obj progmgmt/parser_cmds.c
	$(WCC) $(WCFLAGS) -fo=compiler.obj progmgmt/compiler.c
	$(WCC) $(WCFLAGS) -fo=parser_virtual.obj virtual/parser_virtual.c
	$(WCC) $(WCFLAGS) -fo=vdev.obj virtual/vdev.c
	$(WCC) $(WCFLAGS) -fo=vm.obj virtual/vm.c
	$(WCC) $(WCFLAGS) -fo=memmap.obj memory/memmap.c
	$(WCC) $(WCFLAGS) -fo=builtins_memory.obj memory/builtins_memory.c
	$(WCC) $(WCFLAGS) -fo=builtins_system.obj system/builtins_system.c
	$(WCC) $(WCFLAGS) -fo=builtins_io.obj system/builtins_io.c
	$(WCC) $(WCFLAGS) -fo=dialect.obj dialect/dialect.c
	$(WCC) $(WCFLAGS) -fo=dialect_patb.obj dialect/dialect_patb.c
	$(WCC) $(WCFLAGS) -fo=ast.obj codegen/ast.c
	$(WCC) $(WCFLAGS) -fo=codegen.obj codegen/codegen.c
	$(WCC) $(WCFLAGS) -fo=bytecode.obj codegen/bytecode.c
	$(WCC) $(WCFLAGS) -fo=detok.obj codegen/detok.c
	$(WCC) $(WCFLAGS) -fo=module.obj modules/module.c
	$(WCC) $(WCFLAGS) -fo=mod_stdlib.obj modules/mod_stdlib.c
	$(WCC) $(WCFLAGS) -fo=mod_jit.obj modules/mod_jit.c
	$(WCC) $(WCFLAGS) -fo=runtime.obj runtime/runtime.c
	$(WCC) $(WCFLAGS) -fo=exec.obj runtime/exec.c
	$(WCC) $(WCFLAGS) -fo=parser_misc.obj misc/parser_misc.c
	@echo "system dos" > _link.lnk
	@echo "name $(OUTDIR)/bpp.exe" >> _link.lnk
	@for obj in $(WATCOM_OBJS); do echo "file $$obj" >> _link.lnk; done
	wlink @_link.lnk
	@rm -f _link.lnk
	@echo === bpp.exe built (16-bit FreeDOS, PATB only) ===

# =====================================================================
# OpenWatcom 32-bit build — produces "bpp.exe" with DOS/4GW extender
# =====================================================================
# Same slim dialect set as the 16-bit build (GWBS + E116 only).
# The DOS/4GW extender gives access to extended memory beyond 640K,
# but we still use the slim BPP_FREEDOS pools for consistency.
# To increase pool sizes for 32-bit DOS, edit config.h under the
# BPP_FREEDOS section.
#
watcom386:
	@echo === OpenWatcom 32-bit build (FreeDOS slim: GWBS + E116) ===
	$(WCC386) $(WCFLAGS386) -fo=main.obj core/main.c
	$(WCC386) $(WCFLAGS386) -fo=memory.obj core/memory.c
	$(WCC386) $(WCFLAGS386) -fo=errors.obj core/errors.c
	$(WCC386) $(WCFLAGS386) -fo=value.obj core/value.c
	$(WCC386) $(WCFLAGS386) -fo=stringpool.obj core/stringpool.c
	$(WCC386) $(WCFLAGS386) -fo=platform.obj core/platform.c
	$(WCC386) $(WCFLAGS386) -fo=security.obj core/security.c
	$(WCC386) $(WCFLAGS386) -fo=ldisdbl.obj core/ldisdbl.c
	$(WCC386) $(WCFLAGS386) -fo=config_file.obj core/config_file.c
	$(WCC386) $(WCFLAGS386) -fo=lexer.obj lexer/lexer.c
	$(WCC386) $(WCFLAGS386) -fo=keyword_props.obj lexer/keyword_props.c
	$(WCC386) $(WCFLAGS386) -fo=alias_lang.obj lexer/alias_lang.c
	$(WCC386) $(WCFLAGS386) -fo=parser.obj parser/parser.c
	$(WCC386) $(WCFLAGS386) -fo=parser_expr.obj parser/parser_expr.c
	$(WCC386) $(WCFLAGS386) -fo=parser_flow.obj flow/parser_flow.c
	$(WCC386) $(WCFLAGS386) -fo=parser_loops.obj flow/parser_loops.c
	$(WCC386) $(WCFLAGS386) -fo=parser_io.obj io/parser_io.c
	$(WCC386) $(WCFLAGS386) -fo=parser_fileio.obj io/parser_fileio.c
	$(WCC386) $(WCFLAGS386) -fo=parser_blockio.obj io/parser_blockio.c
	$(WCC386) $(WCFLAGS386) -fo=parser_streamio.obj io/parser_streamio.c
	$(WCC386) $(WCFLAGS386) -fo=fileio.obj io/fileio.c
	$(WCC386) $(WCFLAGS386) -fo=builtins_fileio.obj io/builtins_fileio.c
	$(WCC386) $(WCFLAGS386) -fo=parser_filemgmt.obj filemgmt/parser_filemgmt.c
	$(WCC386) $(WCFLAGS386) -fo=parser_graphics.obj graphics/parser_graphics.c
	$(WCC386) $(WCFLAGS386) -fo=gfxbuf.obj graphics/gfxbuf.c
	$(WCC386) $(WCFLAGS386) -fo=builtins_graphics.obj graphics/builtins_graphics.c
	$(WCC386) $(WCFLAGS386) -fo=parser_display.obj display/parser_display.c
	$(WCC386) $(WCFLAGS386) -fo=parser_sound.obj sound/parser_sound.c
	$(WCC386) $(WCFLAGS386) -fo=parser_vars.obj variables/parser_vars.c
	$(WCC386) $(WCFLAGS386) -fo=parser_assign.obj variables/parser_assign.c
	$(WCC386) $(WCFLAGS386) -fo=parser_mat.obj arrays/parser_mat.c
	$(WCC386) $(WCFLAGS386) -fo=builtins_string.obj strings/builtins_string.c
	$(WCC386) $(WCFLAGS386) -fo=builtins_math.obj math/builtins_math.c
	$(WCC386) $(WCFLAGS386) -fo=funcreg.obj functions/funcreg.c
	$(WCC386) $(WCFLAGS386) -fo=parser_deffn.obj functions/parser_deffn.c
	$(WCC386) $(WCFLAGS386) -fo=builtins.obj functions/builtins.c
	$(WCC386) $(WCFLAGS386) -fo=parser_struct.obj struct/parser_struct.c
	$(WCC386) $(WCFLAGS386) -fo=parser_errhand.obj errhand/parser_errhand.c
	$(WCC386) $(WCFLAGS386) -fo=parser_shell.obj shell/parser_shell.c
	$(WCC386) $(WCFLAGS386) -fo=parser_debug.obj debug/parser_debug.c
	$(WCC386) $(WCFLAGS386) -fo=selftest.obj debug/selftest.c
	$(WCC386) $(WCFLAGS386) -fo=parser_help.obj help/parser_help.c
	$(WCC386) $(WCFLAGS386) -fo=help.obj help/help.c
	$(WCC386) $(WCFLAGS386) -fo=parser_config.obj config/parser_config.c
	$(WCC386) $(WCFLAGS386) -fo=override.obj config/override.c
	$(WCC386) $(WCFLAGS386) -fo=scope.obj config/scope.c
	$(WCC386) $(WCFLAGS386) -fo=scope_stack.obj config/scope_stack.c
	$(WCC386) $(WCFLAGS386) -fo=parser_progmgmt.obj progmgmt/parser_progmgmt.c
	$(WCC386) $(WCFLAGS386) -fo=parser_cmds.obj progmgmt/parser_cmds.c
	$(WCC386) $(WCFLAGS386) -fo=compiler.obj progmgmt/compiler.c
	$(WCC386) $(WCFLAGS386) -fo=parser_virtual.obj virtual/parser_virtual.c
	$(WCC386) $(WCFLAGS386) -fo=vdev.obj virtual/vdev.c
	$(WCC386) $(WCFLAGS386) -fo=vm.obj virtual/vm.c
	$(WCC386) $(WCFLAGS386) -fo=memmap.obj memory/memmap.c
	$(WCC386) $(WCFLAGS386) -fo=builtins_memory.obj memory/builtins_memory.c
	$(WCC386) $(WCFLAGS386) -fo=builtins_system.obj system/builtins_system.c
	$(WCC386) $(WCFLAGS386) -fo=builtins_io.obj system/builtins_io.c
	$(WCC386) $(WCFLAGS386) -fo=dialect.obj dialect/dialect.c
	$(WCC386) $(WCFLAGS386) -fo=dialect_gwbs.obj dialect/dialect_gwbs.c
	$(WCC386) $(WCFLAGS386) -fo=dialect_ecma116.obj dialect/dialect_ecma116.c
	$(WCC386) $(WCFLAGS386) -fo=ast.obj codegen/ast.c
	$(WCC386) $(WCFLAGS386) -fo=codegen.obj codegen/codegen.c
	$(WCC386) $(WCFLAGS386) -fo=bytecode.obj codegen/bytecode.c
	$(WCC386) $(WCFLAGS386) -fo=detok.obj codegen/detok.c
	$(WCC386) $(WCFLAGS386) -fo=module.obj modules/module.c
	$(WCC386) $(WCFLAGS386) -fo=mod_stdlib.obj modules/mod_stdlib.c
	$(WCC386) $(WCFLAGS386) -fo=runtime.obj runtime/runtime.c
	$(WCC386) $(WCFLAGS386) -fo=exec.obj runtime/exec.c
	$(WCC386) $(WCFLAGS386) -fo=parser_misc.obj misc/parser_misc.c
	@echo "system dos4g" > _link.lnk
	@echo "name $(OUTDIR)/bpp.exe" >> _link.lnk
	@for obj in $(WATCOM_OBJS); do echo "file $$obj" >> _link.lnk; done
	wlink @_link.lnk
	@rm -f _link.lnk
	@echo === bpp.exe built (32-bit FreeDOS, GWBS+E116) ===

# =====================================================================
# Clean
# =====================================================================
clean:
	rm -f $(OBJECTS) $(OUTDIR)/baspp $(OUTDIR)/basicpp.exe $(OUTDIR)/bpp.exe *.obj
	find . -name '*.o' -delete 2>/dev/null || true
	find . -name '*.obj' -delete 2>/dev/null || true
	rm -f _link.lnk

.PHONY: all debug msvc watcom watcom386 clean
