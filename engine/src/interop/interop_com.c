// FILENAME: interop_com.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (interop_com_register.c)
// NEEDS: libcore (interop_com.h, string.h)
// NEEDS: libengine (string.c)
// Provides core logic and interface definitions for interop_com within BASIC++.
//
// ---- Includes ----

#include "interop/interop_com.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32

typedef struct {
    IBasicPPEngineVtbl *lpVtbl;
    LONG ref_count;
} BasicPPEngineImpl;

static HRESULT STDMETHODCALLTYPE basicpp_QueryInterface(IBasicPPEngine *This, REFIID riid, void **ppvObject) {
    if (!ppvObject) return E_POINTER;
    *ppvObject = NULL;

    if (IsEqualIID(riid, &IID_IUnknown) ||
        IsEqualIID(riid, &IID_IDispatch) ||
        IsEqualIID(riid, &IID_IBasicPPEngine)) {
        *ppvObject = This;
        This->lpVtbl->AddRef(This);
        return S_OK;
    }
    return E_NOINTERFACE;
}

static ULONG STDMETHODCALLTYPE basicpp_AddRef(IBasicPPEngine *This) {
    BasicPPEngineImpl *impl = (BasicPPEngineImpl *)This;
    return InterlockedIncrement(&impl->ref_count);
}

static ULONG STDMETHODCALLTYPE basicpp_Release(IBasicPPEngine *This) {
    BasicPPEngineImpl *impl = (BasicPPEngineImpl *)This;
    ULONG count = InterlockedDecrement(&impl->ref_count);
    if (count == 0) {
        free(impl);
    }
    return count;
}

static HRESULT STDMETHODCALLTYPE basicpp_GetTypeInfoCount(IBasicPPEngine *This, UINT *pctinfo) {
    if (!pctinfo) return E_POINTER;
    *pctinfo = 0;
    return S_OK;
}

static HRESULT STDMETHODCALLTYPE basicpp_GetTypeInfo(IBasicPPEngine *This, UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo) {
    if (!ppTInfo) return E_POINTER;
    *ppTInfo = NULL;
    return E_NOTIMPL;
}

static HRESULT STDMETHODCALLTYPE basicpp_GetIDsOfNames(IBasicPPEngine *This, REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId) {
    if (!rgDispId || cNames == 0) return E_POINTER;
    for (UINT i = 0; i < cNames; ++i) {
        if (_wcsicmp(rgszNames[i], L"Exec") == 0) rgDispId[i] = 1;
        else if (_wcsicmp(rgszNames[i], L"Eval") == 0) rgDispId[i] = 2;
        else if (_wcsicmp(rgszNames[i], L"GetVar") == 0) rgDispId[i] = 3;
        else if (_wcsicmp(rgszNames[i], L"SetVar") == 0) rgDispId[i] = 4;
        else if (_wcsicmp(rgszNames[i], L"Version") == 0) rgDispId[i] = 5;
        else if (_wcsicmp(rgszNames[i], L"Init") == 0) rgDispId[i] = 6;
        else if (_wcsicmp(rgszNames[i], L"Shutdown") == 0) rgDispId[i] = 7;
        else rgDispId[i] = DISPID_UNKNOWN;
    }
    return S_OK;
}

static HRESULT STDMETHODCALLTYPE basicpp_Invoke(IBasicPPEngine *This, DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr) {
    return E_NOTIMPL;
}

static HRESULT STDMETHODCALLTYPE basicpp_Exec(IBasicPPEngine *This, BSTR code, long *result) {
    if (result) *result = 0;
    return S_OK;
}

static HRESULT STDMETHODCALLTYPE basicpp_Eval(IBasicPPEngine *This, BSTR expression, VARIANT *result) {
    if (result) VariantInit(result);
    return S_OK;
}

static HRESULT STDMETHODCALLTYPE basicpp_GetVar(IBasicPPEngine *This, BSTR varName, VARIANT *value) {
    if (value) VariantInit(value);
    return S_OK;
}

static HRESULT STDMETHODCALLTYPE basicpp_SetVar(IBasicPPEngine *This, BSTR varName, VARIANT value) {
    return S_OK;
}

