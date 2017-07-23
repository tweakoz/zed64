
#include "mos6502.h"
#include <assert.h>


#define GENINS(m,op) {#op,_am##m,&mos6502::Op_##op}

mos6502::mos6502(BusRead r, BusWrite w)
{
	_write = (BusWrite)w;
	_read = (BusRead)r;
	Instr instr;
		
    _amACC = {
        &mos6502::Addr_ACC,
        "ACC"
    };
    _amIMM = {
        &mos6502::Addr_IMM,
        "IMM"
    };
    _amABS = {
        &mos6502::Addr_ABS,
        "ABS"
    };
    _amZER = {
        &mos6502::Addr_ZER,
        "ZER"
    };
    _amZEX = {
        &mos6502::Addr_ZEX,
        "ZEX"
    };
    _amZEY = {
        &mos6502::Addr_ZEY,
        "ZEY"
    };
    _amABX = {
        &mos6502::Addr_ABX,
        "ABX"
    };
    _amABY = {
        &mos6502::Addr_ABY,
        "ABY"
    };
    _amIMP = {
        &mos6502::Addr_IMP,
        "IMP"
    };
    _amREL = {
        &mos6502::Addr_REL,
        "REL"
    };
    _amINX = {
        &mos6502::Addr_INX,
        "INX"
    };
    _amINY = {
        &mos6502::Addr_INY,
        "INY"
    };
    _amABI = {
        &mos6502::Addr_ABI,
        "ABI"
    };

    // fill jump table with ILLEGALs

    for(int i = 0; i < 256; i++)
        InstrTable[i] =  GENINS(IMP,ILLEGAL);

	// insert opcodes
	
	InstrTable[0x69] = GENINS(IMM,ADC);
	InstrTable[0x6D] = GENINS(ABS,ADC);
	InstrTable[0x65] = GENINS(ZER,ADC);
	InstrTable[0x61] = GENINS(INX,ADC);
	InstrTable[0x71] = GENINS(INY,ADC);
    InstrTable[0x72] = GENINS(ZER,ADC);
	InstrTable[0x75] = GENINS(ZEX,ADC);	
	InstrTable[0x7D] = GENINS(ABX,ADC);
	InstrTable[0x79] = GENINS(ABY,ADC);
	
	InstrTable[0x29] = GENINS(IMM,AND);
	InstrTable[0x2D] = GENINS(ABS,AND);
	InstrTable[0x25] = GENINS(ZER,AND);
	InstrTable[0x21] = GENINS(INX,AND);
	InstrTable[0x31] = GENINS(INY,AND);
	InstrTable[0x35] = GENINS(ZEX,AND);	
	InstrTable[0x3D] = GENINS(ABX,AND);
	InstrTable[0x39] = GENINS(ABY,AND);
	
	InstrTable[0x0E] = GENINS(ABS,ASL);
	InstrTable[0x06] = GENINS(ZER,ASL);
	InstrTable[0x0A] = GENINS(ACC,ASL_ACC);	
	InstrTable[0x16] = GENINS(ZEX,ASL);
	InstrTable[0x1E] = GENINS(ABX,ASL);
	
	InstrTable[0x90] = GENINS(REL,BCC);
	InstrTable[0xB0] = GENINS(REL,BCS);
	InstrTable[0xF0] = GENINS(REL,BEQ);
	
	InstrTable[0x2C] = GENINS(ABS,BIT);
	InstrTable[0x24] = GENINS(ZER,BIT);
	
	InstrTable[0x30] = GENINS(REL,BMI);
	InstrTable[0xD0] = GENINS(REL,BNE);
	InstrTable[0x10] = GENINS(REL,BPL);

	InstrTable[0x00] = GENINS(IMP,BRK);
	
	InstrTable[0x50] = GENINS(REL,BVC);
	InstrTable[0x70] = GENINS(REL,BVS);
	
	InstrTable[0x18] = GENINS(IMP,CLC);	
	InstrTable[0xD8] = GENINS(IMP,CLD);
	InstrTable[0x58] = GENINS(IMP,CLI);
	InstrTable[0xB8] = GENINS(IMP,CLV);
	
	InstrTable[0xC9] = GENINS(IMM,CMP);
	InstrTable[0xCD] = GENINS(ABS,CMP);
	InstrTable[0xC5] = GENINS(ZER,CMP);
	InstrTable[0xC1] = GENINS(INX,CMP);
	InstrTable[0xD1] = GENINS(INY,CMP);
	InstrTable[0xD5] = GENINS(ZEX,CMP);	
	InstrTable[0xDD] = GENINS(ABX,CMP);
	InstrTable[0xD9] = GENINS(ABY,CMP);
	
	InstrTable[0xE0] = GENINS(IMM,CPX);
	InstrTable[0xEC] = GENINS(ABS,CPX);
	InstrTable[0xE4] = GENINS(ZER,CPX);
	
	InstrTable[0xC0] = GENINS(IMM,CPY);
	InstrTable[0xCC] = GENINS(ABS,CPY);
	InstrTable[0xC4] = GENINS(ZER,CPY);
	
    InstrTable[0x3A] = GENINS(IMP,DEC);
	InstrTable[0xCE] = GENINS(ABS,DEC);
	InstrTable[0xC6] = GENINS(ZER,DEC);
	InstrTable[0xD6] = GENINS(ZEX,DEC);
	InstrTable[0xDE] = GENINS(ABX,DEC);
	
	InstrTable[0xCA] = GENINS(IMP,DEX);
	InstrTable[0x88] = GENINS(IMP,DEY);
	
	InstrTable[0x49] = GENINS(IMM,EOR);
	InstrTable[0x4D] = GENINS(ABS,EOR);
	InstrTable[0x45] = GENINS(ZER,EOR);
	InstrTable[0x41] = GENINS(INX,EOR);
	InstrTable[0x51] = GENINS(INY,EOR);
	InstrTable[0x55] = GENINS(ZEX,EOR);	
	InstrTable[0x5D] = GENINS(ABX,EOR);
	InstrTable[0x59] = GENINS(ABY,EOR);
	
    InstrTable[0x1A] = GENINS(IMP,INC);
	InstrTable[0xEE] = GENINS(ABS,INC);
	InstrTable[0xE6] = GENINS(ZER,INC);
	InstrTable[0xF6] = GENINS(ZEX,INC);
	InstrTable[0xFE] = GENINS(ABX,INC);
	
	InstrTable[0xE8] = GENINS(IMP,INX);
	InstrTable[0xC8] = GENINS(IMP,INY);
	
	InstrTable[0x4C] = GENINS(ABS,JMP);
	InstrTable[0x6C] = GENINS(ABI,JMP);

	InstrTable[0x20] = GENINS(ABS,JSR);
	
	InstrTable[0xA9] = GENINS(IMM,LDA);
	InstrTable[0xAD] = GENINS(ABS,LDA);
	InstrTable[0xA5] = GENINS(ZER,LDA);
	InstrTable[0xA1] = GENINS(INX,LDA);
	InstrTable[0xB1] = GENINS(INY,LDA);
    InstrTable[0xB2] = GENINS(ZER,LDA); //65c02 ZP
	InstrTable[0xB5] = GENINS(ZEX,LDA);	
	InstrTable[0xBD] = GENINS(ABX,LDA);
	InstrTable[0xB9] = GENINS(ABY,LDA);
	
	InstrTable[0xA2] = GENINS(IMM,LDX);
	InstrTable[0xAE] = GENINS(ABS,LDX);
	InstrTable[0xA6] = GENINS(ZER,LDX);
	InstrTable[0xBE] = GENINS(ABY,LDX);
	InstrTable[0xB6] = GENINS(ZEY,LDX);
	
	InstrTable[0xA0] = GENINS(IMM,LDY);
	InstrTable[0xAC] = GENINS(ABS,LDY);
	InstrTable[0xA4] = GENINS(ZER,LDY);
	InstrTable[0xB4] = GENINS(ZEX,LDY);
	InstrTable[0xBC] = GENINS(ABX,LDY);
	
	InstrTable[0x4E] = GENINS(ABS,LSR);
	InstrTable[0x46] = GENINS(ZER,LSR);
	InstrTable[0x4A] = GENINS(ACC,LSR_ACC);
	InstrTable[0x56] = GENINS(ZEX,LSR);
	InstrTable[0x5E] = GENINS(ABX,LSR);
	
	InstrTable[0xEA] = GENINS(IMP,NOP);
	
	InstrTable[0x09] = GENINS(IMM,ORA);
	InstrTable[0x0D] = GENINS(ABS,ORA);
	InstrTable[0x05] = GENINS(ZER,ORA);
	InstrTable[0x01] = GENINS(INX,ORA);
	InstrTable[0x11] = GENINS(INY,ORA);
	InstrTable[0x15] = GENINS(ZEX,ORA);	
	InstrTable[0x1D] = GENINS(ABX,ORA);
	InstrTable[0x19] = GENINS(ABY,ORA);
	
	InstrTable[0x48] = GENINS(IMP,PHA);
	InstrTable[0x08] = GENINS(IMP,PHP);
	InstrTable[0x68] = GENINS(IMP,PLA);
	InstrTable[0x28] = GENINS(IMP,PLP);
	
	InstrTable[0x2E] = GENINS(ABS,ROL);
	InstrTable[0x26] = GENINS(ZER,ROL);
	InstrTable[0x2A] = GENINS(ACC,ROL_ACC);
	InstrTable[0x36] = GENINS(ZEX,ROL);
	InstrTable[0x3E] = GENINS(ABX,ROL);
	
	InstrTable[0x6E] = GENINS(ABS,ROR);
	InstrTable[0x66] = GENINS(ZER,ROR);
	InstrTable[0x6A] = GENINS(ACC,ROR_ACC);
	InstrTable[0x76] = GENINS(ZEX,ROR);
	InstrTable[0x7E] = GENINS(ABX,ROR);
	
	InstrTable[0x40] = GENINS(IMP,RTI);
	InstrTable[0x60] = GENINS(IMP,RTS);
	
	InstrTable[0xE9] = GENINS(IMM,SBC);
	InstrTable[0xED] = GENINS(ABS,SBC);
	InstrTable[0xE5] = GENINS(ZER,SBC);
	InstrTable[0xE1] = GENINS(INX,SBC);
	InstrTable[0xF1] = GENINS(INY,SBC);
	InstrTable[0xF5] = GENINS(ZEX,SBC);
	InstrTable[0xFD] = GENINS(ABX,SBC);
	InstrTable[0xF9] = GENINS(ABY,SBC);
	
	InstrTable[0x38] = GENINS(IMP,SEC);
	InstrTable[0xF8] = GENINS(IMP,SED);
	InstrTable[0x78] = GENINS(IMP,SEI);
	
	InstrTable[0x8D] = GENINS(ABS,STA);
	InstrTable[0x85] = GENINS(ZER,STA);
	InstrTable[0x81] = GENINS(INX,STA);
	InstrTable[0x91] = GENINS(INY,STA);
	InstrTable[0x95] = GENINS(ZEX,STA);
	InstrTable[0x9D] = GENINS(ABX,STA);
	InstrTable[0x99] = GENINS(ABY,STA);
	
	InstrTable[0x8E] = GENINS(ABS,STX);
	InstrTable[0x86] = GENINS(ZER,STX);
	InstrTable[0x96] = GENINS(ZEY,STX);
	
	InstrTable[0x8C] = GENINS(ABS,STY);
	InstrTable[0x84] = GENINS(ZER,STY);
	InstrTable[0x94] = GENINS(ZEX,STY);
	
	InstrTable[0xAA] = GENINS(IMP,TAX);
	InstrTable[0xA8] = GENINS(IMP,TAY);
	InstrTable[0xBA] = GENINS(IMP,TSX);
	InstrTable[0x8A] = GENINS(IMP,TXA);
	InstrTable[0x9A] = GENINS(IMP,TXS);
	InstrTable[0x98] = GENINS(IMP,TYA);
	
	Reset();
	
	return;
}

