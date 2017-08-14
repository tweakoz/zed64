#!/usr/bin/env python
import os, localopts,string,sys

opts = string.join(sys.argv[1:])

Z64ROOT=os.environ["Z64ROOT"]
NX4ROOT = Z64ROOT+"/zed64"

CXXFLAGS = "--std=c++11 -I /usr/local/include/iverilog -O3"
#    CXXFLAGS = "--std=c++11 -I /usr/include/iverilog -O3"
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
cmd += " zed64/rtl/6502_cpu.v"
cmd += " zed64/rtl/6502_alu.v"
cmd += " zed64/rtl/6502_fsm.v"
cmd += " zed64/rtl/6502_inc.v"
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
#os.system(cmd)