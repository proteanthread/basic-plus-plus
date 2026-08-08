#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

use std::os::raw::{c_char, c_int, c_void};

pub type BppEngineContext = c_void;

#[repr(C)]
pub enum BppValueType {
    BPP_VAL_NULL = 0,
    BPP_VAL_NUMBER = 1,
    BPP_VAL_STRING = 2,
    BPP_VAL_ERROR = 3,
}

extern "C" {
    pub fn bpp_init(ram_bytes: usize) -> *mut BppEngineContext;
    pub fn bpp_shutdown(ctx: *mut BppEngineContext);
    pub fn bpp_exec_string(ctx: *mut BppEngineContext, code: *const c_char) -> c_int;
    pub fn bpp_load_and_run(ctx: *mut BppEngineContext, filepath: *const c_char) -> c_int;
    pub fn bpp_version_string() -> *const c_char;
}