uint16_t mos6502::Addr_ACC()
{
	return 0; // not used
}

uint16_t mos6502::Addr_IMM()
{
	return pc++;
}

uint16_t mos6502::Addr_ABS()
{
	uint16_t addrL;
	uint16_t addrH;
	uint16_t addr;
	
	addrL = Read(pc++);
	addrH = Read(pc++);
	
	addr = addrL + (addrH << 8);
		
	return addr;
}

uint16_t mos6502::Addr_ZER()
{
	return Read(pc++);
}

uint16_t mos6502::Addr_IMP()
{
	return 0; // not used
}

uint16_t mos6502::Addr_REL()
{
	uint16_t offset;
	uint16_t addr;
	
	offset = (uint16_t)Read(pc++);
    if (offset & 0x80) offset |= 0xFF00;	
    addr = pc + (int16_t)offset;
	return addr;
}

uint16_t mos6502::Addr_ABI()
{
	uint16_t addrL;
	uint16_t addrH;
	uint16_t effL;
	uint16_t effH;
	uint16_t abs;
	uint16_t addr;
	
	addrL = Read(pc++);
	addrH = Read(pc++);
	
	abs = (addrH << 8) | addrL;
	
	effL = Read(abs);
	effH = Read((abs & 0xFF00) + ((abs + 1) & 0x00FF) );
	
	addr = effL + 0x100 * effH;
	
	return addr;
}

