import os, sys
import ctypes
import ctypes.util
from ctypes import *
import argparse

__all__ = ["Pyrtb"]
__version__ = "0.0.1"

class CtypesMotorStatus(ctypes.Structure):
    _fields_ = [
        ("controlword", ctypes.c_uint16),
        ("targetPosition", ctypes.c_int32),
        ("motorDriveSubmodeSelect", ctypes.c_uint32),
        ("modesOfOperation", ctypes.c_int8),
        ("statusword", ctypes.c_uint16),
        ("modesOfOperationDisplay", ctypes.c_int8),
        ("positionActualValue", ctypes.c_int32),
        ("velocityActualValue", ctypes.c_int32)
    ]

class MotorStatus():
    def __init__(self, ctypes_motor_status):
        self.controlword = ctypes_motor_status.controlword
        self.targetPosition = ctypes_motor_status.targetPosition
        self.motorDriveSubmodeSelect = ctypes_motor_status.motorDriveSubmodeSelect
        self.modesOfOperation = ctypes_motor_status.modesOfOperation

        self.statusword = ctypes_motor_status.statusword
        self.modesOfOperationDisplay = ctypes_motor_status.modesOfOperationDisplay
        self.positionActualValue = ctypes_motor_status.positionActualValue
        self.velocityActualValue = ctypes_motor_status.velocityActualValue
    
    def Decode_modesOfOperation(self, value):
        if(value < 0):
            return "manufacturer-specific"
        modesOfOperationTbl = []
        modesOfOperationTbl.append("no mode change / no mode assigned")
        modesOfOperationTbl.append("profile position mode")
        modesOfOperationTbl.append("velocity mode")
        modesOfOperationTbl.append("profile velocity mode")
        modesOfOperationTbl.append("profile torque mode")
        modesOfOperationTbl.append("reserved")
        modesOfOperationTbl.append("homing mode")
        modesOfOperationTbl.append("interpolated position mode")
        modesOfOperationTbl.append("cyclic synchronous position mode")
        modesOfOperationTbl.append("cyclic synchronous velocity mode")
        modesOfOperationTbl.append("cyclic synchronous torque mode")
        return modesOfOperationTbl[value]

    def Decode_controlword(self, value):
        lst = []
        lst.append(("Switch on", value & 1 << 0))
        lst.append(("Enable voltage", value & 1 << 1))
        lst.append(("Quick stop", value & 1 << 2))
        lst.append(("Enable operation", value & 1 << 3))
        #lst.append(("Mode specific 4", value & 1 << 4))
        #lst.append(("Mode specific 5", value & 1 << 5))
        #lst.append(("Mode specific 6", value & 1 << 6))
        lst.append(("Fault reset", value & 1 << 7))
        lst.append(("Halt", value & 1 << 8))
        #lst.append(("Mode specific 9", value & 1 << 9))
        #lst.append(("Reserved", value & 1 << 10))
        lst.append(("Begin on time", value & 1 << 11))
        #lst.append(("manufacturer-specific 12", value & 1 << 12))
        #lst.append(("manufacturer-specific 13", value & 1 << 13))
        #lst.append(("manufacturer-specific 14", value & 1 << 14))
        #lst.append(("manufacturer-specific 15", value & 1 << 15))
        return "\n   ".join((c[0]+ ": " + ("Off" if c[1] == 0 else "On")) for c in lst)
    
    def Decode_statusword(self, value):
        lst = []
        lst.append(("Ready to switch on", value & 1 << 0))
        lst.append(("Switched on", value & 1 << 1))
        lst.append(("Operation enabled", value & 1 << 2))
        lst.append(("Fault", value & 1 << 3))
        lst.append(("Voltage enabled", value & 1 << 4))
        lst.append(("Quick stop", value & 1 << 5))
        lst.append(("Switch on disabled", value & 1 << 6))
        lst.append(("Warning", value & 1 << 7))
        #lst.append(("manufacturer-specific 8", value & 1 << 8))
        lst.append(("Remote", value & 1 << 9))
        lst.append(("Target reached", value & 1 << 10))
        lst.append(("Internal limit active", value & 1 << 11))
        #lst.append(("Mode specific 12", value & 1 << 12))
        #lst.append(("Mode specific 13", value & 1 << 13))
        #lst.append(("manufacturer-specific 14", value & 1 << 14))
        #lst.append(("manufacturer-specific 15", value & 1 << 15))
        return "\n   ".join((c[0]+ ": " + ("Off" if c[1] == 0 else "On")) for c in lst)
    
    def Decode_motordrivesubmodeselect(self, value):
        lst = []
        lst.append(("CL/OL", value & 1 << 0))
        lst.append(("VoS", value & 1 << 1))
        lst.append(("Brake", value & 1 << 2))
        lst.append(("Current Reduction", value & 1 << 3))
        lst.append(("AutoAl", value & 1 << 4))
        lst.append(("Torque", value & 1 << 5))
        lst.append(("BLDC", value & 1 << 6))
        lst.append(("Slow ", value & 1 << 7))
        return "\n   ".join((c[0]+ ": " + ("Off" if c[1] == 0 else "On")) for c in lst)

    def __str__(self):
        resStr = f"""Motor control:
- Control word: {self.controlword}
   {self.Decode_controlword(self.controlword)}
- Target position: {self.targetPosition}
- Motor drive submode select: {self.motorDriveSubmodeSelect}
   {self.Decode_motordrivesubmodeselect(self.motorDriveSubmodeSelect)}
- Modes of operation: {self.modesOfOperation}, '{self.Decode_modesOfOperation(self.modesOfOperation)}'
Motor status:
- Status word: {self.statusword}
   {self.Decode_statusword(self.statusword)}
- Modes of operation display: {self.modesOfOperationDisplay}, '{self.Decode_modesOfOperation(self.modesOfOperationDisplay)}'
- Position: {self.positionActualValue}
- Velocity: {self.velocityActualValue}\n"""
        return resStr

