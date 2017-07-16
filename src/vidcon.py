#######################################################################
##
## Zed64 MetroComputer
##
## Unless a module otherwise marked,
## Copyright 2014, Michael T. Mayers (michael@tweakoz.com
## Provided under the Creative Commons Attribution License 3.0
## Please see https://creativecommons.org/licenses/by/3.0/us/legalcode
##
#######################################################################

import sys, os
from myhdl import *
from stdlib import  *

###########################################################

CELL_BASE = 0x8000
FONT_BASE = 0xc000

###########################################################

class VideoImpl:
    ################################################
    def __init__(self,
                 reset ):

        self.reset = reset

    ################################################
    def FrameController(    self,
                            pixel_clock,
                            hdisp, hsyncstart, hsyncend, htotal, hsyncinvert,
                            vdisp, vsyncstart, vsyncend, vtotal, vsyncinvert,
                            is_blanking,
                            hcount, vcount,
                            hpulse, vpulse,
                            hblank, vblank,
                            hwrap, vwrap ):

        ######################################
        # internal registers
        ######################################
        fcount = SigMod64()
        fpulse = SigBool()
        hcp1, hcp2 = Gen2(SigPixCnt)
        vcp1, vcp2 = Gen2(SigPixCnt)
        get_hc, get_vc = Gen2(SigPixCnt)
        hb, vb = Gen2(SigBool)
        ######################################
        @always_comb
        def comb_lev1():
            hcp1.next = hcount+1
            hcp2.next = hcount+2
            vcp1.next = vcount+1
            vcp2.next = vcount+2
        ######################################
        @always_comb
        def comb_lev2():
            hpulse.next = ((hcp2>=hsyncstart) and (hcp2<hsyncend)) ^ hsyncinvert
            vpulse.next = ((vcp2>=vsyncstart) and (vcp2<vsyncend)) ^ vsyncinvert
            hb.next = (hcount>=hdisp) and (hcount<=htotal)
            vb.next = (vcount>=vdisp) and (vcount<=vtotal)
            #bug vb should be low on last hcount of last vline
        ######################################
        @always_comb
        def comb_lev3a():
            hblank.next = hb
            vblank.next = vb
            is_blanking.next = hb | vb
        ######################################
        sel2a = Sel2(get_hc,hcp1,0,hwrap)
        sel2b = Sel2(get_vc,vcp1,0,vwrap)
        ######################################
        @always_seq(pixel_clock.negedge,self.reset)
        def hgen():
            hwrap.next = (hcp2==htotal)
            hcount.next = get_hc
        ######################################
        @always_seq(hwrap.negedge,self.reset)
        def vgen():
            vwrap.next = (vcp2==vtotal)
            vcount.next = get_vc
        ######################################
        return instances()
    ################################################
    def PixelGenerator( self,
                        pixel_clock, 
                        hdisp, hsyncstart, hsyncend, htotal, hsyncinvert,
                        vdisp, vsyncstart, vsyncend, vtotal, vsyncinvert,
                        is_blanking,
                        vram_adr_out, vram_dat_in, 
                        hblank, hcount, vcount,
                        red_out, grn_out, blu_out ):
        ######################################
        # internal registers
        ######################################
        charclock = SigBool()
        pix8 = SigByte()
        hlines = hdisp
        hsync_inv = hsyncinvert
        ######################################
        pixbit = Signal(modbv(0)[3:])
        @always_comb
        def comb_lev1():
            pixbit.next = concat(6-hcount[3:0])
            charclock.next = concat(hcount[1])
        ######################################
        current_pixel = SigBool()
        @always_comb
        def comb_lev2():
            current_pixel.next = pix8[pixbit]
            ############################
            # generate rgb
            ############################
            R = concat(hcount[8:4])
            G = concat(vcount[8:4])
            B = concat(hcount[10:8], vcount[10:8])
            red_out.next = R if pix8[pixbit] else 0
            grn_out.next = G if pix8[pixbit] else 0
            blu_out.next = B if pix8[pixbit] else 0
        ##############
        # charcell clock
        @always_seq(charclock.negedge,self.reset)
        def addrgen_charcell():
          vram_adr_out.next = concat(hcount[9:3],vcount[3:0])
        ##############
        @always_seq(charclock.posedge,self.reset)
        def fetch_pos():
            pix8.next = vram_dat_in
        ######################################
        return instances()
    ################################################
    def Top(    self, 
                pixel_clock,
                hdisp, hsyncstart, hsyncend, htotal, hsyncinvert,
                vdisp, vsyncstart, vsyncend, vtotal, vsyncinvert,
                vram_adr_out, vram_dat_in,
                rout, gout, bout, hout, vout ):
        ######################################
        # internal registers
        ######################################

        hcount, vcount = Gen2(SigPixCnt)
        hpulse, vpulse = Gen2(SigBool)
        hblank, vblank = Gen2(SigBool)
        hwrap, vwrap = Gen2(SigBool)
        is_blanking = SigBool()
        frame_counter,line_counter = Gen2(SigMod64)
        sel_r,sel_g,sel_b = Gen3(SigColorChannel)
        ######################################
        pix_r,pix_g,pix_b = Gen3(SigColorChannel)
        ######################################
        fc = self.FrameController(  
                                    pixel_clock,
                                    hdisp, hsyncstart, hsyncend, htotal, hsyncinvert,
                                    vdisp, vsyncstart, vsyncend, vtotal, vsyncinvert,
                                    is_blanking,
                                    hcount, vcount,
                                    hpulse, vpulse,
                                    hblank, vblank,
                                    hwrap, vwrap )
        ######################################
        pg = self.PixelGenerator(   
                                    pixel_clock,
                                    hdisp, hsyncstart, hsyncend, htotal, hsyncinvert,
                                    vdisp, vsyncstart, vsyncend, vtotal, vsyncinvert,
                                    is_blanking,
                                    vram_adr_out, vram_dat_in,
                                    hblank, hcount, vcount,
                                    pix_r,pix_g,pix_b )
        ######################################
        # enforce black pixels in blanking regions
        #  syncho won't work otherwise!
        ######################################
        sela = Sel2(sel_r,pix_r,0,is_blanking)
        selb = Sel2(sel_g,pix_g,0,is_blanking)
        selc = Sel2(sel_b,pix_b,0,is_blanking)

        @always_comb
        def color_out():
            rout.next = sel_r
            gout.next = sel_g
            bout.next = sel_b
            hout.next = hpulse
            vout.next = vpulse
        ######################################
        #@always_seq(vpulse.posedge,self.reset)
        #def frameinc():
        #    frame_counter.next = frame_counter+1
        ######################################
        #@always_seq(hpulse.posedge,self.reset)
        #def lineinc():
        #    line_counter.next = line_counter+1
        ######################################
        return instances()

