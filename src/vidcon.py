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

################################################33

mode_768p = ModeLine(   1024,24,136,160,True,
                        768,3,6,29,True )

mode_720p = ModeLine(   1280,110,40,220,False,
                        720,5,5,20,False )

mline = mode_768p

CELL_BASE = 0x8000
FONT_BASE = 0xc000

################################################33

class VideoImpl:
    ################################################
    def __init__(self,reset,pixel_clock):
        self.reset = reset
        self.pix_clk = pixel_clock
    ################################################
    def FrameController(    self,
                            hcount, vcount,
                            hpulse, vpulse,
                            hblank, vblank,
                            hwrap, vwrap,
                            is_blanking ):
        ######################################
        # internal registers
        ######################################
        fcount = SigMod64()
        fpulse = SigBool()
        hcp1, hcp2 = Gen2(SigPixCnt)
        vcp1, vcp2 = Gen2(SigPixCnt)
        get_hc, get_vc = Gen2(SigPixCnt)
        hb, vb = Gen2(SigBool)
        v0 = mline.v0
        h0 = mline.h0
        v1 = mline.v1
        h1 = mline.h1
        v2 = mline.v2
        h2 = mline.h2
        v3 = mline.v3
        h3 = mline.h3
        hsi = mline.hsi
        vsi = mline.vsi
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
            hpulse.next = ((hcp2>=h1) and (hcp2<h2)) ^ hsi
            vpulse.next = ((vcp2>=v1) and (vcp2<v2)) ^ vsi
            hb.next = (hcount>=h0) and (hcount<h3)
            vb.next = (vcount>=v0) and (vcount<v3)
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
        @always_seq(self.pix_clk.posedge,self.reset)
        def hgen():
            hwrap.next = (hcp2==h3)
            hcount.next = get_hc
        ######################################
        @always_seq(hwrap.negedge,self.reset)
        def vgen():
            vwrap.next = (vcp2==v3)
            vcount.next = get_vc
        ######################################
        return instances()
    ################################################
    def PixelGenerator( self, is_blanking,
                        vram_adr_out, vram_dat_in, 
                        hblank, hcount, vcount,
                        red_out, grn_out, blu_out ):
        ######################################
        # internal registers
        ######################################
        #gen_r, gen_g, gen_b = Gen3(SigColorChannel)
        pix8 = SigByte()
        dat_i = SigByte()
        hlines = mline.h0
        hsync_inv = mline.hsi
        ######################################
        xpos, ypos = Gen2(SigAdr16)
        xposp1, yposp1 = Gen2(SigAdr16)
        sela = Sel2(xpos,hcount,0,is_blanking)
        selb = Sel2(ypos,vcount,0,is_blanking)
        selc = Sel2(xposp1,hcount+1,0,is_blanking)
        seld = Sel2(yposp1,vcount+1,0,is_blanking)
        ######################################
        xcell, ycell = Gen2(SigByte)
        xcellp1, ycellp1 = Gen2(SigByte)
        hsync_fetch = SigBool()
        pixbit = Signal(modbv(0)[3:])
        @always_comb
        def comb_lev1():
            xcell.next = concat(xpos[11:3])
            ycell.next = concat(ypos[11:3])
            xcellp1.next = concat(xposp1[11:3])
            ycellp1.next = concat(yposp1[11:3])
            hsync_fetch.next = hblank
            pixbit.next = concat(6-hcount[3:0])
        ######################################
        xstate = Signal(modbv(0)[4:]);
        ycell_subrow = Signal(modbv(0)[3:])
        current_pixel = SigBool()
        @always_comb
        def comb_lev2():
            red_out.next = concat(hcount[8:4])
            grn_out.next = concat(vcount[8:4])
            blu_out.next = concat(hcount[10:6])
            xstate.next = concat(hsync_fetch,hcount[3:0])
            ycell_subrow.next = concat(ypos[3:0])
            current_pixel.next = pix8[pixbit]
        ######################################
        row_start = SigAdr16()
        @always_seq(hblank.posedge,self.reset)
        def hblankgen():
            row_start.next = CELL_BASE + xcellp1*0x100
        ######################################
        hsync_fetch_cycle = SigByte()
        @always_seq(self.pix_clk.posedge,self.reset)
        def addrgen0():
            if hsync_fetch:
                hsync_fetch_cycle.next = hsync_fetch_cycle+1
            else:
                hsync_fetch_cycle.next = 0
            dat_i.next = vram_dat_in
        ######################################
        charcode = SigByte()
        fontoffs = SigAdr16()
        ##############
        ## DDR fetching (use both pos/neg clock edges)
        ##############
        @always_seq(self.pix_clk.negedge,self.reset)
        def fetch_neg():
            if hsync_fetch_cycle==0:
                if xstate==0: # gen charcode addr
                    vram_adr_out.next = ypos*hlines+xpos
                ########################################
                elif xstate==1: # read char code, gen pixrow addr
                    vram_adr_out.next = FONT_BASE + fontoffs
        ##############
        @always_seq(self.pix_clk.posedge,self.reset)
        def fetch_pos():
            if hsync_fetch_cycle==0:
                ########################################
                if xstate==0: # gen charcode addr
                    charcode.next = vram_dat_in
                    # font addr
                    fontoffs.next = concat( intbv(0)[5:],  # 5 bits
                                            charcode.next, # 8 bits
                                            ycell_subrow ) # 3 bits
                ########################################
                elif xstate==1: # read pixrow
                    pix8.next = vram_dat_in
        ######################################
        return instances()
    ################################################
    def Top(    self,
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
        fc = self.FrameController(  hcount, vcount,
                                    hpulse, vpulse,
                                    hblank, vblank,
                                    hwrap, vwrap,
                                    is_blanking )
        ######################################
        pg = self.PixelGenerator(   is_blanking,
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
def video(  reset,pixel_clock,
            vram_adr_out,vram_dat_in,
            out_red,out_grn,out_blu,
            out_hs, out_vs ):

    v = VideoImpl(reset,pixel_clock)

    vtop = v.Top(   vram_adr_out,vram_dat_in,
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
                            ios["vram_adr_out"],
                            ios["vram_dat_in"],
                            ios["out_red"],
                            ios["out_grn"],
                            ios["out_blu"],
                            ios["out_hsync"],
                            ios["out_vsync"])

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
