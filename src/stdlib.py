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
import sys, os, md5, string
import localopts

hvcount_bits = 11

################################################33

def SigBool(def_val=False):
    return Signal(intbv(def_val)[1:])

def RegByte(def_val=0):
    return Signal(intbv(def_val)[8:])

def RegAdr16(def_val=0):
    return Signal(intbv(def_val)[16:])

def RegMod64(def_val=0):
    return modbv(def_val)[64:]

def pixdimval(value):
    return modbv(value)[hvcount_bits:]

def RegPixCnt(initval=0):
    return Signal(pixdimval(initval))

def Sel2(output,inpa,inpb,sel):
    @always_comb
    def logic():
        if sel:
            output.next = inpb
        else:
            output.next = inpa
    return logic

def SigColorChannel():
    return Signal(intbv(0)[4:])

class ModeLine:
    def __init__(   self,
                    hlines,hfp,hpulse,hsp,hsi,
                    vlines,vfp,vpulse,vsp,vsi ):
        self.h0 = hlines
        self.h1 = self.h0+hfp
        self.h2 = self.h1+hpulse
        self.hN = self.h2+hsp-4
        self.h3 = self.h2+hsp
        self.hsi = hsi
        self.v0 = vlines
        self.v1 = self.v0+vfp
        self.v2 = self.v1+vpulse
        self.v3 = self.v2+vsp
        self.vsi = vsi

################################################33

def Gen2( x ):
    return x(), x()
def Gen3( x ):
    return x(), x(), x()

################################################33

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

    def generate_verilog(self, out_folder, timescale,ios):
        toVerilog.timescale = timescale
        toVerilog.name = self.instance_name
        addr = Signal(intbv(0)[self.addrwidth:])
        data = Signal(intbv(0)[self.datawidth:])
        content = self.content
        #module = AsyncRomFile(Addr,Data,filename)
        veri_inst = toVerilog( self.gen, addr, data, content )
        os.system("mv %s.v %s/" % (self.instance_name,out_folder) )
        os.system("mv tb_%s.v %s/" % (self.instance_name,out_folder) )

################################################33

class UcfLine:
    def __init__(self,txt,newline=True):
        self.txt = txt
        self.newline = newline
    def gen(self):
        str = "%s" % self.txt
        if self.newline:
            str += ";\n"
        return str

class UcfNet:

    def __init__(self,name,loc,std,constraints=None):
        self.name = name
        self.loc = loc
        self.std = std
        self.constraints = constraints
        self.txt = None

    def gen(self):
        str = 'NET "%s" LOC="%s" | IOSTANDARD="%s";\n' % (self.name,self.loc,self.std)
        if self.constraints!=None:
            str += 'NET "%s" %s;\n' % (self.name,self.constraints)
        return str

class UcfGen:

    def __init__(self,ucf_name,def_std="LVCMOS33"):
        self.ucf_name = ucf_name
        self.nets = list()
        self.def_std = def_std
        self.addsep()
        self.addline("# UCF : %s autogenerated with tozedakit\n" % ucf_name, False)
        self.addsep()
    
    def addline(self,txt,newline=True):
        lin = UcfLine(txt,newline)
        self.nets.append(lin)

    def addnet(self,name,loc,std=None,constraints=None):
        if std==None:
            std=self.def_std
        net = UcfNet( name, loc, std, constraints )
        self.nets.append(net)

    def addnetarray(self,basename,locs,std=None):
        loclist = string.split(locs)
        index = 0
        for item in loclist:
            self.addnet( "%s<%d>"%(basename,index) ,item, std )
            index += 1

    def addsep(self):
        self.addline("##############################################\n",False)

    def emit(self,filename):
        fout = open(filename,"wt")
        for item in self.nets:
            fout.write("%s"%item.gen())
        fout.close()

################################################33

def pushdir(newdir):
        olddir = os.getcwd()
        os.chdir(newdir)
        return olddir

def popdir(a):
        os.chdir(a)

################################################33

states = enum( "Reset",
               "StateA",
               "StateB",
               "StateC",
               encoding="one_hot" )

def FSM(reset,clock):
    st = Signal(states.Reset)
    
    @always(clock.posedge)
    def gen():
        if reset:
            st.next = states.Reset
        else:
            if st==states.Reset:
                st.next = states.StateA
            elif st==states.StateA:
                st.next = states.StateC
            elif st==states.StateB:
                st.next = states.StateA
            elif st==states.StateC:
                st.next = states.StateB
            else:
                st.next = states.Reset

    return instances()

timescale = "1ns/1ps"

__all__ = [ 'timescale',
            'SigColorChannel',
            'SigBool',
            'RegMod64',
            'RegPixCnt',
            'RegByte',
            'RegAdr16',
            'Sel2',
            'ModeLine',
            'Gen2',
            'Gen3',
            'AsyncRomFile',
            'UcfGen',
            'pushdir','popdir']