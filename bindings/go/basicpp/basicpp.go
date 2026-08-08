package basicpp

/*
#cgo CFLAGS: -I../../engine/include
#cgo LDFLAGS: -L../../build_win/Release -lbasicpp
#include "bpp_api.h"
*/
import "C"
import (
	"errors"
	"unsafe"
)

type Engine struct {
	ctx *C.BppEngineContext
}

func NewEngine(ramBytes uint64) (*Engine, error) {
	ctx := C.bpp_init(C.size_t(ramBytes))
	if ctx == nil {
		return nil, errors.New("failed to initialize BASIC++ engine context")
	}
	return &Engine{ctx: ctx}, nil
}

func (e *Engine) Close() {
	if e.ctx != nil {
		C.bpp_shutdown(e.ctx)
		e.ctx = nil
	}
}

func (e *Engine) Exec(code string) error {
	cCode := C.CString(code)
	defer C.free(unsafe.Pointer(cCode))

	ret := C.bpp_exec_string(e.ctx, cCode)
	if ret != 0 {
		return errors.New("BASIC++ execution failed with error code")
	}
	return nil
}

func Version() string {
	return C.GoString(C.bpp_version_string())
}
