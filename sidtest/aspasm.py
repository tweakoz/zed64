#!/usr/bin/python

import os, sys, string, math, shlex, re

num_args = len(sys.argv)

if num_args != 2:
    print "usage aspasm.py <asmfile.asm>"
    print "   generates <asmfile.hex> and <asmfile.lst>"
    sys.exit(0)

asmfilename = sys.argv[1]
print "asmfilename<%s>" % asmfilename

if 0==os.path.exists(asmfilename):
    print "cannot open file<%s>" % asmfilename
    sys.exit(0)

inpfile = open(asmfilename)
inptext = inpfile.read()
inpfile.close()

####################################################

labels = dict()

####################################################
opcodes = {
'nop': 0x00,
'ld.mem': 0x10, # loadmem
'ld.memr': 0x11, # loadmemr (address in reg)
'ld.i18': 0x12, # loadimi (immed18)
'ld.l18': 0x13, # loadimi (lower18)
'ld.u18': 0x14, # loadimi (upper18)
'ld.var': 0x15, # loadvar
'ld.prod.ab.lo': 0x16, # load mul product lo (35..0)
'ld.prod.ab.hi': 0x17, # load mul product lo (71..36)
'ld.prod.cd.lo': 0x18, # load mul product lo (35..0)
'ld.prod.cd.hi': 0x19, # load mul product lo (71..36)
'ld.x36': 0xff, # load fixed(36) MACROINS
'ld.0': 0xff, # load zero
'st.mem': 0x20, # store
'st.io': 0x21, # store
'push': 0x31, # ..
'pop': 0x32, # ..
'add': 0x40, # add
'sub': 0x41, # sub
'and': 0x42, # and
'or' : 0x43, # or
'xor': 0x44, # xor
'not': 0x45, # negation
'mul.async': 0x50, # mul
'mulshr': 0x51, # mul and shiftr 
'div.async': 0x52, # div
'mac': 0x52, # mac
'shl': 0x60, # shiftL
'shr': 0x61, # shiftR
'lt': 0x70, # accum[0] = r0 < r1
'gt': 0x71, # : accum[1] = r0 > r1
'eq': 0x72, # accum[1] = r0 == r1
'call': 0xf0,
'ret': 0xf1,
'jmp' : 0xf2,
'jz': 0xf3,
'jnz': 0xf4,
'wait': 0xf5,
}
####################################################
regs = {
"r0":0,
"r1":1,
"r2":2,
"r3":3,
"r4":4,
"r5":5,
"r6":6,
"r7":7,
"acc":8,
"cc":9
}
####################################################
events={
    "sclock":7
}
####################################################
keywords={
    "$":0,
    "=":1,
    ":":2,
    "pc":3,
}
####################################################
re_float = r"[-]?(\d+(\.\d*))([eE][-+]?\d+)?"
rx_float = re.compile(re_float,re.VERBOSE)
re_decim = r"[-]?(\d+\d*)"
rx_decim = re.compile(re_decim,re.VERBOSE)
re_hexad = r"0[x](\d+\d*)"
rx_hexad = re.compile(re_hexad,re.VERBOSE)
re_ident = r"[a-zA-Z](\w*)"
rx_ident = re.compile(re_ident,re.VERBOSE)
####################################################
class ltok:
    def __init__(self,tok,typ,lineno):
        self.tok = tok
        self.typ = typ
        self.lineno = lineno
ltokens = list()
####################################################
print "lexed toks"
lexr = shlex.shlex(inptext)
lexr.commenters = ["#"]
lexr.wordchars += "."
lexr.whitespace += ";"
lexr.source = "import"
lineno = 0
for tok in lexr:
    is_float = rx_float.match(tok)
    is_decim = rx_decim.match(tok)
    is_hexad = rx_hexad.match(tok)
    is_ident = rx_ident.match(tok)
    if is_float:
        ltokens.append( ltok(tok,"float",lineno) )
    elif is_hexad:
        ltokens.append( ltok(tok,"hexad",lineno) )
    elif is_decim:
        ltokens.append( ltok(tok,"decim",lineno) )
    elif tok in opcodes:
        ltokens.append( ltok(tok,"opcode",lineno) )
    elif tok in regs:
        ltokens.append( ltok(tok,"reg",lineno) )
    elif tok in events:
        ltokens.append( ltok(tok,"event",lineno) )
    elif tok in keywords:
        ltokens.append( ltok(tok,"keyword",lineno) )
    elif is_ident:
        ltokens.append( ltok(tok,"ident",lineno) )
    else:
        ltokens.append( ltok(tok,"unknown",lineno) )
    lineno = lexr.lineno
