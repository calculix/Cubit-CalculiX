import ctypes as c
from ctypes import wintypes as w
import os
from pathlib import Path
import struct
import subprocess
import time

root = Path(__file__).resolve().parents[2]
output = root / "startup-diagnostics" / "current-debug"
output.mkdir(parents=True, exist_ok=True)
kernel = c.WinDLL("kernel32", use_last_error=True)
dbg = c.WinDLL("dbghelp", use_last_error=True)
kernel.OpenThread.argtypes = [w.DWORD, w.BOOL, w.DWORD]
kernel.OpenThread.restype = w.HANDLE
kernel.GetThreadContext.argtypes = [w.HANDLE, c.c_void_p]
kernel.ContinueDebugEvent.argtypes = [w.DWORD, w.DWORD, w.DWORD]
kernel.WaitForDebugEvent.argtypes = [c.c_void_p, w.DWORD]
kernel.CloseHandle.argtypes = [w.HANDLE]
dbg.SymInitialize.argtypes = [w.HANDLE, c.c_char_p, w.BOOL]
dbg.SymFromAddr.argtypes = [w.HANDLE, c.c_uint64, c.POINTER(c.c_uint64), c.c_void_p]
dbg.SymGetModuleBase64.argtypes = [w.HANDLE, c.c_uint64]
dbg.SymGetModuleBase64.restype = c.c_uint64
dbg.SymFunctionTableAccess64.argtypes = [w.HANDLE, c.c_uint64]
dbg.SymFunctionTableAccess64.restype = c.c_void_p
dbg.StackWalk64.argtypes = [w.DWORD, w.HANDLE, w.HANDLE, c.c_void_p, c.c_void_p, c.c_void_p, c.c_void_p, c.c_void_p, c.c_void_p]
dbg.SymLoadModuleExW.argtypes = [w.HANDLE, w.HANDLE, w.LPCWSTR, w.LPCWSTR, c.c_uint64, w.DWORD, c.c_void_p, w.DWORD]
dbg.SymLoadModuleExW.restype = c.c_uint64
kernel.GetFinalPathNameByHandleW.argtypes = [w.HANDLE, w.LPWSTR, w.DWORD, w.DWORD]
env = os.environ.copy()
env["CUBIT_PLUGIN_DIR"] = str(root / "build/Release")
env["CUBITSIM_STARTUP_TRACE"] = str(output / "debug-startup.log")
env["PATH"] = "C:/Program Files/Coreform Cubit 2026.11/acis/code/bin;" + env.get("PATH", "")
env.pop("VIRTUAL_ENV", None)
startup = subprocess.STARTUPINFO()
startup.dwFlags |= subprocess.STARTF_USESHOWWINDOW
startup.wShowWindow = 0
with (output / "debug-stdout.log").open("wb") as stdout, (output / "debug-stderr.log").open("wb") as stderr:
    process = subprocess.Popen(["C:/Program Files/Coreform Cubit 2026.11/bin/clarox.exe", "-prompt", "Cubit"], cwd=output, env=env, startupinfo=startup, stdout=stdout, stderr=stderr, creationflags=2)
    handle = w.HANDLE(int(process._handle))
    dbg.SymSetOptions(0x00080206)
    print("Diagnostic PID:", process.pid, flush=True)
    modules = {}
    deadline = time.monotonic() + 60
    while time.monotonic() < deadline:
        event = c.create_string_buffer(176)
        if not kernel.WaitForDebugEvent(event, 1000):
            continue
        code, pid, tid = struct.unpack_from("<III", event.raw)
        status = 0x10002
        if code in [3, 6]:
            base_offset = 40
            if code == 6:
                base_offset = 24
            file_handle = struct.unpack_from("<Q", event.raw, 16)[0]
            base = struct.unpack_from("<Q", event.raw, base_offset)[0]
            path_buffer = c.create_unicode_buffer(4096)
            kernel.GetFinalPathNameByHandleW(w.HANDLE(file_handle), path_buffer, 4096, 0)
            path = path_buffer.value
            if path:
                modules[base] = path
                kernel.CloseHandle(w.HANDLE(file_handle))
        if code == 1:
            exception = struct.unpack_from("<I", event.raw, 16)[0]
            first = struct.unpack_from("<I", event.raw, 168)[0]
            if exception not in [0x80000003, 0x80000004]:
                status = 0x80010001
            if exception == 0xc0000005:
                print("ACCESS VIOLATION first chance:", first, flush=True)
                thread = kernel.OpenThread(0x1fffff, False, tid)
                storage = c.create_string_buffer(1250)
                address = (c.addressof(storage) + 15) & ~15
                context = (c.c_char * 1232).from_address(address)
                struct.pack_into("<I", context, 48, 0x10000b)
                if kernel.GetThreadContext(thread, context):
                    frame = c.create_string_buffer(1024)
                    rip = struct.unpack_from("<Q", context, 248)[0]
                    rsp = struct.unpack_from("<Q", context, 152)[0]
                    rbp = struct.unpack_from("<Q", context, 160)[0]
                    for offset, value in [(0, rip), (32, rbp), (48, rsp)]:
                        struct.pack_into("<QII", frame, offset, value, 0, 3)
                    for index in range(35):
                        pc = struct.unpack_from("<Q", frame.raw, 0)[0]
                        symbol = c.create_string_buffer(1100)
                        struct.pack_into("<I", symbol, 0, 88)
                        struct.pack_into("<I", symbol, 80, 1000)
                        displacement = c.c_uint64()
                        name = ""
                        if False:
                            name = symbol.raw[84:].split(b"\0")[0].decode(errors="replace")
                        bases = [base for base in modules if base <= pc]
                        location = hex(pc)
                        if bases:
                            base = max(bases)
                            location = Path(modules[base]).name + "+" + hex(pc-base)
                        print(index, location, name, hex(displacement.value), flush=True)
                        if not dbg.StackWalk64(0x8664, handle, thread, frame, context, None, c.cast(dbg.SymFunctionTableAccess64, c.c_void_p), c.cast(dbg.SymGetModuleBase64, c.c_void_p), None):
                            break
                kernel.CloseHandle(thread)
        kernel.ContinueDebugEvent(pid, tid, status)
        if code == 5:
            print("Debuggee exited", flush=True)
            break
    else:
        process.terminate()
