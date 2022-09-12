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
```

## Dependencies

### Windows
- [WinPCap](https://www.winpcap.org/)
