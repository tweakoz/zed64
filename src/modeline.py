import os, sys
from myhdl import *
from stdlib import *

###########################################################
# pixelclocks
###########################################################

#    pixel_clocks[6], // 148.5
#    pixel_clocks[5], // 74.25

#    pixel_clocks[4], // 108mhz
#    pixel_clocks[3], // 40 
#    pixel_clocks[2], // 36
#    pixel_clocks[1], // 27
#    pixel_clocks[0], // 13.5


###########################################################
# mode lines
###########################################################

#Modeline syntax: 
#dotclk(mhz) 
#hdisp hsyncstart hsyncend htotal 
#vdisp vsyncstart vsyncend vtotal 
#[flags]


###########################################################
# HDTV works on sony
###########################################################

#ModeLine "1920x1080" 148.50 1920 2008 2052 2200 1080 1084 1088 1125 -HSync -VSync
#ModeLine "ATSC-720-60p"     74.25 1280 1320 1376 1650 720 722 728 750

mode_1080p = (  "1080p", 6, # 148.5
                1920,2008,2052,2200,True,
                1080,1084,1088,1125,True )

mode_720p = (   "720p", 5, # 74.25
                1280,1320,1376,1650,False,
                720,722,728,750,False )

###########################################################
# VESA works on sony
###########################################################

#ModeLine "640x480 85hz" 36.00 640 696 752 832 480 481 484 509 -HSync -VSync
#ModeLine "720x480" 27.00 720 736 798 858 480 489 495 525 -HSync -VSync
#ModeLine "800x600" 40.00 800 840 968 1056 600 601 605 628 +HSync +VSync
#ModeLine "1280x1024" 108.00 1280 1328 1440 1688 1024 1025 1028 1066 +HSync +VSync


mode_320t = (   "320test", 2, # 36,
                 320,324,328,332,True,
                 240,242,244,248,True ) 
mode_vga85 = (   "vga85", 2, # 36,
                 640,696,752,832,True,
                 480,481,484,509,True ) 
mode_ntscp = (   "ntscp", 1, # 27
                 720,736,798,858,True,
                 480,489,495,525,True ) 
mmode_svga60 = ( "svga60", 3, # 40
                 800,840,968,1056,False,
                 600,601,605,628,False )
mode_1024p = (   "1024p", 4, # 108 
                 1280,1328,1440,1688,False,
                 1024,1025,1028,1066,False )

mode_test = (   "test", 4, # 108 
                 128,130,132,134,False,
                 128,136,144,152,False )

modelist = [ mode_720p, mode_1024p, mode_ntscp, mmode_svga60 ]
#modelist = [ mode_test, mode_test, mode_test, mode_test ]

###########################################################

mode_hsi,mode_hdisp,mode_hstart,mode_hend,mode_htot = [],[],[],[],[]
mode_vsi,mode_vdisp,mode_vstart,mode_vend,mode_vtot = [],[],[],[],[]

###########################################################

for item in modelist:
  name = item[0]
  clockid = item[1]

  mode_hdisp.append( item[2] )
  mode_hstart.append( item[3] )
  mode_hend.append( item[4] )
  mode_htot.append( item[5] )
  mode_hsi.append( item[6] )

  mode_vdisp.append( item[7] )
  mode_vstart.append( item[8] )
  mode_vend.append( item[9] )
  mode_vtot.append( item[10] )
  mode_vsi.append( item[11] )

###########################################################

def rom(dout, addr, contents):

    @always_comb
    def read():
        dout.next = contents[int(addr)]

    return read

###########################################################

def verout(vparams,name,obj,dwidth,awidth,contents):
	dout = Signal(intbv(0)[dwidth:])
	addr = Signal(intbv(0)[awidth:])
	ios = vparams.ios
	outfolder = vparams.outfolder
	toVerilog.name = name
	toVerilog.timescale = vparams.timescale
	veri_inst = toVerilog(rom, dout, addr, contents )
	os.system("mv %s.v %s/" % (name,outfolder) )
	os.system("mv tb_%s.v %s/" % (name,outfolder) )


###########################################################

def gen_verilog( vparams ):

	verout( vparams, "modeline_hsi", rom, 1, 2, tuple(mode_hsi) ) 
	verout( vparams, "modeline_hdisp", rom, 12, 2, tuple(mode_hdisp) ) 
	verout( vparams, "modeline_hstart", rom, 12, 2, tuple(mode_hstart) ) 
	verout( vparams, "modeline_hend", rom, 12, 2, tuple(mode_hend) ) 
	verout( vparams, "modeline_htot", rom, 12, 2, tuple(mode_htot) ) 

	verout( vparams, "modeline_vsi", rom, 1, 2, tuple(mode_vsi) ) 
	verout( vparams, "modeline_vdisp", rom, 12, 2, tuple(mode_vdisp) ) 
	verout( vparams, "modeline_vstart", rom, 12, 2, tuple(mode_vstart) ) 
	verout( vparams, "modeline_vend", rom, 12, 2, tuple(mode_vend) ) 
	verout( vparams, "modeline_vtot", rom, 12, 2, tuple(mode_vtot) ) 

###########################################################
