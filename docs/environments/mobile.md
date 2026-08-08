# BASIC++ Mobile Environment Edition (`libbasicpp_mobile.so` / Swift Framework)

## Overview
The **Mobile Environment Edition** bridges BASIC++ into native iOS (Swift / Objective-C) and Android (Java / Kotlin JNI) applications.

## Binary Artifacts
- **Android**: `libbasicpp_mobile.so` (ARM64-v8a, armeabi-v7a, x86_64 JNI shared libraries)
- **iOS**: `BASICPP.framework` / `libbasicpp_mobile.a` (XCFramework for iOS Devices & Simulators)

## Native Bridge Interfaces
- **Android JNI**: Native C bridge defined in `engine/src/bootstrap/mobile/main_mobile.c`
- **iOS Swift**: C17 module map wrapping `bpp_api.h`
