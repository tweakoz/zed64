#!/usr/bin/env python
import os
Z64ROOT=os.environ["Z64ROOT"]
os.chdir(Z64ROOT)
os.system("./6502sim/6502sim.exe ./reasm/gen/test1.bin ")
