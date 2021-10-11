import ctypes

if __name__ == "__main__":
    dll = ctypes.WinDLL ("../build/Debug/rtbtest.dll")
    # setup rtb_init()
    rtb_init = dll.rtb_init
    rtb_init.restype = ctypes.c_void_p
    # setup rtb_showInterfaces()
    rtb_showInterfaces = dll.rtb_showInterfaces
    rtb_showInterfaces.argtypes = [ ctypes.c_void_p ]
    # setup rtb_term()
    rtb_term = dll.rtb_term
    rtb_term.argtypes = [ ctypes.c_void_p ]
    
    h = rtb_init()
    rtb_showInterfaces(h)
    rtb_term(h)