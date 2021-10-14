import os
import ctypes
import argparse

if __name__ == "__main__":
    lib = None
    if os.name == "posix":
        lib = ctypes.CDLL("../build/librtbessential2dctl.so") 
    if os.name == "nt":
        lib = ctypes.WinDLL ("../build/Debug/rtbessential2dctl.dll")

    parser = argparse.ArgumentParser(description='dSPACE Radar Testbench - Essential 2D Control')
    parser.add_argument('-ifs', '--interfaces', help='List available interfaces', action="store_true")
    parser.add_argument('-if', '--interface', help='Initialize EtherCAT Master on the given interface')
    
    args = parser.parse_args()

    # setup rtb_init()
    rtb_init = lib.rtb_init
    rtb_init.restype = ctypes.c_void_p

    # setup rtb_showInterfaces()
    rtb_showInterfaces = lib.rtb_showInterfaces
    rtb_showInterfaces.argtypes = [ ctypes.c_void_p ]

    # setup rtb_start()
    rtb_start = lib.rtb_start
    rtb_start.argtypes = [ ctypes.c_void_p, ctypes.c_char_p ]

    # setup rtb_term()
    rtb_term = lib.rtb_term
    rtb_term.argtypes = [ ctypes.c_void_p ]

    # run library
    h = rtb_init()

    if args.interfaces:
        rtb_showInterfaces(h)
    
    if args.interface:
        rc = rtb_start(h, args.interface.encode('UTF-8'))

    rtb_term(h)