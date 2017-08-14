#!/usr/bin/env python
import os
Z64ROOT=os.environ["Z64ROOT"]
NX4ROOT = Z64ROOT+"/zed64"
os.system("_z64_simhdlbuild.py -DDO_VPI -DDO_SIM")
os.chdir(Z64ROOT)
os.system("vvp -M%s nx4test.exe" % (NX4ROOT+"/vpi") )