################################################
def video(  reset,
            pixel_clock,
            hdisp, hsyncstart, hsyncend, htotal, hsyncinvert,
            vdisp, vsyncstart, vsyncend, vtotal, vsyncinvert,
            vram_adr_out,vram_dat_in,
            out_red,out_grn,out_blu,
            out_hs, out_vs ):

    v = VideoImpl(
            reset,
        )

    vtop = v.Top(   pixel_clock,
                    hdisp, hsyncstart, hsyncend, htotal, hsyncinvert,
                    vdisp, vsyncstart, vsyncend, vtotal, vsyncinvert,
                    vram_adr_out,vram_dat_in,
                    out_red,out_grn,out_blu, 
                    out_hs, out_vs )


    return instances()
################################################
def gen_verilog( name, vparams ):

    ios = vparams.ios
    outfolder = vparams.outfolder

    toVerilog.name = name
    toVerilog.timescale = vparams.timescale
    veri_inst = toVerilog(  video,
                            ios["reset"],
                            ios["pixel_clock"],
                            ios["hdisp"],
                            ios["hsyncstart"],
                            ios["hsyncend"],
                            ios["htotal"],
                            ios["hsyncinvert"],
                            ios["vdisp"],
                            ios["vsyncstart"],
                            ios["vsyncend"],
                            ios["vtotal"],
                            ios["vsyncinvert"],
                            ios["vram_adr_out"],
                            ios["vram_dat_in"],
                            ios["out_red"],
                            ios["out_grn"],
                            ios["out_blu"],
                            ios["out_hsync"],
                            ios["out_vsync"] )

    os.system("mv %s.v %s/" % (name,outfolder) )
    os.system("mv tb_%s.v %s/" % (name,outfolder) )

################################################
def gen_vhdl( ios ):
    # this does not work yet!
    # the vhdl generator
    # complains about the reset signal not having a 
    # name attribute !
    vhdl_inst = toVHDL( video,
                        ios["reset"],
                        ios["pixel_clock"],
                        ios["vram_adr_out"],
                        ios["vram_dat_in"],
                        ios["out_red"],
                        ios["out_grn"],
                        ios["out_blu"])