####################################################
class output_item:
    def __init__(self):
        self.code = None
        self.byts = list()
        self.comment = None
        self.eval = None
        self.evalargs = None
        self.pc = None
    def comment_0arg(self,pc,opn):
        self.comment = "[$%02x] %s" % (pc,opn)
    def comment_1arg(self,pc,opn,arg):
        self.comment = "[$%02x] %s %s" % (pc, opn,arg)
    def comment_2arg(self,pc,opn,arg,arg2):
        self.comment = "[$%02x] %s %s %s" % (pc, opn,arg,arg2)
    def comment_3arg(self,pc,opn,arg,arg2,arg3):
        self.comment = "[$%02x] %s %s %s %s" % (pc, opn,arg,arg2,arg3)
    def addbyt(self,byt):
        self.byts.append(byt)
    def addint16(self,int16):
        masked = int16&0xffff
        self.addbyt((masked&0xff00)>>8)
        self.addbyt((masked&0x00ff))
    def addint24(self,int24):
        masked = int24&0xffffff
        self.addbyt((masked&0xff0000)>>16)
        self.addbyt((masked&0x00ff00)>>8)
        self.addbyt((masked&0x0000ff))
    def addopcint24(self,opc,int24):
        self.addbyt(opc)
        self.addint24(int24)

class output_stream:
    def __init__(self):
        self.out_items = list()
        self.pc = 0
    def additem(self,it,pcadd=1):
        it.pc = self.pc
        self.out_items.append(it)
        self.pc += pcadd

ostream = output_stream()

####################################################
def numeric_val(tok):
    if tok.typ == "decim":
        num = int(tok.tok)
    elif tok.typ == "hexad":
        num = int(tok.tok,16)
    elif tok.typ == "float":
        num = float(tok.tok)
    return num
####################################################
def encode_shift_amt(inp):
    num = int(inp)
    if num==1: return 0
    elif num==2: return 1
    #elif num==3: return 2
    elif num==4: return 3
    #elif num==5: return 4
    #elif num==6: return 5
    #elif num==7: return 6
    elif num==8: return 7
    #elif num==12: return 8
    elif num==16: return 9
    elif num==18: return 10
    elif num==24: return 11
    elif num==29: return 12
    else: return -1
####################################################
def coding_error( tok, errtxt ):
    print "coding error <%s> : tok<%s> line<%s>" % (errtxt, tok.tok, tok.lineno)
    sys.exit(0)
####################################################
def on_keyw(ltoks,idx):
    tok = ltoks[idx]
    return idx+1
def on_ident(ltoks,idx):
    tok = ltoks[idx]
    ntok = ltoks[idx+1]
    nidx = idx+1
    if ntok.tok==":":
        labels[tok.tok] = ostream.pc
        outi = output_item()
        outi.comment = "///////////////////////////////////////////////// [$%02x] LABEL<%s> " % (ostream.pc, tok.tok)
        ostream.additem(outi,0)
        nidx=idx+2
    return nidx
####################################################
def emit_opc(idx,opc):
    opcval = opcodes[opc.tok]
    opnam = opc.tok
    outi = output_item()
    outi.code = "{8'h%02x,24'h000000}" % (opcval)
    outi.comment_0arg(ostream.pc, opnam)
    outi.addopcint24(opcval,0)
    ostream.additem(outi)
    return idx+1
