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

#!/usr/bin/python

from myhdl import *
from stdlib import *
import sys, os, md5, string
import localopts

################################################
# Dual port RAM with seperate in and out data lines
################################################

class DualSepPortRam:

    def __init__( self, instname, addrwidth, datawidth ):

        self.addrwidth = addrwidth
        self.datawidth = datawidth
        self.num_elems = 2**self.addrwidth        
        self.instance_name = instname
        print "RAM instname<%s>" % instname

        self.ramsig = [Signal(intbv(0)[self.datawidth:]) for ii in range(self.num_elems)]

        print "yo, that was an awfully slow method to create a memory block!"

    def gen(    self,
                a_clk, a_wena, a_addr, a_din, a_dout,
                b_clk, b_wena, b_addr, b_din, b_dout ):

        mem = self.ramsig

        #################3
        # port A
        #################3
        @always(a_clk.posedge)
        def a_write():
            if a_wena:
                mem[a_addr].next = a_din

        @always_comb
        def a_read():
            a_dout.next = mem[a_addr]

        #################3
        # port B
        #################3
        @always(b_clk.posedge)
        def b_write():
            if b_wena:
                mem[b_addr].next = b_din

        @always_comb
        def b_read():
            b_dout.next = mem[b_addr]

        return instances()

    def gen_verilog(self, vparams):
        toVerilog.timescale = vparams.timescale
        toVerilog.name = self.instance_name

        a_clk = SigBool()
        a_addr = SigWord(self.addrwidth)
        a_wena = SigBool()
        a_din = SigWord(self.datawidth)
        a_dout = SigWord(self.datawidth)
        b_clk = SigBool()
        b_addr = SigWord(self.addrwidth)
        b_din = SigWord(self.datawidth)
        b_dout = SigWord(self.datawidth)
        b_wena = SigBool()
        #module = AsyncRomFile(Addr,Data,filename)
        veri_inst = toVerilog(  self.gen, 
                                a_clk, a_wena, a_addr, a_din, a_dout,
                                b_clk, b_wena, b_addr, b_din, b_dout )

        outf = vparams.outfolder
        name = self.instance_name
        os.system("mv %s.v %s/" % (name,outf) )
        os.system("mv tb_%s.v %s/" % (name,outf) )

################################################
# Asynchronous, file initialized, ROM 
################################################

class AsyncRomFile:
    def __init__(self,instname,filename,addrwidth,datawidth):
        self.filename = filename
        self.file = open(filename,"r")
        self.file_data = self.file.read()
        self.size = len(self.file_data)
        self.m5 = md5.new()
        self.m5.update(self.file_data)
        self.chks = self.m5.hexdigest()
        self.addrwidth = addrwidth
        self.datawidth = datawidth
        self.instance_name = instname
        print "ROM fname<%s> len<%s> md5<%s>" % (filename,self.size,self.chks)

        max_size = 2**self.addrwidth        
        assert(self.size<=max_size)

        romsig = [Signal(intbv(0)[self.datawidth:]) for ii in range(max_size)]

        for i in range(max_size):
            romsig[i] = 0
        for i in range(self.size):
            romsig[i] = ord(self.file_data[i])

        self.content = tuple(romsig)

    def gen(self,addr,data,content):

        mcont = content
        @always_comb
        def read_rom():
            lookup = mcont[int(addr)]
            data.next = lookup

        return instances()

    def gen_verilog(self, vparams):
        out_folder = vparams.outfolder
        ios = vparams.ios
        toVerilog.timescale = vparams.timescale
        toVerilog.name = self.instance_name
        addr = Signal(intbv(0)[self.addrwidth:])
        data = Signal(intbv(0)[self.datawidth:])
        content = self.content
        #module = AsyncRomFile(Addr,Data,filename)
        veri_inst = toVerilog( self.gen, addr, data, content )
        os.system("mv %s.v %s/" % (self.instance_name,out_folder) )
        os.system("mv tb_%s.v %s/" % (self.instance_name,out_folder) )

###########################################
## Synchronous fifo
###########################################

class SynchronousFifo:

    def __init__(self,name,addrwidth,datawidth):
        self.name = name
        self.addrwidth = addrwidth
        self.nelem = 2**addrwidth
        self.datawidth = datawidth
        self.ramsig = [Signal(intbv(0)[self.datawidth:]) for ii in range(self.nelem)]

    def top(self,reset,clk,din,dout,rena,wena,full,empty):
        
        nelem = self.nelem
        addrw = self.addrwidth
        mem = self.ramsig
        count = SigByte()
        idxI = SigMod(addrw)
        idxO = SigMod(addrw)
        emp = SigBool()
        ful = SigBool()
        do_read = SigBool()
        do_write = SigBool()

        @always_comb
        def emp_ful():
            emp.next = (count==0)
            ful.next = (count==nelem)

        @always_comb
        def emp_ful2():
            do_read.next = (rena & (~emp))
            do_write.next = (wena & (~ful))

        @always_seq(clk.posedge,reset)
        def counter():
            if reset:
                count.next = 0
            elif do_read & ~ do_write:
                count.next = count-1
            elif do_write & ~ do_read:
                count.next = count+1

        @always_seq(clk.posedge,reset)
        def indices():
            if reset:
                idxI.next = 0
                idxO.next = 0
            else:
                if do_write:
                    idxO.next = (idxO+1)
                if do_read:
                    idxI.next = (idxI+1)

        @always_seq(clk.posedge,reset)
        def read():
            if reset:
                dout.next = 0
            elif do_read:
                dout.next = mem[idxI]

        @always_seq(clk.posedge,reset)
        def write():
            if do_write:
                mem[idxO].next = din



        return instances()



    def gen_verilog(self, vparams):
        out_folder = vparams.outfolder
        ios = vparams.ios
        toVerilog.timescale = vparams.timescale
        toVerilog.name = self.name
        veri_inst = toVerilog( 
                self.top, 
                SigReset(), # reset
                SigBool(),  # clk
                SigByte(),  # din
                SigByte(),  # dout
                SigBool(),  # rena
                SigBool(),  # wena
                SigBool(),  # full
                SigBool())  # empty
        os.system("mv %s.v %s/" % (self.name,out_folder) )
        os.system("mv tb_%s.v %s/" % (self.name,out_folder) )

__all__ = [ 'AsyncRomFile', "DualSepPortRam", "SynchronousFifo" ]