static HRESULT STDMETHODCALLTYPE basicpp_Version(IBasicPPEngine *This, BSTR *version) {
    if (version) *version = SysAllocString(L"6.5");
    return S_OK;
}

static HRESULT STDMETHODCALLTYPE basicpp_Init(IBasicPPEngine *This, long ramMB) {
    return S_OK;
}

static HRESULT STDMETHODCALLTYPE basicpp_Shutdown(IBasicPPEngine *This) {
    return S_OK;
}

static IBasicPPEngineVtbl g_BasicPPEngineVtbl = {
    basicpp_QueryInterface,
    basicpp_AddRef,
    basicpp_Release,
    basicpp_GetTypeInfoCount,
    basicpp_GetTypeInfo,
    basicpp_GetIDsOfNames,
    basicpp_Invoke,
    basicpp_Exec,
    basicpp_Eval,
    basicpp_GetVar,
    basicpp_SetVar,
    basicpp_Version,
    basicpp_Init,
    basicpp_Shutdown
};

typedef struct {
    IClassFactoryVtbl *lpVtbl;
    LONG ref_count;
} ClassFactoryImpl;

static HRESULT STDMETHODCALLTYPE cf_QueryInterface(IClassFactory *This, REFIID riid, void **ppvObject) {
    if (!ppvObject) return E_POINTER;
    *ppvObject = NULL;
    if (IsEqualIID(riid, &IID_IUnknown) || IsEqualIID(riid, &IID_IClassFactory)) {
        *ppvObject = This;
        This->lpVtbl->AddRef(This);
        return S_OK;
    }
    return E_NOINTERFACE;
}

static ULONG STDMETHODCALLTYPE cf_AddRef(IClassFactory *This) {
    ClassFactoryImpl *impl = (ClassFactoryImpl *)This;
    return InterlockedIncrement(&impl->ref_count);
}

static ULONG STDMETHODCALLTYPE cf_Release(IClassFactory *This) {
    ClassFactoryImpl *impl = (ClassFactoryImpl *)This;
    ULONG count = InterlockedDecrement(&impl->ref_count);
    if (count == 0) {
        free(impl);
    }
    return count;
}

static HRESULT STDMETHODCALLTYPE cf_CreateInstance(IClassFactory *This, IUnknown *pUnkOuter, REFIID riid, void **ppvObject) {
    if (pUnkOuter) return CLASS_E_NOAGGREGATION;
    if (!ppvObject) return E_POINTER;

    BasicPPEngineImpl *engine = (BasicPPEngineImpl *)calloc(1, sizeof(BasicPPEngineImpl));
    if (!engine) return E_OUTOFMEMORY;

    engine->lpVtbl = &g_BasicPPEngineVtbl;
    engine->ref_count = 1;

    HRESULT hr = engine->lpVtbl->QueryInterface((IBasicPPEngine *)engine, riid, ppvObject);
    engine->lpVtbl->Release((IBasicPPEngine *)engine);
    return hr;
}

static HRESULT STDMETHODCALLTYPE cf_LockServer(IClassFactory *This, BOOL fLock) {
    return S_OK;
}

static IClassFactoryVtbl g_ClassFactoryVtbl = {
    cf_QueryInterface,
    cf_AddRef,
    cf_Release,
    cf_CreateInstance,
    cf_LockServer
};

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv) {
    if (!IsEqualCLSID(rclsid, &CLSID_BasicPPEngine)) return CLASS_E_CLASSNOTAVAILABLE;
    
    ClassFactoryImpl *cf = (ClassFactoryImpl *)calloc(1, sizeof(ClassFactoryImpl));
    if (!cf) return E_OUTOFMEMORY;
    
    cf->lpVtbl = &g_ClassFactoryVtbl;
    cf->ref_count = 1;

    HRESULT hr = cf->lpVtbl->QueryInterface((IClassFactory *)cf, riid, ppv);
    cf->lpVtbl->Release((IClassFactory *)cf);
    return hr;
}

STDAPI DllCanUnloadNow(void) {
    return S_OK;
}

#endif // _WIN32
