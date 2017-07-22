#!/usr/bin/python

import sys, os, md5, string
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

def usage():
    print "usage make.py prep|synver|synvhd|ise|sidasm"
    sys.exit(0)

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

################################################33

def compile(opts=""):
#    CXXFLAGS = "--std=c++11 -I /usr/local/include/iverilog -O3"
    CXXFLAGS = "--std=c++11 -I /usr/include/iverilog -O3"
    os.chdir(NX4ROOT+"/vpi")
    os.system("clang++ %s vpi_zed.cpp -c -fPIC -o vpi_zed.o"%CXXFLAGS)
    os.system("iverilog-vpi -v -lOpenImageIO vpi_zed.o")
    #
    os.chdir(Z64ROOT)
    lfix = "xilinx_lib/"
    cmd = "iverilog "
    cmd += " -mvpi_zed" 
    cmd += " "+opts+" "
    cmd += " -y "+lfix+"simprims"
    cmd += " -y "+lfix+"unisims"
    cmd += " -y "+lfix+"XilinxCoreLib"
    cmd += " -o nx4test.exe"
    cmd += " "+lfix+"glbl.v"
    cmd += " zed64/rtl/xilinx/pll_hdtv.v"
    cmd += " zed64/rtl/xilinx/pll_vesa.v"
    cmd += " zed64/rtl/xilinx/dpram.v"
    cmd += " zed64/rtl/vidcon.v"
    cmd += " zed64/rtl/gen/chargen.v"
    cmd += " zed64/rtl/gen/rom2.v"
    cmd += " zed64/rtl/gen/modeline_hdisp.v"
    cmd += " zed64/rtl/gen/modeline_hstart.v"
    cmd += " zed64/rtl/gen/modeline_hend.v"
    cmd += " zed64/rtl/gen/modeline_htot.v"
    cmd += " zed64/rtl/gen/modeline_hsi.v"
    cmd += " zed64/rtl/gen/modeline_vdisp.v"
    cmd += " zed64/rtl/gen/modeline_vstart.v"
    cmd += " zed64/rtl/gen/modeline_vend.v"
    cmd += " zed64/rtl/gen/modeline_vtot.v"
    cmd += " zed64/rtl/gen/modeline_vsi.v"
    cmd += " zed64/rtl/gen/cpu.v"
    cmd += " zed64/rtl/tb_top_ise.v"
    cmd += " zed64/rtl/nexys4_top.v"
    print cmd
    os.system(cmd)

################################################33

if len(sys.argv)!=2:
    usage()

########################################
if sys.argv[1]=="synver":

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

########################################
elif sys.argv[1]=="synvhd":
    vidcon.gen_vhdl(ios)
########################################
elif sys.argv[1]=="cc65test":
    os.chdir(NX4ROOT+"/testprogs")
    os.system("make")
    os.system("hexdump -C ./gen/test1.bin > ./gen/test1.hex")
    os.system("ls -l ./gen")
    os.chdir(Z64ROOT+"/6502sim")
    os.system("make")
    os.system("./6502sim.exe ../zed64/testprogs/gen/test1.bin")
########################################
elif sys.argv[1]=="compile":
    os.chdir(Z64ROOT)
    compile("-Wall")
    #iverilog -s hello_pli -y. -mhello_vpi -o hello_pli.exe hello_pli.v 
    #vvp -M. ./hello_pli.exe
    sys.exit(0)
    os.chdir(Z64ROOT+"/vpitest")
    os.system("make")
########################################
elif sys.argv[1]=="sim":
    compile("-DDO_VPI -DDO_SIM")
    os.system("vvp -M%s nx4test.exe" % (NX4ROOT+"/vpi") )
########################################
elif sys.argv[1]=="simv":
    compile("-DDO_DUMP -DDO_SIM")
    os.system("vvp nx4test.exe" )
    os.system("gtkwave n4.vcd n4.gtkw")
########################################
elif sys.argv[1]=="ise":
    print localopts.ISE_BIN_DIR()
    os.system("%s/ise zed64/zed64.xise"%localopts.ISE_BIN_DIR())
########################################
elif sys.argv[1]=="sidasm":
    os.chdir("./sidtest")
    os.system( "./aspasm.py ./test.asm")
########################################
elif sys.argv[1]=="prep":
    os.system("rm -rf isedir")
    os.system("rm -rf isim")
    os.system("rm -rf zed64/rtl/gen")
    os.system("mkdir -p isedir/isim")
    os.system("mkdir -p zed64/rtl/gen/")
    os.system("wget http://www.zimmers.net/anonftp/pub/cbm/firmware/computers/c64/characters.901225-01.bin -O ./roms/chargen.bin")
    os.system("ln -s isedir/isim isim" )
    os.system("./make.py synver")
########################################
else:
    usage()


