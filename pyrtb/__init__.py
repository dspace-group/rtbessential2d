import os, sys
import ctypes
import ctypes.util
import argparse

__all__ = ["Pyrtb"]
__version__ = "0.0.1"

class Pyrtb:
    _lib = None
    
    _init = None
    _show_interfaces = None
    _get_number_of_interfaces = None
    _get_interface = None
    _start = None
    _term = None

    _handle = None

    def __init__(self, path_to_library):
        if os.name == "posix":
            self._lib = ctypes.CDLL(path_to_library) 
        if os.name == "nt":
            self._lib = ctypes.WinDLL (path_to_library)
        
        # setup rtb_init()
        self._init = self._lib.rtb_init
        self._init.restype = ctypes.c_void_p

        # setup rtb_getState()
        self._get_state = self._lib.rtb_getState
        self._get_state.argtypes = [ ctypes.c_void_p, ctypes.c_void_p ]
        self._get_state.restype = ctypes.c_void_p

        # setup rtb_showInterfaces()
        self._show_interfaces = self._lib.rtb_showInterfaces
        self._show_interfaces.argtypes = [ ctypes.c_void_p ]
        self._show_interfaces.restype = ctypes.c_uint

        # setup rtb_showdetectedslaves
        self._show_detected_slaves = self._lib.rtb_showdetectedslaves
        self._show_detected_slaves.argtypes = [ ctypes.c_void_p, ctypes.c_char_p ]
        self._show_detected_slaves.restype = ctypes.c_uint

        self._get_number_of_interfaces = self._lib.rtb_getNumberOfInterfaces
        self._get_number_of_interfaces.argtypes = [ ctypes.c_void_p, ctypes.c_void_p ]
        self._get_number_of_interfaces.restype = ctypes.c_uint

        self._get_interface = self._lib.rtb_getInterface
        self._get_interface.argtypes = [ ctypes.c_void_p, ctypes.c_uint, ctypes.c_char_p, ctypes.c_char_p ]
        self._get_interface.restype = ctypes.c_uint
        
        # setup rtb_start()
        self._start = self._lib.rtb_start
        self._start.argtypes = [ ctypes.c_void_p, ctypes.c_char_p ]
        self._start.restype = ctypes.c_uint

        # setup rtb_stop()
        self._stop = self._lib.rtb_stop
        self._stop.argtypes = [ ctypes.c_void_p ]
        self._stop.restype = ctypes.c_uint

        # setup rtb_term()
        self._term = self._lib.rtb_term
        self._term.argtypes = [ ctypes.c_void_p ]
        self._term.restype = ctypes.c_uint

        # initialize
        self._handle = self._init()

    def __del__(self):
        self._term(self._handle)
    
    def Get_interfaces(self):
        blacklist = ["lo", "wlan0", "eth0"]

        n = ctypes.c_uint()
        name = ctypes.create_string_buffer(b'\000' * 1024)
        desc = ctypes.create_string_buffer(b'\000' * 1024)
        self._get_number_of_interfaces(self._handle, ctypes.byref(n))
        res = []
        for idx in range(n.value):
            self._get_interface(self._handle, idx, name, desc)
            if name.value.decode("utf-8") in blacklist:
                continue
            res.append({"name" : name.value.decode("utf-8"), "desc" : desc.value.decode("utf-8")})

        return res
    
    def Get_detected_slaves(self):
        buf = ctypes.create_string_buffer(b'\000' * 2 * 1024 * 1024)
        self._show_detected_slaves(self._handle, buf)
        return buf.value.decode("utf-8")
    
    def Get_state(self):
        state = ctypes.c_uint()
        self._get_state(self._handle, ctypes.byref(state))
        return state.value
   
    def Start(self, interface):
        return self._start(self._handle, interface.encode('UTF-8'))
    
    def Stop(self):
        return self._stop(self._handle)