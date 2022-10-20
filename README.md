# rtbessential2d
[![CMake](https://github.com/dspace-group/rtbessential2d/actions/workflows/cmake.yml/badge.svg)](https://github.com/dspace-group/rtbessential2d/actions/workflows/cmake.yml)

Radar Test Bench - Essential 2D Control Library

## Build
```
git clone --recurse-submodules https://github.com/dspace-group/rtbessential2d.git

cd rtbessential2d/
mkdir build
cd build

cmake ..
cmake --build .
```

## Example
```
>>> import pyrtb as Pyrtb
>>> pyrtb = Pyrtb.Pyrtb('build/librtbessential2dctl.so')
>>> pyrtb.Get_interfaces()
[{'name': 'enp0s31f6', 'desc': 'enp0s31f6'}, {'name': 'enx00116b683f83', 'desc': 'enx00116b683f83'}]

>>> pyrtb.Start("enx00116b683f83")
Nanotec Electronic GmbH - N5 EtherCAT Drive, slave 2 set, retval = 39
Nanotec Electronic GmbH - N5 EtherCAT Drive, slave 3 set, retval = 39
Slaves in SAFE_OP
Start working loop...done
Slaves in OP
0

>>> ms = pyrtb.Get_motor_status()
>>> print(ms[0]) # motor 1
Motor control:
- Control word: 31
   Switch on: On
   Enable voltage: On
   Quick stop: On
   Enable operation: On
   Fault reset: Off
   Halt: Off
   Begin on time: Off
- Target position: 0
- Motor drive submode select: 65
   CL/OL: On
   VoS: Off
   Brake: Off
   Current Reduction: Off
   AutoAl: Off
   Torque: Off
   BLDC: On
   Slow : Off
- Modes of operation: 6, 'homing mode'
Motor status:
- Status word: 5687
   Ready to switch on: On
   Switched on: On
   Operation enabled: On
   Fault: Off
   Voltage enabled: On
   Quick stop: On
   Switch on disabled: Off
   Warning: Off
   Remote: On
   Target reached: On
   Internal limit active: Off
- Modes of operation display: 6, 'homing mode'
- Position: 0
- Velocity: 0

>>> pyrtb.Enable_enpo()
0
```

## Dependencies

This work depends on [SOEM](https://github.com/OpenEtherCATsociety/SOEM), a simple open EtherCAT Master Library. SOEM on Windows depends on WinPCap or Npcap. Note that WinPCap nor Npcap are open source software.

### Windows
- [WinPCap](https://www.winpcap.org/) or [Npcap](https://npcap.com/)