uint16_t mos6502::Addr_ZEX()
{
	uint16_t addr = (Read(pc++) + X) % 256;
	return addr;
}

uint16_t mos6502::Addr_ZEY()
{
	uint16_t addr = (Read(pc++) + Y) % 256;
	return addr;
}

uint16_t mos6502::Addr_ABX()
{
	uint16_t addr;
	uint16_t addrL;
	uint16_t addrH;
	
	addrL = Read(pc++);
	addrH = Read(pc++);
	
	addr = addrL + (addrH << 8) + X;
	return addr;
}

uint16_t mos6502::Addr_ABY()
{
	uint16_t addr;
	uint16_t addrL;
	uint16_t addrH;
	
	addrL = Read(pc++);
	addrH = Read(pc++);
	
	addr = addrL + (addrH << 8) + Y;
	return addr;
}


uint16_t mos6502::Addr_INX()
{
	uint16_t zeroL;
	uint16_t zeroH;
	uint16_t addr;
	
	zeroL = (Read(pc++) + X) % 256;
	zeroH = (zeroL + 1) % 256;
	addr = Read(zeroL) + (Read(zeroH) << 8);
	
	return addr;
}

uint16_t mos6502::Addr_INY()
{
	uint16_t zeroL;
	uint16_t zeroH;
	uint16_t addr;
	
	zeroL = Read(pc++);
	zeroH = (zeroL + 1) % 256;
	addr = Read(zeroL) + (Read(zeroH) << 8) + Y;
	
	return addr;
}

