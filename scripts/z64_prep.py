#!/usr/bin/env python
import os
Z64ROOT=os.environ["Z64ROOT"]
os.chdir(Z64ROOT)
os.system("rm -rf isedir")
os.system("rm -rf isim")
os.system("rm -rf zed64/rtl/gen")
os.system("mkdir -p isedir/isim")
os.system("mkdir -p zed64/rtl/gen/")
os.system("mkdir -p zed64/testprogs/gen/")
os.system("wget http://www.zimmers.net/anonftp/pub/cbm/firmware/computers/c64/characters.901225-01.bin -O ./roms/chargen.bin")
os.system("ln -s isedir/isim isim" )
os.system("z64_hdlgen.py")
