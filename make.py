#!/usr/bin/python

import sys, os, md5
sys.path.append("./src/") 
sys.path.append("./scripts/") 
import localopts

from myhdl import *
from stdlib import *
import vidcon

################################################33
# output
################################################33

timescale = "1ns/1ps"

def usage():
    print "usage make.py prep|synver|synvhd|ise"
    sys.exit(0)

################################################33
# IO ports
################################################33

ios = dict()
ios["pixel_clock"] = SigBool()
ios["reset"] =  ResetSignal(0, active=1, async=True)
ios["out_red"] = SigColorChannel()
ios["out_grn"] = SigColorChannel()
ios["out_blu"] = SigColorChannel()
ios["out_hsync"] = SigBool()
ios["out_vsync"] = SigBool()
ios["vram_adr_out"] = RegAdr16()
ios["vram_dat_in"] = RegByte()

#for io_name in ios:
#    io = ios[io_name]
#    print "io:%s -> %s -> %s" % (io_name,io,dir(io))
#    ios[io_name].name = io_name

u = UcfGen("NEXYS4","LVCMOS33")
u.addnet("sys_clk", "E3", None, "TNM_NET = sys_clk_pin" )
u.addline("TIMESPEC TS_sys_clk_pin = PERIOD sys_clk_pin 100 MHz HIGH 50%")
u.addsep()
u.addnetarray("vgaR","A3 B4 C5 A4" )
u.addnetarray("vgaG","C6 A5 B6 A6" )
u.addnetarray("vgaB","B7 C7 D7 D8" )
u.addnet( "vgaH", "B11" )
u.addnet( "vgaV", "B12" )
u.addsep()
u.addnet("sys_reset", "C12" )
u.emit("./rtl/nexys.ucf")

################################################33

if len(sys.argv)!=2:
    usage()

########################################
if sys.argv[1]=="synver":
    vidcon.generate_verilog("vidcon", "rtl/",timescale,ios)
    chargen = AsyncRomFile("chargen","roms/chargen.bin",12,8)
    chargen.generate_verilog("rtl/",timescale,ios)
########################################
elif sys.argv[1]=="synvhd":
    vidcon.generate_vhdl(ios)
########################################
elif sys.argv[1]=="ise":
    print localopts.ISE_BIN_DIR()
    os.system("%s/ise zed64.xise"%localopts.ISE_BIN_DIR())
########################################
elif sys.argv[1]=="prep":
    os.system("rm -rf isedir")
    os.system("rm -rf isim")
    os.system("mkdir -p isedir/isim")
    os.system("mkdir -p roms/")
    os.system("wget http://www.zimmers.net/anonftp/pub/cbm/firmware/computers/c64/characters.901225-01.bin -O ./roms/chargen.bin")
    os.system("ln -s isedir/isim isim" )
    os.system("./make.py synver")
########################################
else:
    usage()


