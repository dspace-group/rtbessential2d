# rtbessential2d
Radar Test Bench - Essential 2D Control Library

## Build
```
git clone --recurse-submodules https://github.com/dspace-group/rtbessential2d.git

cd rtbessential2d/
mkdir build
cd build

cmake ..
make
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
- Control word: 0
   Switch on: Off
   Enable voltage: Off
   Quick stop: Off
   Enable operation: Off
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
- Status word: 4616
   Ready to switch on: Off
   Switched on: Off
   Operation enabled: Off
   Fault: On
   Voltage enabled: Off
   Quick stop: Off
   Switch on disabled: Off
   Warning: Off
   Remote: On
   Target reached: Off
   Internal limit active: Off
- Modes of operation display: 6, 'homing mode'
- Position: 68
- Velocity: 0

>>> pyrtb.Enable_enpo()
0
```

## Dependencies

### Windows
- [WinPCap](https://www.winpcap.org/)
