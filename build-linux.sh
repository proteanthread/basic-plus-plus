#!/bin/bash
# ========================================================
# BASIC++ 4.1.2 — Linux Build (gcc)
# ========================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR/source"

echo "========================================================"
echo "BASIC++ 4.1.2 — Linux Build (gcc)"
echo "========================================================"

SOURCES="core/main.c core/memory.c core/errors.c core/value.c \
core/stringpool.c core/platform.c core/security.c core/ldisdbl.c \
core/config_file.c core/stdlib_core.c core/stdlib_dialect.c \
core/error_registry.c core/pcode_compiler.c core/pcode_emit.c \
core/vm_exec.c core/boot.c core/rpn.c core/console.c core/task.c \
lexer/lexer.c lexer/keyword_props.c lexer/alias_lang.c \
parser/parser.c parser/parser_expr.c spec.c \
flow/parser_flow.c flow/parser_loops.c \
io/parser_io.c io/parser_fileio.c io/parser_blockio.c \
io/parser_streamio.c io/fileio.c io/builtins_fileio.c io/vdev_net.c \
io/format_using.c io/format_input.c io/device_alias.c \
io/builtins_sio.c io/builtins_bio.c io/builtins_txn.c \
io/builtins_net.c io/txn.c io/parser_txn.c io/vfs.c \
filemgmt/parser_filemgmt.c \
graphics/parser_graphics.c graphics/gfxbuf.c \
graphics/builtins_graphics.c \
display/parser_display.c \
sound/parser_sound.c \
variables/parser_vars.c variables/parser_assign.c \
arrays/parser_mat.c \
strings/builtins_string.c \
math/builtins_math.c \
functions/funcreg.c functions/parser_deffn.c functions/builtins.c \
struct/parser_struct.c \
errhand/parser_errhand.c \
shell/parser_shell.c \
debug.c \
help/parser_help.c help/help.c \
config/parser_config.c config/override.c config/scope.c \
config/scope_stack.c \
progmgmt/parser_progmgmt.c progmgmt/parser_cmds.c \
progmgmt/compiler.c \
virtual/parser_virtual.c virtual/vdev.c virtual/vm.c \
memory/memmap.c memory/builtins_memory.c \
system/builtins_system.c system/builtins_io.c \
dialect/dialect.c \
dialect/dialect_patb.c dialect/dialect_trs1.c dialect/dialect_trs2.c \
dialect/dialect_gwbs.c dialect/dialect_ecma55.c \
dialect/dialect_ecma116.c dialect/dialect_qbasic.c \
dialect/dialect_aint.c dialect/dialect_asft.c \
dialect/dialect_atari.c dialect/dialect_c64.c dialect/dialect_coco.c \
dialect/dialect_mbasic.c dialect/dialect_sinclair.c \
dialect/dialect_superbasic.c dialect/dialect_sbasic.c \
codegen/ast.c codegen/codegen.c codegen/target.c codegen/bytecode.c codegen/detok.c codegen/archive.c \
modules/module.c modules/mod_stdlib.c \
modules/mod_usb.c modules/mod_fujinet.c \
modules/mod_upnp.c modules/mod_jit.c \
modules/ext_lib.c modules/ext_func.c \
modules/ext_feature.c modules/ext_plugin.c \
modules/lib_space.c modules/bpl_format.c \
modules/parser_block.c \
modules/mbf_math.c modules/segmented_mem.c \
modules/sdl2_emu.c modules/compat_plugin.c \
modules/serial_compat.c modules/mod_legacy_compat.c \
runtime/runtime.c runtime/exec.c \
misc/parser_misc.c"

# Detect CPU cores for parallel compilation
NUM_CORES=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
echo "[INFO] Building with $NUM_CORES parallel processes..."

echo "[INFO] Building blite..."
rm -f ../blite
make clean
make -j$NUM_CORES blite
echo "[OK] blite built."

echo "[INFO] Building baspp-console..."
make TARGET=../baspp-console clean
make -j$NUM_CORES TARGET=../baspp-console LINUX_CFLAGS="-std=c17 -Wall -Wextra -O2 -I. -D_POSIX_C_SOURCE=200809L -DINPUT_CONSOLE $(sdl2-config --cflags)" LDFLAGS="$(sdl2-config --libs)" all
echo "[OK] baspp-console built."

echo "[INFO] Building baspp..."
make TARGET=../baspp clean
make -j$NUM_CORES TARGET=../baspp LINUX_CFLAGS="-std=c17 -Wall -Wextra -O2 -I. -D_POSIX_C_SOURCE=200809L $(sdl2-config --cflags)" LDFLAGS="$(sdl2-config --libs)" all
echo "[OK] baspp built."

echo "[INFO] Building legacy standalone prototypes..."
gcc -ansi -Wall -Wextra -O2 -o ../tinybasic ../standalone/source/tinybasic.c || echo "[WARN] tinybasic build failed."
gcc -ansi -Wall -Wextra -O2 -o ../level1 ../standalone/source/level1.c || echo "[WARN] level1 build failed."
gcc -ansi -Wall -Wextra -O2 -o ../apple2 ../standalone/source/apple2.c || echo "[WARN] apple2 build failed."
gcc -ansi -Wall -Wextra -O2 -o ../1964 ../standalone/source/1964.c -lm || echo "[WARN] 1964 build failed."
echo "[INFO] Building trans and bppc..."
make trans-linux || echo "[WARN] trans build failed."
make bppc-linux || echo "[WARN] bppc build failed."

echo "========================================================"
echo "All builds complete."
echo "========================================================"
echo "  baspp         — BASIC++ GUI interpreter"
echo "  baspp-console — BASIC++ Console interpreter"
echo "  blite         — BASIC++ Lite interpreter"
echo "========================================================"
