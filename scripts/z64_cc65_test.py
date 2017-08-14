#!/usr/bin/env python
import os
Z64ROOT=os.environ["Z64ROOT"]
NX4ROOT = Z64ROOT+"/zed64"
os.chdir(NX4ROOT+"/testprogs")
os.system("make")
os.system("hexdump -C ./gen/test1.bin > ./gen/test1.hex")
os.system("ls -l ./gen")
os.chdir(Z64ROOT+"/6502sim")
os.system("make")
os.system("./6502sim.exe ../zed64/testprogs/gen/test1.bin")