####################################################
def emit_rDST(idx,opc,arg):
    opcval = opcodes[opc.tok]
    opnam = opc.tok
    reg = regs[arg.tok]
    outi = output_item()
    outi.code = "{8'h%02x,24'h%1x00000}" % (opcval,reg)
    outi.comment_1arg(ostream.pc, opnam,arg.tok)
    outi.addopcint24(opcval,reg<<20)
    ostream.additem(outi)
    return idx+2
####################################################
def emit_rDST_rB(idx,opc,arg,arg2):
    opcval = opcodes[opc.tok]
    opnam = opc.tok
    adr = regs[arg.tok]
    outi = output_item()
    dat = regs[arg2.tok]
    outi.comment_2arg(ostream.pc, opnam,arg.tok,arg2.tok)
    outi.code = "{8'h%02x,24'h%1x%05x}" % (opcval,adr,dat)
    outi.addopcint24(opcval,adr<<20|dat)
    ostream.additem(outi)
    return idx+3
####################################################
def emit_rDST_num(idx,opc,arg,arg2):
    opcval = opcodes[opc.tok]
    opnam = opc.tok
    adr = regs[arg.tok]
    outi = output_item()
    num = numeric_val(arg2)
    outi.comment_2arg(ostream.pc, opnam,arg.tok,arg2.tok)
    outi.code = "{8'h%02x,24'h%1x%05x}" % (opcval,adr,num)
    outi.addopcint24(opcval,adr<<20|num)
    ostream.additem(outi)
    return idx+3
####################################################
def emit_rA_rB(idx,opc,arg,arg2):
    opcval = opcodes[opc.tok]
    opnam = opc.tok
    ra = regs[arg.tok]
    rb = regs[arg2.tok]
    outi = output_item()
    outi.code = "{8'h%02x,24'h0000%x%x}" % (opcval,ra,rb)
    outi.comment_2arg(ostream.pc, opnam, arg.tok, arg2.tok)
    outi.addopcint24(opcval,(ra<<4)|rb)
    ostream.additem(outi)
    return idx+3
####################################################
def emit_rDST_rA_rB(idx,opc,arg,arg2,arg3):
    opcval = opcodes[opc.tok]
    opnam = opc.tok
    ra = regs[arg.tok]
    rb = regs[arg2.tok]
    rc = regs[arg3.tok]
    outi = output_item()
    outi.code = "{8'h%02x,24'h%x000%x%x}" % (opcval,ra,rb,rc)
    outi.comment_3arg(ostream.pc, opnam, arg.tok, arg2.tok, arg3.tok)
    outi.addopcint24(opcval,(ra<<20)|(rb<<4)|rc)
    ostream.additem(outi)
    return idx+4
####################################################