void mos6502::Reset()
{
	A = 0x00;
	Y = 0x00;
	X = 0x00;
	
	pc = (Read(rstVectorH) << 8) + Read(rstVectorL); // load PC from reset vector
		
    sp = 0xFD;
    
    status |= CONSTANT;
    
	cycles = 6; // according to the datasheet, the reset routine takes 6 clock cycles

	illegalOpcode = false;
	
    printf( "%s%s", getAccString().c_str(),RESETX);

	return;
}

void mos6502::StackPush(uint8_t byte)
{	
	Write(0x0100 + sp, byte);
	if(sp == 0x00) sp = 0xFF;
	else sp--;
}

uint8_t mos6502::StackPop()
{
	if(sp == 0xFF) sp = 0x00;
	else sp++;
	auto popped = Read(0x0100 + sp);
	return popped;
}

void mos6502::IRQ()
{
	if(!IF_INTERRUPT())
	{
		SET_BREAK(0);
		StackPush((pc >> 8) & 0xFF);
		StackPush(pc & 0xFF);
		StackPush(status);
		SET_INTERRUPT(1);
		pc = (Read(irqVectorH) << 8) + Read(irqVectorL);
	}
}

void mos6502::NMI()
{
	SET_BREAK(0);
	StackPush((pc >> 8) & 0xFF);
	StackPush(pc & 0xFF);
	StackPush(status);
	SET_INTERRUPT(1);
	pc = (Read(nmiVectorH) << 8) + Read(nmiVectorL);
}

auto RED = rgb256(255,0,0);
auto GRN = rgb256(0,255,0);

uint8_t mos6502::Read(uint16_t addr)
{
    auto data = _read(addr);
    _accstring += Format( "%s(%04x:%02x)", GRN.c_str(),addr, data );
    return data;
}
void mos6502::Write(uint16_t addr,uint8_t data)
{
    _accstring += Format( "%s(%04x:%02x)", RED.c_str(), addr, data );
    _write(addr,data);
}

static std::string statusstring(uint8_t st)
{
    std::string rval = "";
    if(st&1)
        rval += "C";
    if(st&2)
        rval += "Z";
    if(st&4)
        rval += "I";
    if(st&8)
        rval += "D";
    if(st&16)
        rval += "B";
    if(st&64)
        rval += "V";
    if(st&128)
        rval += "S";
    return rval;
}

std::string mos6502::compstatline()
{
    static auto CW = rgb256(255,255,255);
    static auto CG = rgb256(128,128,128);
    static auto NCH = rgb256(128,128,160);
    static auto CH1 = rgb256(255,128,128);
    static auto CH2 = rgb256(255,128,255);
    static auto C1 = rgb256(128,128,224);
    static auto C2 = rgb256(255,100,100);
    static auto C3 = rgb256(0,160,160);
    static auto CGX= rgb256(128,128,128);

    auto PC = this->pc-1;
    auto SP = this->sp;
    auto X = this->X;
    auto Y = this->Y;
    auto A = this->A;
    auto ST = statusstring(this->status);

    static uint8_t LSP = SP;
    static auto LST = ST;
    static uint8_t LX = X;
    static uint8_t LY = Y;
    static uint8_t LA = A;

    bool chg_sp = LSP!=SP;
    bool chg_st = LST!=ST;
    bool chg_a = LA!=A;
    bool chg_x = LX!=X;
    bool chg_y = LY!=Y;


    LST = ST;
    LSP = SP;
    LA = A;
    LX = X;
    LY = Y;

    std::string statusline = "";

    statusline += CG+" pc:"+(_pcCHG?rgb256(255,255,0):CW)+Format( "%04X", PC );

    statusline += (chg_sp?CH1:NCH)+" sp:"+(chg_sp?C2:C1)+Format("%02X",SP);
    statusline += (chg_a?CH1:NCH)+" a:"+(chg_a?C2:C1)+Format("%02X",A);
    statusline += (chg_x?CH1:NCH)+" x:"+(chg_x?C2:C1)+Format("%02X",X);
    statusline += (chg_y?CH1:NCH)+" y:"+(chg_y?C2:C1)+Format("%02X",Y);
    statusline += (chg_st?CH1:NCH)+" st:"+(chg_st?C2:C1)+Format("[%s]",ST.c_str());

    int sizeline = statusline.length()-11*12;
    int space = 38-sizeline;
    statusline += "  ";

    return Format( "%-*s", (int)statusline.length()+space, statusline.c_str() );
}

