import sys
import time
sys.path.append("..")

import unittest
import pyrtb as Pyrtb

ETH_INTERFACE = "enx00116ba0922c"
M1 = 0
M2 = 0

class TestPyRtbLib(unittest.TestCase):
    def test_init(self):
        pyrtb = Pyrtb.Pyrtb('../build/librtbessential2dctl.so')
        self.assertTrue(pyrtb != None)

    def test_getInterfaces(self):
        pyrtb = Pyrtb.Pyrtb('../build/librtbessential2dctl.so')

        ifs = pyrtb.Get_interfaces()
        print("Interfaces:")
        for if_ in ifs:
            print(f"- {if_['name']} ({if_['desc']})")
        print("\n")
        self.assertEqual(type(ifs), list)


    def test_startstop(self):
        pyrtb = Pyrtb.Pyrtb('../build/librtbessential2dctl.so')

        rc = pyrtb.Start(ETH_INTERFACE)
        self.assertEqual(rc, 0)

        slaves = pyrtb.Get_detected_slaves()
        self.assertEqual(len(slaves), 3)

        print("Interfaces:")
        for slave in slaves:
            print(f"- {slave['name']} (cfgAddr: {slave['configAdr']}, manId: {slave['manId']}, prodId: {slave['prodId']})")
        print("\n")

        print("Run for 3 seconds...");
        t = pyrtb.Get_simulation_time()
        while(t[0] < 3):
            time.sleep(0.1)
            t = pyrtb.Get_simulation_time()
        print("done");

        rc = pyrtb.Stop()
        self.assertEqual(rc, 0)
    
    def test_movement(self):
        pyrtb = Pyrtb.Pyrtb('../build/librtbessential2dctl.so')

        pyrtb.Start(ETH_INTERFACE)

        t = pyrtb.Get_simulation_time()
        while(t[0] < 10):
            
            time.sleep(0.1)
            t = pyrtb.Get_simulation_time()

        rc = pyrtb.Stop()


if __name__ == '__main__':
    unittest.main()