def on_opcode(ltoks,idx):
    opc = ltoks[idx]
    if ((idx+1)<len(ltoks)):
        arg = ltoks[idx+1]
    else:
        arg = None
    if ((idx+2)<len(ltoks)):
        arg2 = ltoks[idx+2]
    else:
        arg2 = None
    if ((idx+3)<len(ltoks)):
        arg3 = ltoks[idx+3]
    else:
        arg3 = None
    opcval = opcodes[opc.tok]
    nidx = idx+1
    ################################
    # alu ops (implicit args - opa/opb)
    opnam = opc.tok
    if opnam =="nop":
        nidx = emit_opc(idx,opc)
    elif opnam in ["mul.async","div.async"]:
        if arg.typ == "reg" and arg2.typ=="reg":
            nidx = emit_rA_rB(idx,opc,arg,arg2)
    elif opnam in ["add","sub","mac","and","or","xor"]:
        if arg.typ == "reg" and arg2.typ=="reg" and arg3.typ=="reg":
            nidx = emit_rDST_rA_rB(idx,opc,arg,arg2,arg3)
    elif opnam in ["mulshr"]:
        shval = encode_shift_amt(int(arg.tok))
        if shval == -1:
            coding_error( arg, "unsupported shift value <%s>" % arg.tok )
        outi = output_item()
        outi.code = "{8'h%02x,24'h%06x}" % (opcval,shval)
        outi.comment_1arg(ostream.pc, opnam,arg.tok)
        outi.addopcint24(opcval,shval)
        ostream.additem(outi)
        nidx = idx+1
    ################################
    elif opnam=="ld.0":
        if (arg.typ == "reg"):
            syn_opc = opc
            syn_opc.tok="ld.var"
            nidx = emit_rDST(idx,syn_opc,arg)
    elif opnam in ["st.io"]:
        if (arg.typ == "reg") and (arg2.typ=="reg"):
            nidx = emit_rDST_rB(idx,opc,arg,arg2)
    elif opnam in ["ld.var","ld.i18","ld.u18","ld.l18","ld.x36", "st.mem", "ld.mem"]:
        if (arg.typ == "reg") and (arg2.typ in ["decim","hexad"]):
            nidx = emit_rDST_num(idx,opc,arg,arg2)
        elif opnam=="ld.x36" and arg.typ=="float":
            #
            l18 = opcodes["ld.l18"]
            u18 = opcodes["ld.u18"]
            #
            num = numeric_val(arg)
            frac,whol = math.modf(num)
            fx = frac*262144.0
            outi = output_item()
            outi.code = "{8'h%02x,6'h00,18'h%05x}" % (u18,int(whol))
            outi.comment = "[$%02x] %s %s (ld.u18 whol)" % (ostream.pc, opnam,arg.tok)
            outi.addopcint24(u18,int(whol)&0x3ffff)
            ostream.additem(outi)
            # fx
            outi = output_item()
            outi.code = "{8'h%02x,6'h00,18'h%05x}" % (l18,int(fx))
            outi.comment = "[$%02x] %s %s (ld.l18 frac)" % (ostream.pc, opnam,arg.tok)
            outi.addopcint24(l18,int(fx)&0x3ffff)
            ostream.additem(outi)
            nidx = idx+2
    ################################
    elif opnam in ["ld.prod.ab.lo","ld.prod.ab.hi","ld.prod.cd.lo","ld.prod.cd.hi"]:
        if arg.typ == "reg":
            nidx = emit_rDST(idx,opc,arg)
    ################################
    elif opnam in ["push","pop"]:
        if arg.typ == "reg":
            nidx = emit_rDST(idx,opc,arg)
    ################################
    elif opnam in ["ld.memr"]:
        if arg.typ == "reg":
            reg = regs[arg.tok]
            outi = output_item()
            outi.code = "{8'h%02x,24'h%06x}" % (opcval,reg)
            outi.comment_1arg(ostream.pc, opnam,arg.tok)
            outi.addopcint24(opcval,reg)
            ostream.additem(outi)
            nidx = idx+2
    ################################
    elif opnam in ["shl","shr"]:
        if arg.typ == "reg" and arg2.typ in ["decim","hexad"]:
            reg = regs[arg.tok]
            shval = encode_shift_amt(arg2.tok)
            if shval == -1:
                coding_error( arg, "unsupported shift value <%s>" % arg.tok )
            outi = output_item()
            outi.code = "{8'h%02x,24'h%1x%05x}" % (opcval,reg,shval)
            outi.comment_2arg(ostream.pc, opnam,arg.tok,arg2.tok)
            outi.addopcint24(opcval,(reg<<20)|shval)
            ostream.additem(outi)
            nidx = idx+2
    ################################
    elif (opnam in ["wait"]) and (arg.typ in ["decim","hexad"]):
        evnum = numeric_val(arg)
        outi = output_item()
        outi.code = "{8'h%02x,24'h%06x}" % (opcval,evnum)
        outi.comment_1arg(ostream.pc, opnam,arg.tok)
        outi.addopcint24(opcval,evnum)
        ostream.additem(outi)
        nidx = idx+2
    ################################
    elif opnam in ["ret"]:
        nidx = emit_opc(idx,opc)
    ################################
    elif opnam in ["jz", "jnz"]:
        if (arg.tok in regs.keys()) and (arg2.typ in ["decim","hexad","ident"]):
            reg = regs[arg.tok]
            loc = arg2.tok
            if arg2.typ=="ident":
                def l(ctx):
                    loc = labels[ctx["loc"]]
                    code = "{8'h%02x,24'h%x%05x}" % (opcval,reg,loc)
                    comment = "[$%02x] %s %s %s" % (ctx["pc"], opnam, arg.tok, arg2.tok)
                    return code,comment,opcval,(reg<<20)|loc
                ctx = dict()
                ctx["loc"] = loc
                ctx["pc"] = ostream.pc
                outi = output_item()
                outi.eval = l
                outi.evalargs=ctx
                ostream.additem(outi)
                nidx = idx+2
            else:
                nidx = emit_rDST_rB(idx,opc,arg,arg2)
    elif opnam in ["jmp","call"]:
        if arg.typ in ["decim","hexad","ident"]:
            loc = arg.tok
            if arg.typ=="ident":
                def l(ctx):
                    loc = labels[ctx["loc"]]
                    code = "{8'h%02x,24'h%06x}" % (opcval,loc)
                    comment = "[$%02x] %s %s" % (ctx["pc"], opnam, arg.tok)
                    return code,comment,opcval,loc
                ctx = dict()
                ctx["loc"] = loc
                ctx["pc"] = ostream.pc
                outi = output_item()
                outi.eval = l
                outi.evalargs=ctx
                ostream.additem(outi)
            else:
                outi = output_item()
                outi.code = "{8'h%02x,24'h%06x}" % (opcval,loc)
                outi.comment = "[$%02x] %s %s" % (ostream.pc, opnam, arg.tok)
                outi.addopcint24(opcval,loc)
                ostream.additem(outi)
            nidx = idx+2
    ################################
    else: # unknown
        nidx = idx
        print "unknown instruction<opc:%s arg:%s>" % (opnam,arg.tok)
        sys.exit(0)
    ################################
    return nidx