void mos6502::Run(uint32_t n)
{
	uint32_t start = cycles;
	uint8_t opcode;

	while(start + n > cycles && !illegalOpcode)
	{
		// fetch
		opcode = Read(pc++);
		
		// decode
		_curins = InstrTable[opcode];
		
		// execute
		Exec();

		cycles++;
	}
}

auto EXECCOLOR = rgb256(255,255,128);

void mos6502::Exec()
{

    auto outline = compstatline();

    _pcCHG = false;
    _braNOCHG = false;

    //printf( "%s", status.c_str() );
    std::string insstr;
    auto insMODE = _curins.addr._name;
    if(insMODE == "IMP")
    {
        insstr = Format( "  %s%s          ", EXECCOLOR.c_str()
                                   , _curins._name.c_str() );

    }
    else if(insMODE=="REL")
    {        
        auto offset = (uint16_t)_read(pc);
        if (offset & 0x80) offset |= 0xFF00;    
        auto addr = pc + 1 + (int16_t)offset;

        insstr = Format( "  %s%s $%04X          ", EXECCOLOR.c_str()
                                       , _curins._name.c_str()
                                       , addr );
    }
    else if(insMODE=="ABS")
    {
        auto lo = _read(pc);
        auto hi = _read(pc+1);
        uint16_t addr = uint16_t(lo)|uint16_t(hi)<<8;

        insstr = Format( "  %s%s $%04X          ", EXECCOLOR.c_str()
                                       , _curins._name.c_str()
                                       , addr );
    }
    else if(insMODE=="ABX")
    {
        auto lo = _read(pc);
        auto hi = _read(pc+1);
        uint16_t addr = uint16_t(lo)|uint16_t(hi)<<8;

        insstr = Format( "  %s%s $%04X,x          ", EXECCOLOR.c_str()
                                       , _curins._name.c_str()
                                       , addr );
    }
    else if(insMODE=="ABY")
    {
        auto lo = _read(pc);
        auto hi = _read(pc+1);
        uint16_t addr = uint16_t(lo)|uint16_t(hi)<<8;

        insstr = Format( "  %s%s $%04X,y          ", EXECCOLOR.c_str()
                                       , _curins._name.c_str()
                                       , addr );
    }
    else if(insMODE=="INX") // Indexed Indirect (x)
    {
        insstr = Format( "  %s%s ($%02X),x            ", EXECCOLOR.c_str()
                                       , _curins._name.c_str()
                                       , _read(pc));
    }
    else if(insMODE=="INY") // Indirect Indexed (y)
    {
        insstr = Format( "  %s%s ($%02X),y            ", EXECCOLOR.c_str()
                                       , _curins._name.c_str()
                                       , _read(pc));
    }
    else if(insMODE=="IMM")
    {
        auto immval = _read(pc);

        insstr = Format( "  %s%s #$%02X          ", EXECCOLOR.c_str()
                                       , _curins._name.c_str()
                                       , immval );
    }
    else if(insMODE=="ZER")
    {
        auto zerval = _read(pc);

        insstr = Format( "  %s%s $%02X          ", EXECCOLOR.c_str()
                                       , _curins._name.c_str()
                                       , zerval );
    }
    else
    {
        insstr = Format( "  %s%s(%s)          ", EXECCOLOR.c_str()
                                               , _curins._name.c_str()
                                               , insMODE.c_str() );
    }
    int len = 24-(insstr.length()-12);
    outline += Format("%-*s", insstr.length()+len, insstr.c_str());

    auto addrmode = _curins.addr;
    auto addrcalc = addrmode.compute;
    assert(addrcalc!=nullptr);
	uint16_t src = (this->*addrcalc)();

    outline += getAccString();
	(this->*_curins.code)(src);
    auto nas = getAccString();
    if( nas.length() )
        outline += rgb256(255,255,0)+" | " + nas;

    std::string bgcolor = (cycles&1) ? rgb256bg(0,0,64)
                                     : rgb256bg(0,0,0);
    if( _pcCHG)
        bgcolor = rgb256bg(0,64,0);
    else if(_braNOCHG)
        bgcolor = rgb256bg(64,0,0);

    outline = bgcolor+outline;

	printf( "%s%s\n", outline.c_str(),RESETX);

    if( illegalOpcode )
    {
        printf( "\n(ILLEGAL INSTRUCTION) exiting...\n");
        exit(0);        
    }
}

std::string mos6502::getAccString()
{
    auto rval = _accstring;
    _accstring = "";
    return rval;
}