class Pyrtb:
    _lib = None
    
    _init = None
    _show_interfaces = None
    _get_interface = None
    _get_number_of_detected_slaves = None
    _get_slave_information = None
    _start = None
    _term = None
    _setCorrectionFactor = None
    _setAngles = None
    _enableTestbench = None
    _enableSwEnpo = None
    _setOperationMode = None
    _ackError = None
    _enableHoming = None
    _getSimulationTime = None
    _getCtypesMotorStatus = None

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

        # setup rtb_getNumberOfDetectedSlaves()
        self._get_number_of_detected_slaves = self._lib.rtb_getNumberOfDetectedSlaves
        self._get_number_of_detected_slaves.argtypes = [ ctypes.c_void_p, ctypes.c_void_p ]
        self._get_number_of_detected_slaves.restype = ctypes.c_uint

        # setup rtb_getSlaveInformation()
        self._get_slave_information = self._lib.rtb_getSlaveInformation
        self._get_slave_information.argtypes = [ ctypes.c_void_p, ctypes.c_int, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p]
        self._get_slave_information.restype = ctypes.c_uint

        # setup rtb_getNumberOfInterfaces
        self._get_number_of_interfaces = self._lib.rtb_getNumberOfInterfaces
        self._get_number_of_interfaces.argtypes = [ ctypes.c_void_p, ctypes.c_void_p ]
        self._get_number_of_interfaces.restype = ctypes.c_uint

        # setup rtb_getInterface
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

        # setup _setCorrectionFactor
        self._setCorrectionFactor = self._lib.rtb_setCorrectionFactor
        self._setCorrectionFactor.argtypes = [ ctypes.c_void_p, ctypes.c_double, ctypes.c_double, ctypes.c_double, ctypes.c_double ]
        self._setCorrectionFactor.restype = ctypes.c_uint

        # setup _setAngles
        self._setAngles = self._lib.rtb_setAngles
        self._setAngles.argtypes = [ ctypes.c_void_p, ctypes.c_double, ctypes.c_double ]
        self._setAngles.restype = ctypes.c_uint

        # setup _enableTestbench
        self._enableTestbench = self._lib.rtb_enableTestbench
        self._enableTestbench.argtypes = [ ctypes.c_void_p, ctypes.c_bool ]
        self._enableTestbench.restype = ctypes.c_uint

        # setup _enableSwEnpo
        self._enableSwEnpo = self._lib.rtb_enableSwEnpo
        self._enableSwEnpo.argtypes = [ ctypes.c_void_p, ctypes.c_bool ]
        self._enableSwEnpo.restype = ctypes.c_uint

        # setup _ackError
        self._ackError = self._lib.rtb_ackError
        self._ackError.argtypes = [ ctypes.c_void_p ]
        self._ackError.restype = ctypes.c_uint

        # setup _enableHoming
        self._enableHoming = self._lib.rtb_enableHoming
        self._enableHoming.argtypes = [ ctypes.c_void_p, ctypes.c_bool ]
        self._enableHoming.restype = ctypes.c_uint

        # setup _getMotorStatus
        self._getMotorStatus = self._lib.rtb_getMotorStatus
        self._getMotorStatus.argtypes = [ ctypes.c_void_p, POINTER(CtypesMotorStatus), POINTER(CtypesMotorStatus) ]
        self._getMotorStatus.restype = ctypes.c_uint

        # setup _setOperationMode
        self._setOperationMode = self._lib.rtb_setOperationMode
        self._setOperationMode.argtypes = [ ctypes.c_void_p, ctypes.c_uint ]
        self._setOperationMode.restype = ctypes.c_uint

        #setup _getSimulationTime
        self._getSimulationTime = self._lib.rtb_getSimulationTime
        self._getSimulationTime.argtypes = [ ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p ]
        self._getSimulationTime.restype = ctypes.c_uint

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
        n = ctypes.c_uint()
        self._get_number_of_detected_slaves(self._handle, ctypes.byref(n))

        name = ctypes.create_string_buffer(b'\000' * 1024)
        configAdr = ctypes.c_uint()
        manId = ctypes.c_uint()
        prodId = ctypes.c_uint()
        res = []
        for idx in range(n.value):
            self._get_slave_information(self._handle, idx, name, ctypes.byref(configAdr), ctypes.byref(manId), ctypes.byref(prodId))
            res.append({"name" : name.value.decode("utf-8"), "configAdr" : configAdr.value, "manId" : manId.value , "prodId" : prodId.value})
        return res
    
    def Get_state(self):
        state = ctypes.c_uint()
        self._get_state(self._handle, ctypes.byref(state))
        return state.value
   
    def Start(self, interface):
        return self._start(self._handle, interface.encode('UTF-8'))
    
    def Stop(self):
        return self._stop(self._handle)
    
    def Set_correction_factor(self, cf_m1, cf_m2, offset_m1, offset_m2):
        return self._setCorrectionFactor(self._handle, cf_m1, cf_m2, offset_m1, offset_m2)
    
    def Set_angles(self, az_deg, el_deg):
        return self._setAngles(self._handle, az_deg, el_deg)
    
    def Set_operation_mode(self, om):
        # 6: Homing
        # 1: Control
        # 3: Jog
        return self._setOperationMode(self._handle, om)
    
#    def Enable_testbench(self, enable=True):
#        return self._enableTestbench(self._handle, enable)
    
    def Enable_enpo(self, enable=True):
        return self._enableSwEnpo(self._handle, enable)
    
    def Ack_error(self):
        return self._ackError(self._handle)
    
    def Enable_homing(self, enable=True):
        return self._enableHoming(self._handle, enable)

    def Get_motor_status(self):
        m1 = CtypesMotorStatus()
        m2 = CtypesMotorStatus()
        rc = self._getMotorStatus(self._handle, ctypes.byref(m1), ctypes.byref(m2))
        if(rc != 0):
            return (None, None)
        
        return (MotorStatus(m1), MotorStatus(m2))

    def Get_simulation_time(self):
        t = ctypes.c_double()
        steps = ctypes.c_uint()
        res = self._getSimulationTime(self._handle, ctypes.byref(t), ctypes.byref(steps))
        if(res == 0):
            return [t.value / 1e6, steps.value]
        return [None, None]
        