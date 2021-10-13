import ctypes
import argparse

if __name__ == "__main__":
    dll = ctypes.WinDLL ("../build/Debug/rtbessential2dctl.dll")

    parser = argparse.ArgumentParser(description='dSPACE Radar Testbench - Essential 2D Control')
    parser.add_argument('-ifs', '--interfaces', help='List available interfaces', action="store_true")
    parser.add_argument('-if', '--interface', help='Initialize EtherCAT Master on the given interface')
    
    args = parser.parse_args()

    # setup rtb_init()
    rtb_init = dll.rtb_init
    rtb_init.restype = ctypes.c_void_p

    # setup rtb_showInterfaces()
    rtb_showInterfaces = dll.rtb_showInterfaces
    rtb_showInterfaces.argtypes = [ ctypes.c_void_p ]

    # setup rtb_start()
    rtb_start = dll.rtb_start
    rtb_start.argtypes = [ ctypes.c_void_p, ctypes.c_char_p ]

    # setup rtb_term()
    rtb_term = dll.rtb_term
    rtb_term.argtypes = [ ctypes.c_void_p ]

    # run library
    h = rtb_init()

    if args.interfaces:
        rtb_showInterfaces(h)
    
    if args.interface:
        rc = rtb_start(h, args.interface.encode('UTF-8'))

    rtb_term(h)