void mos6502::Op_ILLEGAL(uint16_t src)
{
	illegalOpcode = true;
}


void mos6502::Op_ADC(uint16_t src)
{
	uint8_t m = Read(src);
	unsigned int tmp = m + A + (IF_CARRY() ? 1 : 0);
	SET_ZERO(!(tmp & 0xFF));
    if (IF_DECIMAL())
    {
        if (((A & 0xF) + (m & 0xF) + (IF_CARRY() ? 1 : 0)) > 9) tmp += 6;
        SET_NEGATIVE(tmp & 0x80);
        SET_OVERFLOW(!((A ^ m) & 0x80) && ((A ^ tmp) & 0x80));
        if (tmp > 0x99)
        {
        	tmp += 96;
        }
        SET_CARRY(tmp > 0x99);
    }
	else
	{
		SET_NEGATIVE(tmp & 0x80);
		SET_OVERFLOW(!((A ^ m) & 0x80) && ((A ^ tmp) & 0x80));
		SET_CARRY(tmp > 0xFF);
    }
	
    A = tmp & 0xFF;
}



void mos6502::Op_AND(uint16_t src)
{
	uint8_t m = Read(src);
	uint8_t res = m & A;
	SET_NEGATIVE(res & 0x80);
	SET_ZERO(!res);
	A = res;
}


void mos6502::Op_ASL(uint16_t src)
{
	uint8_t m = Read(src);
    SET_CARRY(m & 0x80);
    m <<= 1;
    m &= 0xFF;
    SET_NEGATIVE(m & 0x80);
    SET_ZERO(!m);
    Write(src, m);
}

void mos6502::Op_ASL_ACC(uint16_t src)
{
	uint8_t m = A;
    SET_CARRY(m & 0x80);
    m <<= 1;
    m &= 0xFF;
    SET_NEGATIVE(m & 0x80);
    SET_ZERO(!m);
    A = m;
}

void mos6502::setPC(uint16_t npc)
{
    pc = npc;
    _pcCHG = true;
}

void mos6502::Op_BCC(uint16_t src)
{
    if (!IF_CARRY())
    	setPC(src);
    else
        _braNOCHG = true;
}


void mos6502::Op_BCS(uint16_t src)
{
    if (IF_CARRY())
        setPC(src);
    else
        _braNOCHG = true;
}

void mos6502::Op_BEQ(uint16_t src)
{
    if (IF_ZERO())
        setPC(src);
    else
        _braNOCHG = true;
}

void mos6502::Op_BIT(uint16_t src)
{
	uint8_t m = Read(src);
	uint8_t res = m & A;
	SET_NEGATIVE(res & 0x80);
	status = (status & 0x3F) | (uint8_t)(m & 0xC0);
	SET_ZERO(!res);
}

void mos6502::Op_BMI(uint16_t src)
{
    if (IF_NEGATIVE())
        setPC(src);
    else
        _braNOCHG = true;
}

void mos6502::Op_BNE(uint16_t src)
{
    if (!IF_ZERO())
        setPC(src);
    else
        _braNOCHG = true;
}

void mos6502::Op_BPL(uint16_t src)
{
    if (!IF_NEGATIVE())
        setPC(src);
    else
        _braNOCHG = true;
}

void mos6502::Op_BRK(uint16_t src)
{
	pc++;
	StackPush((pc >> 8) & 0xFF);
	StackPush(pc & 0xFF);
	StackPush(status | BREAK);
	SET_INTERRUPT(1);
    setPC((Read(irqVectorH) << 8) + Read(irqVectorL));
}

void mos6502::Op_BVC(uint16_t src)
{
    if (!IF_OVERFLOW())
        setPC(src);
    else
        _braNOCHG = true;
}

void mos6502::Op_BVS(uint16_t src)
{
    if (IF_OVERFLOW())
        setPC(src);
    else
        _braNOCHG = true;
}

void mos6502::Op_CLC(uint16_t src)
{
	SET_CARRY(0);
}

void mos6502::Op_CLD(uint16_t src)
{
	SET_DECIMAL(0);
}

void mos6502::Op_CLI(uint16_t src)
{
	SET_INTERRUPT(0);
}

void mos6502::Op_CLV(uint16_t src)
{
	SET_OVERFLOW(0);
}

void mos6502::Op_CMP(uint16_t src)
{
	unsigned int tmp = A - Read(src);
	SET_CARRY(tmp < 0x100);
	SET_NEGATIVE(tmp & 0x80);
	SET_ZERO(!(tmp & 0xFF));
}

void mos6502::Op_CPX(uint16_t src)
{
	unsigned int tmp = X - Read(src);
	SET_CARRY(tmp < 0x100);
	SET_NEGATIVE(tmp & 0x80);
	SET_ZERO(!(tmp & 0xFF));
}

