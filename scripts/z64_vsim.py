#!/usr/bin/env python
import os
Z64ROOT=os.environ["Z64ROOT"]
NX4ROOT = Z64ROOT+"/zed64"
os.system("_z64_simhdlbuild.py -DDO_DUMP -DDO_SIM")
os.system("vvp nx4test.exe" )
os.system("gtkwave n4.vcd n4.gtkw")