def on_reg(ltoks,idx):
    tok = ltoks[idx]
    return idx+1
def on_unk(ltoks,idx):
    tok = ltoks[idx]
    return idx+1
def on_float(ltoks,idx):
    tok = ltoks[idx]
    return idx+1
def on_hexad(ltoks,idx):
    tok = ltoks[idx]
    return idx+1
def on_decim(ltoks,idx):
    tok = ltoks[idx]
    return idx+1
####################################################
idx = 0
while idx < len(ltokens):
    tok = ltokens[idx]
    if tok.typ == "opcode":
        idx = on_opcode(ltokens,idx)
    if tok.typ == "keyword":
        idx = on_keyw(ltokens,idx)
    if tok.typ == "ident":
        idx = on_ident(ltokens,idx)
    if tok.typ == "reg":
        idx = on_reg(ltokens,idx)
    if tok.typ == "float":
        idx = on_float(ltokens,idx)
    if tok.typ == "hexad":
        idx = on_hexad(ltokens,idx)
    if tok.typ == "decim":
        idx = on_decim(ltokens,idx)
    if tok.typ == "unknown":
        idx = on_unk(ltokens,idx)
####################################################
hexfilename = asmfilename.replace(".asm",".hex")
lstfilename = asmfilename.replace(".asm",".lst")
hexfile = open(hexfilename,"wt")
lstfile = open(lstfilename,"wt")
####################################################
print "####################################################"
print "output"
for item in ostream.out_items:
    if item.code==None and item.eval!=None:
        code, comment,opcval,data24 = item.eval(item.evalargs)
        item.addopcint24(opcval,data24)
    else:
        code = item.code
        if code!=None:
            code = "%s;"%item.code
        comment = item.comment

    if code==None:
        print "%s" % (comment)
        lstfile.write("%s\n"%comment)
        hexfile.write("\n")
    else:
        #print = "dsp_rom[%s] = %s;" % (item.pc,code)
        codestr = ('dsp_rom[$%02x] = {0:<34} //{1:>8}'%int(item.pc)).format(*(code,comment))
        print codestr
        lstfile.write("%s\n"%codestr)
        bytsstr = ""
        for b in item.byts:
            bytsstr += "%02x" % b
        hexfile.write("%s\n"%bytsstr)

####################################################
hexfile.close()
lstfile.close()