void mos6502::Op_CPY(uint16_t src)
{
	unsigned int tmp = Y - Read(src);
	SET_CARRY(tmp < 0x100);
	SET_NEGATIVE(tmp & 0x80);
	SET_ZERO(!(tmp & 0xFF));
}

void mos6502::Op_DEC(uint16_t src)
{
    if( _curins.addr._name=="IMP" )
    {
        uint8_t m = A;
        m = (m - 1) % 256;
        SET_NEGATIVE(m & 0x80);
        SET_ZERO(!m);
        A = m;
    }
    else
    {
    	uint8_t m = Read(src);
    	m = (m - 1) % 256;
        SET_NEGATIVE(m & 0x80);
        SET_ZERO(!m);
        Write(src, m);
    }
}

void mos6502::Op_DEX(uint16_t src)
{
	uint8_t m = X;
	m = (m - 1) % 256;
	SET_NEGATIVE(m & 0x80);
    SET_ZERO(!m);
    X = m;
}

void mos6502::Op_DEY(uint16_t src)
{
	uint8_t m = Y;
	m = (m - 1) % 256;
	SET_NEGATIVE(m & 0x80);
    SET_ZERO(!m);
    Y = m;
}

void mos6502::Op_EOR(uint16_t src)
{
	uint8_t m = Read(src);
	m = A ^ m;
	SET_NEGATIVE(m & 0x80);
    SET_ZERO(!m);
    A = m;
}

void mos6502::Op_INC(uint16_t src)
{
    if( _curins.addr._name=="IMP" )
    {
        uint8_t m = A;
        m = (m + 1) % 256;
        SET_NEGATIVE(m & 0x80);
        SET_ZERO(!m);
        A=m;
    }
    else
    {
        uint8_t m = Read(src);
        m = (m + 1) % 256;
        SET_NEGATIVE(m & 0x80);
        SET_ZERO(!m);
        Write(src, m);
    }
}

void mos6502::Op_INX(uint16_t src)
{
	uint8_t m = X;
	m = (m + 1) % 256;
	SET_NEGATIVE(m & 0x80);
    SET_ZERO(!m);
    X = m;
}

void mos6502::Op_INY(uint16_t src)
{
	uint8_t m = Y;
	m = (m + 1) % 256;
	SET_NEGATIVE(m & 0x80);
    SET_ZERO(!m);
    Y = m;
}

void mos6502::Op_JMP(uint16_t src)
{
    setPC(src);
}

void mos6502::Op_JSR(uint16_t src)
{
	pc--;
	StackPush((pc >> 8) & 0xFF);
	StackPush(pc & 0xFF);
    setPC(src);
}

void mos6502::Op_LDA(uint16_t src)
{
	uint8_t m = Read(src);
	SET_NEGATIVE(m & 0x80);
    SET_ZERO(!m);
	A = m;
}

void mos6502::Op_LDX(uint16_t src)
{
	uint8_t m = Read(src);
	SET_NEGATIVE(m & 0x80);
    SET_ZERO(!m);
	X = m;
}

void mos6502::Op_LDY(uint16_t src)
{
	uint8_t m = Read(src);
	SET_NEGATIVE(m & 0x80);
    SET_ZERO(!m);
	Y = m;
}

void mos6502::Op_LSR(uint16_t src)
{
	uint8_t m = Read(src);
	SET_CARRY(m & 0x01);
	m >>= 1;
	SET_NEGATIVE(0);
	SET_ZERO(!m);
	Write(src, m);
}

void mos6502::Op_LSR_ACC(uint16_t src)
{
	uint8_t m = A;
	SET_CARRY(m & 0x01);
	m >>= 1;
	SET_NEGATIVE(0);
	SET_ZERO(!m);
	A = m;
}

void mos6502::Op_NOP(uint16_t src)
{
	return;
}

void mos6502::Op_ORA(uint16_t src)
{
	uint8_t m = Read(src);
	m = A | m;
	SET_NEGATIVE(m & 0x80);
    SET_ZERO(!m);
    A = m;
}

void mos6502::Op_PHA(uint16_t src)
{
	StackPush(A);
}

void mos6502::Op_PHP(uint16_t src)
{	
	StackPush(status | BREAK);
}

void mos6502::Op_PLA(uint16_t src)
{
	A = StackPop();
	SET_NEGATIVE(A & 0x80);
    SET_ZERO(!A);
}

void mos6502::Op_PLP(uint16_t src)
{
	status = StackPop();
	SET_CONSTANT(1);
}

void mos6502::Op_ROL(uint16_t src)
{
	uint16_t m = Read(src);
    m <<= 1;
    if (IF_CARRY()) m |= 0x01;
    SET_CARRY(m > 0xFF);
    m &= 0xFF;
    SET_NEGATIVE(m & 0x80);
    SET_ZERO(!m);
    Write(src, m);
}

