#!/usr/bin/env python
import os
Z64ROOT=os.environ["Z64ROOT"]
os.chdir(Z64ROOT)
os.system("scons -f ork.sconstruct --site-dir ./ork.build/site_scons")
