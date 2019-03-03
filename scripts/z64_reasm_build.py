#!/usr/bin/env python
import os
Z64ROOT=os.environ["Z64ROOT"]
os.chdir("%s/reasm" % Z64ROOT)
os.system("./reasm.py")
