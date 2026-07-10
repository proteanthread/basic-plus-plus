# PACMAN BAS Manual

This file serves as the definitive reference manual for `pacman.BAS`.

## Overview
`pacman.BAS` is part of the BASIC++ 5.0.1 Game Engine framework.
As a BAS file, it is designed to run natively within the BASIC++ text-based interpreter.

## Usage
- For Engines (`.BAS`): Load using `LOAD "games/pacman.BAS"`. Ensure you are executing from `basicpp-console.exe`. Use Arrow keys for movement, SPACE for actions, `S` for saving, `L` for loading, and `ESC` to quit.
- For Libraries (`.LIB`): Import using `LOAD LIBRARY "lib/pacman.BAS"`. Refer to the Master Guides for the specific subroutines exported.

## Dependencies
This file is self-contained or relies entirely on standard BASIC++ syntax and its paired library module. No external SDL bindings are utilized.
