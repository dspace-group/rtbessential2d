# rtbessential2d
[![CMake](https://github.com/dspace-group/rtbessential2d/actions/workflows/cmake.yml/badge.svg)](https://github.com/dspace-group/rtbessential2d/actions/workflows/cmake.yml)

Radar Test Bench - Essential 2D Control Library

## Preconditions

Please ensure that the following tools are installed and appropriately advertised via the PATH environment variable.

### Linux

- gcc (`apt install build-essential`)
- [CMake](https://cmake.org/) (`apt install cmake`)

### Windows

- MSVC (Microsoft Visual C++ or [Microsoft C++ Build Tools](https://visualstudio.microsoft.com/visual-cpp-build-tools/)) compiler toolchain
- make (install [choco](https://chocolatey.org/install) and then install make with `choco install make`)
- [CMake](https://cmake.org/)

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

## Python wrapper functions
- `pyrtb = Pyrtb.Pyrtb(path)` Initialize Python pyrtb wrapper with the path to the shared library (.so or .dll)
- `pyrtb.Get_state()` Returns the current state of the library (0: Initialized, 1: Starting, 2: Started)
- `pyrtb.Get_interfaces()` Returns a list of interfaces (names and descriptions)
- `pyrtb.Start(interface_name)` Start EtherCAT Master on the given interface
- `pyrtb.Stop()` Stops EtherCAT Master
- `pyrtb.Get_detected_slaves()` Returns a list with the detected EtherCAT Slaves
- `pyrtb.Set_correction_factor(cf_m1, cf_m2, offset_m1, offset_m2)` Set correction factors and offsets for motor1 and motor2
- `pyrtb.Set_angles(az, el)` Set azimuth and elevation (in deg)
- `pyrtb.Enable_enpo(enabled=True)` Enable (or disable) motor controller enpo. Motors won't move if enpo isn't enabled. After initialization enpo is disabled.
- `pyrtb.Ack_error()` Reset motor errors
- `pyrtb.Enable_homing()` Enable homing
- `pyrtb.Get_motor_status()` Return list with status objects for for motor1 and motor2. (`print(pyrtb.Get_motor_status()[0])`)
- `pyrtb.Get_simulation_time()` Return list with the current simulation time and the current simulation step. The values are updated only when the EtherCAT network is in operation.

## Cooking recipe
1. Build library as documented above
2. Start a Python interpreter and load the wrapper with `import pyrtb as Pyrtb`. If your user doesn't have the rights to control the Ethernet interface of your computer, then start the Python interpreter with elevated user rights (Windows: Run as Administrator, Linux: `sudo python`)
3. Create a wrapper object and load the shared library with `>>> pyrtb = Pyrtb.Pyrtb('../build/librtbessential2dctl.so')`
4. List the available interfaces with `>>> pyrtb.Get_interfaces()`
5. Start EtherCAT Master. E.g. with `>>> pyrtb.Start('eth0')`
6. Check whether RTB EtherCAT Slaves were found with `>>> pyrtb.Get_detected_slaves()`
7. Use `pyrtb.Enable_enpo()`, `pyrtb.Set_correction_factor()`, `pyrtb.Set_angles(az, el)`, `pyrtb.Enable_homing()` to write your application.
8. Use `pyrtb.Ack_error()`, `pyrtb.Get_motor_status()`, `pyrtb.Get_simulation_time()` for debugging purposes.

## Dependencies

This work depends on [SOEM](https://github.com/OpenEtherCATsociety/SOEM)([License](https://github.com/OpenEtherCATsociety/SOEM/blob/master/LICENSE)), a simple open EtherCAT Master Library. SOEM on Windows depends on WinPCap or Npcap. Note that neither WinPCap nor Npcap are open source software.

### Windows
- [WinPCap](https://www.winpcap.org/) or [Npcap](https://npcap.com/)
