import ctypes
import os
import sys

def _load_lib():
    lib_names = ["basicpp.dll", "baspp.exe", "libbasicpp.so", "libbasicpp.dylib"]
    search_dirs = [
        os.path.dirname(__file__),
        os.path.abspath(os.path.join(os.path.dirname(__file__), "../../../build_win/Release")),
        os.path.abspath(os.path.join(os.path.dirname(__file__), "../../../build_win")),
        os.path.abspath(os.path.join(os.path.dirname(__file__), "../../../build")),
        "/usr/local/lib",
        "/usr/lib"
    ]
    for d in search_dirs:
        if os.path.exists(d) and hasattr(os, 'add_dll_directory'):
            try:
                os.add_dll_directory(d)
            except Exception:
                pass
        for name in lib_names:
            path = os.path.join(d, name)
            if os.path.exists(path):
                try:
                    return ctypes.CDLL(path)
                except Exception as e:
                    pass
    return None

_lib = _load_lib()

class BppValueUnion(ctypes.Union):
    _fields_ = [
        ("number", ctypes.c_double),
        ("string", ctypes.c_char_p),
        ("error_code", ctypes.c_int)
    ]

class BppValueStruct(ctypes.Structure):
    _fields_ = [
        ("type", ctypes.c_int),
        ("val", BppValueUnion)
    ]

class VM:
    def __init__(self, ram_mb: int = 640):
        self._ctx = None
        if _lib and hasattr(_lib, 'bpp_init'):
            _lib.bpp_init.argtypes = [ctypes.c_size_t]
            _lib.bpp_init.restype = ctypes.c_void_p
            ram_bytes = ram_mb * 1024 * 1024
            self._ctx = _lib.bpp_init(ram_bytes)

    def exec(self, code: str) -> int:
        if not self._ctx or not _lib:
            print(f"[BASIC++] Mock Exec: {code}")
            return 0
        _lib.bpp_exec_string.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
        _lib.bpp_exec_string.restype = ctypes.c_int
        return _lib.bpp_exec_string(self._ctx, code.encode('utf-8'))

    def eval(self, expr: str):
        if not self._ctx or not _lib or not hasattr(_lib, 'bpp_eval_expr'):
            return 0.0
        _lib.bpp_eval_expr.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
        _lib.bpp_eval_expr.restype = BppValueStruct

        val_struct = _lib.bpp_eval_expr(self._ctx, expr.encode('utf-8'))
        val_type = val_struct.type
        if val_type == 1: # BPP_VAL_NUMBER
            return val_struct.val.number
        elif val_type == 2: # BPP_VAL_STRING
            s = val_struct.val.string.decode('utf-8') if val_struct.val.string else ""
            if hasattr(_lib, 'bpp_value_release'):
                _lib.bpp_value_release(ctypes.byref(val_struct))
            return s
        elif val_type == 3: # BPP_VAL_ERROR
            raise RuntimeError(f"BASIC++ Eval Error code {val_struct.val.error_code}")
        return None

    def __getitem__(self, var_name: str):
        if not self._ctx or not _lib:
            return 0.0
        if var_name.endswith('$'):
            buf = ctypes.create_string_buffer(512)
            _lib.bpp_get_var_str.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_size_t]
            _lib.bpp_get_var_str.restype = ctypes.c_bool
            ok = _lib.bpp_get_var_str(self._ctx, var_name.encode('utf-8'), buf, 512)
            return buf.value.decode('utf-8') if ok else ""
        else:
            _lib.bpp_get_var_num.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
            _lib.bpp_get_var_num.restype = ctypes.c_double
            return _lib.bpp_get_var_num(self._ctx, var_name.encode('utf-8'))

    def __setitem__(self, var_name: str, value):
        if not self._ctx or not _lib:
            return
        if isinstance(value, str) or var_name.endswith('$'):
            _lib.bpp_set_var_str.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_char_p]
            _lib.bpp_set_var_str.restype = ctypes.c_int
            _lib.bpp_set_var_str(self._ctx, var_name.encode('utf-8'), str(value).encode('utf-8'))
        else:
            _lib.bpp_set_var_num.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_double]
            _lib.bpp_set_var_num.restype = ctypes.c_int
            _lib.bpp_set_var_num(self._ctx, var_name.encode('utf-8'), float(value))

    def function(self, func_name=None):
        def decorator(fn):
            name = func_name if func_name else fn.__name__.upper()
            if self._ctx and _lib and hasattr(_lib, 'bpp_register_func'):
                pass
            return fn
        return decorator

    def close(self):
        if self._ctx and _lib and hasattr(_lib, 'bpp_shutdown'):
            _lib.bpp_shutdown.argtypes = [ctypes.c_void_p]
            _lib.bpp_shutdown(self._ctx)
            self._ctx = None

def version() -> str:
    if _lib and hasattr(_lib, 'bpp_version_string'):
        _lib.bpp_version_string.restype = ctypes.c_char_p
        return _lib.bpp_version_string().decode('utf-8')
    return "6.5.2"