void mos6502::Op_ROL_ACC(uint16_t src)
{
	uint16_t m = A;
    m <<= 1;
    if (IF_CARRY()) m |= 0x01;
    SET_CARRY(m > 0xFF);
    m &= 0xFF;
    SET_NEGATIVE(m & 0x80);
    SET_ZERO(!m);
    A = m;
}

void mos6502::Op_ROR(uint16_t src)
{
	uint16_t m = Read(src);
    if (IF_CARRY()) m |= 0x100;
    SET_CARRY(m & 0x01);
    m >>= 1;
    m &= 0xFF;
    SET_NEGATIVE(m & 0x80);
    SET_ZERO(!m);
    Write(src, m);
}

void mos6502::Op_ROR_ACC(uint16_t src)
{
	uint16_t m = A;
    if (IF_CARRY()) m |= 0x100;
    SET_CARRY(m & 0x01);
    m >>= 1;
    m &= 0xFF;
    SET_NEGATIVE(m & 0x80);
    SET_ZERO(!m);
    A = m;
}

void mos6502::Op_RTI(uint16_t src)
{
	uint8_t lo, hi;
	
	status = StackPop();
	
	lo = StackPop();
	hi = StackPop();
	
   setPC((hi << 8) | lo);
}

void mos6502::Op_RTS(uint16_t src)
{
	uint8_t lo, hi;
	
	lo = StackPop();
	hi = StackPop();
	
	setPC(((hi << 8) | lo) + 1);
}

void mos6502::Op_SBC(uint16_t src)
{
	uint8_t m = Read(src);
	unsigned int tmp = A - m - (IF_CARRY() ? 0 : 1);
	SET_NEGATIVE(tmp & 0x80);
	SET_ZERO(!(tmp & 0xFF));
    SET_OVERFLOW(((A ^ tmp) & 0x80) && ((A ^ m) & 0x80));
	
    if (IF_DECIMAL())
    {
    	if ( ((A & 0x0F) - (IF_CARRY() ? 0 : 1)) < (m & 0x0F)) tmp -= 6;
        if (tmp > 0x99)
        {
        	tmp -= 0x60;
        }
    }
    SET_CARRY(tmp < 0x100);
    A = (tmp & 0xFF);
}

void mos6502::Op_SEC(uint16_t src)
{
	SET_CARRY(1);
}

void mos6502::Op_SED(uint16_t src)
{
	SET_DECIMAL(1);
}

void mos6502::Op_SEI(uint16_t src)
{
	SET_INTERRUPT(1);
}

void mos6502::Op_STA(uint16_t src)
{
	Write(src, A);
}

void mos6502::Op_STX(uint16_t src)
{
	Write(src, X);
}

void mos6502::Op_STY(uint16_t src)
{
	Write(src, Y);
}

void mos6502::Op_TAX(uint16_t src)
{
	uint8_t m = A;
    SET_NEGATIVE(m & 0x80);
    SET_ZERO(!m);
	X = m;
}

void mos6502::Op_TAY(uint16_t src)
{
	uint8_t m = A;
    SET_NEGATIVE(m & 0x80);
    SET_ZERO(!m);
	Y = m;
}

void mos6502::Op_TSX(uint16_t src)
{
	uint8_t m = sp;
    SET_NEGATIVE(m & 0x80);
    SET_ZERO(!m);
	X = m;
}

void mos6502::Op_TXA(uint16_t src)
{
	uint8_t m = X;
    SET_NEGATIVE(m & 0x80);
    SET_ZERO(!m);
	A = m;
}

void mos6502::Op_TXS(uint16_t src)
{
	sp = X;
}

void mos6502::Op_TYA(uint16_t src)
{
	uint8_t m = Y;
    SET_NEGATIVE(m & 0x80);
    SET_ZERO(!m);
	A = m;
}

std::string Format( const char* formatstring, ... )
{
	std::string rval;
    char formatbuffer[512];
    va_list args;
    va_start(args, formatstring);
    vsnprintf( &formatbuffer[0], sizeof(formatbuffer), formatstring, args );
    va_end(args);
    rval = formatbuffer;
    return rval;
}

std::string rgb256(int r,int g,int b)
{ 
    int xr = int((r*5)/255);
    int xg = int((g*5)/255);
    int xb = int((b*5)/255);
    int color = 16 + 36 * xr + 6 * xg + xb;
    std::string rval = Format("\033[38;5;%03dm",color);
    return rval;
}
std::string rgb256bg(int r,int g,int b)
{ 
    int xr = int((r*5)/255);
    int xg = int((g*5)/255);
    int xb = int((b*5)/255);
    int color = 16 + 36 * xr + 6 * xg + xb;
    std::string rval = Format("\033[48;5;%03dm",color);
    return rval;
}
