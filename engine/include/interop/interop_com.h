// FILENAME: interop_com.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (interop_com.c, interop_com_register.c)
// NEEDS: platform, memory
// Provides core logic and interface definitions for interop_com within BASIC++.
//
// ---- Includes ----

#ifndef INTEROP_COM_H
#define INTEROP_COM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#include <ole2.h>
#include <oleauto.h>

// IID_IBasicPPEngine
DEFINE_GUID(IID_IBasicPPEngine, 0xb45f89a5, 0xf912, 0x4217, 0xb2, 0x7d, 0xfa, 0x5a, 0x2d, 0xb2, 0xa7, 0x7a);
// CLSID_BasicPPEngine
DEFINE_GUID(CLSID_BasicPPEngine, 0x1a7de99c, 0x8a5e, 0x4b07, 0xa9, 0x61, 0xa0, 0xbc, 0x3d, 0x78, 0x2d, 0xe1);
// LIBID_BasicPP
DEFINE_GUID(LIBID_BasicPPLib, 0xe51e952e, 0x5374, 0x429c, 0xa8, 0xd9, 0xb5, 0x8d, 0xf8, 0xf9, 0x5f, 0xbd);

#ifdef __cplusplus
extern "C" {
#endif

typedef struct IBasicPPEngine IBasicPPEngine;

typedef struct IBasicPPEngineVtbl {
    // IUnknown
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IBasicPPEngine *This, REFIID riid, void **ppvObject);
    ULONG (STDMETHODCALLTYPE *AddRef)(IBasicPPEngine *This);
    ULONG (STDMETHODCALLTYPE *Release)(IBasicPPEngine *This);

    // IDispatch
    HRESULT (STDMETHODCALLTYPE *GetTypeInfoCount)(IBasicPPEngine *This, UINT *pctinfo);
    HRESULT (STDMETHODCALLTYPE *GetTypeInfo)(IBasicPPEngine *This, UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo);
    HRESULT (STDMETHODCALLTYPE *GetIDsOfNames)(IBasicPPEngine *This, REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId);
    HRESULT (STDMETHODCALLTYPE *Invoke)(IBasicPPEngine *This, DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr);

    // IBasicPPEngine
    HRESULT (STDMETHODCALLTYPE *Exec)(IBasicPPEngine *This, BSTR code, long *result);
    HRESULT (STDMETHODCALLTYPE *Eval)(IBasicPPEngine *This, BSTR expression, VARIANT *result);
    HRESULT (STDMETHODCALLTYPE *GetVar)(IBasicPPEngine *This, BSTR varName, VARIANT *value);
    HRESULT (STDMETHODCALLTYPE *SetVar)(IBasicPPEngine *This, BSTR varName, VARIANT value);
    HRESULT (STDMETHODCALLTYPE *Version)(IBasicPPEngine *This, BSTR *version);
    HRESULT (STDMETHODCALLTYPE *Init)(IBasicPPEngine *This, long ramMB);
    HRESULT (STDMETHODCALLTYPE *Shutdown)(IBasicPPEngine *This);
} IBasicPPEngineVtbl;

struct IBasicPPEngine {
    const IBasicPPEngineVtbl *lpVtbl;
};

// Standard DLL exports
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv);
STDAPI DllCanUnloadNow(void);
STDAPI DllRegisterServer(void);
STDAPI DllUnregisterServer(void);

#ifdef __cplusplus
}
#endif

#endif // _WIN32
#endif // INTEROP_COM_H
