#!/usr/bin/python

import sys, os, md5, string
from collections import OrderedDict
sys.path.append("./src/") 
sys.path.append("./scripts/") 
import localopts

from myhdl import *
from stdlib import *
from memory import *
from cpu import *

import vidcon
import modeline

Z64ROOT = os.environ["Z64ROOT"]
NX4ROOT = Z64ROOT+"/zed64"

################################################33
# output
################################################33

timescale = "1ns/1ns"

################################################33
# IO ports
################################################33

ios = dict()

ios["hdisp"] = SigWord(12)
ios["hsyncstart"] = SigWord(12)
ios["hsyncend"] = SigWord(12)
ios["htotal"] = SigWord(12)
ios["hsyncinvert"] = SigBool()
ios["vdisp"] = SigWord(12)
ios["vsyncstart"] = SigWord(12)
ios["vsyncend"] = SigWord(12)
ios["vtotal"] = SigWord(12)
ios["vsyncinvert"] = SigBool(12)

ios["pixel_clock"] = SigBool()
ios["reset"] =  ResetSignal(0, active=1, async=True)
ios["out_red"] = SigColorChannel()
ios["out_grn"] = SigColorChannel()
ios["out_blu"] = SigColorChannel()
ios["out_hsync"] = SigBool()
ios["out_vsync"] = SigBool()
ios["vram_adr_out"] = SigAdr16()
ios["vram_dat_in"] = SigByte()

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
u.addnet( "vgaHout", "K2" )
u.addnet( "pclkout", "H4", constraints='SLEW="FAST"' )
u.addnet( "rgbled1_b", "F6" )
u.addnet( "rgbled2_b", "L16" )
u.addnet( "pclkout", "H4", constraints='SLEW="FAST"' )
u.addsep()
u.addnet("sys_reset", "C12" )
u.addnet("but_center", "E16", constraints='CLOCK_DEDICATED_ROUTE="FALSE"' )
u.emit("zed64/rtl/nexys_gen.ucf")


########################################
def _synver():

    vparams = verilog_params("zed64/rtl/gen/",timescale,ios)

    os.chdir(Z64ROOT)

    modeline.gen_verilog(vparams)
    vidcon.gen_verilog("vidcon", vparams)

    chargen = AsyncRomFile("chargen","roms/chargen.bin",12,8)
    chargen.gen_verilog(vparams)


    #dpram = DualSepPortRam("dpram", 12, 8 )
    #dpram.gen_verilog(vparams)

    cpu = CPU02("cpu")
    cpu.gen_verilog(vparams)

    print("Generating Rom2")
    os.chdir(Z64ROOT+"/roms")
    os.system("g++ genrom2.cpp -o genrom2.exe")
    os.system("./genrom2.exe > rom2.bin")

    os.chdir(Z64ROOT)
    rom2 = AsyncRomFile("rom2","roms/rom2.bin",12,8)
    rom2.gen_verilog(vparams)

    print("Generating Rom2 complete..")

_synver()
