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

BOOT_ADDR = 0x0000

class CPU02:

    def __init__( self, instname ):
    	self.instance_name = instname

    ########################################
    # instruction decoder
    ########################################

    def dec( self, reset, clk, opcode, yo ):
    	@always_comb
    	def logic():
    		yo = opcode
    	return instances()

    ########################################
    # arithmetic logic unit
    ########################################

    def alu( self, reset, clk, oper, a, b, result ):

    	@always_seq( clk.posedge, reset )
    	def compute():
    		if oper==0:
    			result = a+b
    		else:
    			result = a & b

    	return instances()

    ########################################
    # load store unit
    ########################################

    def lsu( self, reset, clk, addr, wena, din, dout ):
    	@always_seq( clk.posedge, reset )
    	def logic():
			if reset:
				dout = 0
			else:
				dout = din

    ########################################
    # program control unit
    ########################################

    def pcu( self, reset, clk ):
    	pc = SigAdr16()
    	next_pc = SigAdr16()

    	@always_seq( clk.posedge, reset )
    	def pc_update():
    		if reset:
    			pc.next = BOOT_ADDR
    		else:
    			pc.next = next_pc

    	return instances()

    ########################################

    def top( self, reset, clk, 
    			   addr, wena, din, dout ):

		opcode = SigByte()
		alu_op = SigByte()
		opa = SigByte()
		opb = SigByte()
		result = SigByte()
		yo = SigByte()

		@always_seq( clk.posedge, reset )
		def logic():
			if reset:
				opcode = 0
				alu_op = 0
				opa = 0
				opb = 0
				result = 0				    	

		DEC = self.dec( reset, clk, opcode, yo )
		PCU = self.pcu( reset, clk )
		LSU = self.lsu( reset, clk,
						addr, wena, din, dout )
		ALU = self.alu( reset, clk,
						alu_op, opa, opb, result )
    	
		return instances()


    ########################################

    def gen_verilog( self, vparams ):
		out_folder = vparams.outfolder
		name = self.instance_name
		ios = vparams.ios
		toVerilog.timescale = vparams.timescale
		toVerilog.name = name

		toVerilog(	self.top,
					SigReset(),
					SigBool(),
					SigAdr16(),
					SigBool(),
					SigByte(),
					SigByte() )

		os.system("mv %s.v %s/" % (name,out_folder) )
		os.system("mv tb_%s.v %s/" % (name,out_folder) )

__all__ = [ "CPU02"]