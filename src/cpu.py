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
from memory import *

import sys, os, md5, string
import localopts
import tozeda as eda

#######################################################################
# pcu        lsu        dec        alu         	
#
# rst        rst        rst        rst
# ifetch     ifetch     idle       idle
# ifetch     a     
	# fetchop
	# decode
	#           exec         fetchd          fetchaL 
	#           
	#
	#

#######################################################################

BOOT_ADDR = 0x0000

class eda_test(eda.module):

  def __init__(self,name):
	s = self
	eda.module.__init__(self,name)
	s.geninp("clk rst rena wena")
	s.genout("ful emp")
	s.geninp("din",8)
	s.genoutr("dout",8)
	s.verilog = (\
	  "assign out1 = inp1 & inp2;\n"
	  "always @(posedge clk)\n"
	  "  yo <= dude;\n"
	)
  def gen_verilog(self,vparams):
	print self.dump()
	print self.to_verilog()

#######################################################

cpu_states = enum(	"reset",
					"inita",
					"initb",
					"ifetch", "ifetched", "dfetch", "aftechL", "afetchH",
					"decoded", "agen",
					"execute",
					"halt" )

class CPU02:

	def __init__( self, instname ):
		self.instance_name = instname

	########################################
	# arithmetic logic unit
	########################################

	def alu( self, reset, clk, alu_op, a, b, dest ):

		#fi_din, fi_dout = Gen2(SigByte)
		#fi_ful, fi_emp = Gen2(SigBool)
		#fi_ren, fi_wen = Gen2(SigBool)
		#fi = SynchronousFifo("inp_fifo",3,32)
		#inp_fifo = fi.top(	reset,clk,
		#					fi_din, fi_dout,
		#					fi_ren, fi_wen,
		#					fi_ful, fi_emp )

		#fo_din, fo_dout = Gen2(SigByte)
		#fo_ful, fo_emp = Gen2(SigBool)
		#fo_ren, fo_wen = Gen2(SigBool)
		#fo = SynchronousFifo("out_fifo",3,16)
		#out_fifo = fo.top(	reset,clk,
		#					fo_din, fo_dout,
		#					fo_ren, fo_wen,
		#					fo_ful, fo_emp )
		enc_op = SigAdr16()


		@always_comb
		def encode():
			enc_op.next = concat(alu_op,a,b,dest)
			#fi_wen.next = (alu_op!=0)

		#@always_seq( clk.posedge, reset )
		#def input():
		#	if fi_wen.next:
		#		fi_din.next = enc_op
		#	else:
		#		fi_din.next = 0


		#@always_seq( clk.posedge, reset )
		#def compute():
		#	if alu_op==0:
		#		result = a+b
		#	else:
		#		result = a & b

		return instances()

	########################################
	# instruction decoder
	########################################

	def dec( self, reset, clk, opcode,
			 next_state, addrgen, r, rmw, w, inssize ):
		@always_comb
		def logic():
			if opcode==0:
				next_state.next = cpu_states.execute # cla
				inssize.next = 1
			elif opcode==1:
				next_state.next = cpu_states.dfetch
				inssize.next = 2
			elif opcode==2:
				next_state.next = cpu_states.aftechL
				inssize.next = 3
			addrgen.next = False
			r.next = True;
			rmw.next = False;
			w.next = False;
		return instances()

	########################################
	# program control unit
	########################################

	def pcu( self, reset, clk,
			 io_addr, io_wena, io_din, io_dout ):

		pc = SigAdr16()
		next_pc = SigAdr16()
		state = Signal(cpu_states.reset )
		next_state = Signal(cpu_states.reset )
		opcode = SigByte()
		addrgen = SigBool()
		will_read = SigBool()
		will_rmw = SigBool()
		will_write = SigBool()
		will_addrgen = SigBool()
		inssize = SigByte()

		#####################################

		DEC = self.dec( reset, clk, opcode, 
						next_state, addrgen, 
						will_read, will_rmw, will_write,
						inssize )

		#####################################

		@always_seq( clk.posedge, reset )
		def fsm():
			if reset:
				state.next = cpu_states.reset
			else:
				if state==cpu_states.reset:
					state.next = cpu_states.inita
				elif state==cpu_states.inita:
					state.next = cpu_states.initb
				elif state==cpu_states.initb:
					state.next = cpu_states.ifetch
				elif state==cpu_states.ifetch:
					io_addr.next = pc 
					state.next = cpu_states.ifetched
				elif state==cpu_states.ifetched:
					opcode.next = io_din
					state.next = cpu_states.decoded
				elif state==cpu_states.decoded:
					state.next = next_state
				#elif state==cpu_states.aftechL:


		#####################################

		@always_seq( clk.posedge, reset )
		def pc_update():
			if reset:
				pc.next = BOOT_ADDR
			else:
				pc.next = next_pc

		return instances()

########################################3

	def lsu(	self, reset, clk,
				io_addr, io_wena, io_din, io_dout,
				int_addr, int_wena, int_din, int_dout ):

		#fi_din, fi_dout = Gen2(SigByte)
		#fi_ful, fi_emp = Gen2(SigBool)
		#fi_ren, fi_wen = Gen2(SigBool)
		#fi = SynchronousFifo("inp_fifo",3,32)
		#inp_fifo = fi.top(	reset,clk,
		#					fi_din, fi_dout,
		#					fi_ren, fi_wen,
		#					fi_ful, fi_emp )
		return instances()

	########################################

	def top( self, reset, clk, 
				   io_addr, io_wena, io_din, io_dout ):

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

		PCU = self.pcu( reset, clk, io_addr, io_wena, io_din, io_dout )
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
					SigReset(), # reset
					SigBool(False),	# clock
					SigAdr16(0),	# addr
					SigBool(False),	# wena
					SigByte(0),	# din
					SigByte(0)	# dout
				 )

		os.system("mv %s.v %s/" % (name,out_folder) )
		os.system("mv tb_%s.v %s/" % (name,out_folder) )

__all__ = [ "CPU02","eda_test"]