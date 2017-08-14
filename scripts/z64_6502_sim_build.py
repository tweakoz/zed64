#!/usr/bin/env python
import os
Z64ROOT=os.environ["Z64ROOT"]
os.chdir(Z64ROOT+"/6502sim")
os.system( "make")
