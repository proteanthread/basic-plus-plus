// FILENAME: interop_com_register.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (interop_com.h, interop_com.c)
// Provides core logic and interface definitions for interop_com_register within BASIC++.
//
// ---- Includes ----

#include "interop/interop_com.h"
#include <stdio.h>

#ifdef _WIN32

static const char *g_ClsidKey = "Software\\Classes\\CLSID\\{1A7DE99C-8A5E-4B07-A961-A0BC3D782DE1}";
static const char *g_ProgIdKey = "Software\\Classes\\BASICPP.Engine";

static HRESULT SetRegKey(const char *key, const char *subKey, const char *value) {
    HKEY hKey;
    char fullKey[512];
    snprintf(fullKey, sizeof(fullKey), "%s%s%s", key, subKey ? "\\" : "", subKey ? subKey : "");

    if (RegCreateKeyExA(HKEY_LOCAL_MACHINE, fullKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS) {
        return E_FAIL;
    }

    if (value) {
        RegSetValueExA(hKey, NULL, 0, REG_SZ, (const BYTE *)value, (DWORD)strlen(value) + 1);
    }
    RegCloseKey(hKey);
    return S_OK;
}

STDAPI DllRegisterServer(void) {
    SetRegKey(g_ProgIdKey, NULL, "BASIC++ Engine Class");
    SetRegKey(g_ProgIdKey, "CLSID", "{1A7DE99C-8A5E-4B07-A961-A0BC3D782DE1}");

    SetRegKey(g_ClsidKey, NULL, "BASIC++ Engine Class");
    SetRegKey(g_ClsidKey, "ProgID", "BASICPP.Engine");
    SetRegKey(g_ClsidKey, "InprocServer32", "basicpp.dll");

    return S_OK;
}

STDAPI DllUnregisterServer(void) {
    RegDeleteTreeA(HKEY_LOCAL_MACHINE, g_ProgIdKey);
    RegDeleteTreeA(HKEY_LOCAL_MACHINE, g_ClsidKey);
    return S_OK;
}

#endif // _WIN32
