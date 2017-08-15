//============================================================================
// Name             : mos6502 
// Author           : Gianluca Ghettini
//  Modifications   : Michael T. Mayers
// Version          : 1.0
// Copyright        : 
// Description      : A MOS 6502 CPU emulator written in C++
//============================================================================

#include <iostream>
#include <stdint.h>
#include <string>
#include <ork/types.h>
#include <ork/concurrent_queue.hpp>
#include <ork/stringutils.h>
#include <ork/cvector3.h>

using namespace std;
using namespace ork;

#define MOS65_NEGATIVE  0x80
#define MOS65_OVERFLOW  0x40
#define MOS65_CONSTANT  0x20
#define MOS65_BREAK     0x10
#define MOS65_DECIMAL   0x08
#define MOS65_INTERRUPT 0x04
#define MOS65_ZERO      0x02
#define MOS65_CARRY     0x01

#define SET_NEGATIVE(x) (x ? (status |= MOS65_NEGATIVE) : (status &= (~MOS65_NEGATIVE)) )
#define SET_OVERFLOW(x) (x ? (status |= MOS65_OVERFLOW) : (status &= (~MOS65_OVERFLOW)) )
#define SET_CONSTANT(x) (x ? (status |= MOS65_CONSTANT) : (status &= (~MOS65_CONSTANT)) )
#define SET_BREAK(x) (x ? (status |= MOS65_BREAK) : (status &= (~MOS65_BREAK)) )
#define SET_DECIMAL(x) (x ? (status |= MOS65_DECIMAL) : (status &= (~MOS65_DECIMAL)) )
#define SET_INTERRUPT(x) (x ? (status |= MOS65_INTERRUPT) : (status &= (~MOS65_INTERRUPT)) )
#define SET_ZERO(x) (x ? (status |= MOS65_ZERO) : (status &= (~MOS65_ZERO)) )
#define SET_CARRY(x) (x ? (status |= MOS65_CARRY) : (status &= (~MOS65_CARRY)) )

#define IF_NEGATIVE() ((status & MOS65_NEGATIVE) ? true : false)
#define IF_OVERFLOW() ((status & MOS65_OVERFLOW) ? true : false)
#define IF_CONSTANT() ((status & MOS65_CONSTANT) ? true : false)
#define IF_BREAK() ((status & MOS65_BREAK) ? true : false)
#define IF_DECIMAL() ((status & MOS65_DECIMAL) ? true : false)
#define IF_INTERRUPT() ((status & MOS65_INTERRUPT) ? true : false)
#define IF_ZERO() ((status & MOS65_ZERO) ? true : false)
#define IF_CARRY() ((status & MOS65_CARRY) ? true : false)

struct uicmd
{
    int _key, _mods;
};
struct memacc
{
    bool _write;
    uint8_t _val;
    uint16_t _addr;
};

typedef std::vector<memacc> accesslist;
struct insline
{
    int _pc, _lpc;
    bool _pcjmp;
    fvec3 _bgcolor;
    u8 _sp, _a, _x, _y, _st;
    u8 _lsp, _la, _lx, _ly, _lst;
    std::string _instext;
    accesslist _preacc;
    accesslist _postacc;
};

struct mos6502
{
    accesslist _memaccesses;

	// registers
	uint8_t A; // accumulator
	uint8_t X; // X-index
	uint8_t Y; // Y-index
	
	// stack pointer
	uint8_t sp;
	
	// program counter
	uint16_t pc;
	
	// status register
	uint8_t status;
	
	// consumed clock cycles 
	uint32_t cycles;
	
	typedef void (mos6502::*CodeExec)(uint16_t);
    typedef uint16_t (mos6502::*AddrExec)();

    struct AddrMode
    {
        AddrExec compute;
        std::string _name;
    };

	struct Instr
	{
        std::string _name;
		AddrMode addr;
		CodeExec code;
	};
	
	Instr InstrTable[256];
	
    Instr _curins;
	void Exec();
	
	bool illegalOpcode;
	
    AddrMode _amACC;
    AddrMode _amIMM;
    AddrMode _amABS;
    AddrMode _amZER;
    AddrMode _amZEX;
    AddrMode _amZEY;
    AddrMode _amABX;
    AddrMode _amABY;
    AddrMode _amIMP;
    AddrMode _amREL;
    AddrMode _amINX;
    AddrMode _amINY;
    AddrMode _amABI;

	// addressing modes
	uint16_t Addr_ACC(); // ACCUMULATOR
	uint16_t Addr_IMM(); // IMMEDIATE
	uint16_t Addr_ABS(); // ABSOLUTE
	uint16_t Addr_ZER(); // ZERO PAGE
	uint16_t Addr_ZEX(); // INDEXED-X ZERO PAGE
	uint16_t Addr_ZEY(); // INDEXED-Y ZERO PAGE
	uint16_t Addr_ABX(); // INDEXED-X ABSOLUTE
	uint16_t Addr_ABY(); // INDEXED-Y ABSOLUTE
	uint16_t Addr_IMP(); // IMPLIED
	uint16_t Addr_REL(); // RELATIVE
	uint16_t Addr_INX(); // INDEXED-X INDIRECT
	uint16_t Addr_INY(); // INDEXED-Y INDIRECT
	uint16_t Addr_ABI(); // ABSOLUTE INDIRECT
	
	// opcodes (grouped as per datasheet)
	void Op_ADC(uint16_t src);
	void Op_AND(uint16_t src);
	void Op_ASL(uint16_t src); 	void Op_ASL_ACC(uint16_t src);
	void Op_BCC(uint16_t src);
	void Op_BCS(uint16_t src);
	
	void Op_BEQ(uint16_t src);
	void Op_BIT(uint16_t src);
	void Op_BMI(uint16_t src);
	void Op_BNE(uint16_t src);
	void Op_BPL(uint16_t src);
	
	void Op_BRK(uint16_t src);
	void Op_BVC(uint16_t src);
	void Op_BVS(uint16_t src);
	void Op_CLC(uint16_t src);
	void Op_CLD(uint16_t src);
	
	void Op_CLI(uint16_t src);
	void Op_CLV(uint16_t src);
	void Op_CMP(uint16_t src);
	void Op_CPX(uint16_t src);
	void Op_CPY(uint16_t src);
	
	void Op_DEC(uint16_t src);
	void Op_DEX(uint16_t src);
	void Op_DEY(uint16_t src);
	void Op_EOR(uint16_t src);
	void Op_INC(uint16_t src);
	
	void Op_INX(uint16_t src);
	void Op_INY(uint16_t src);
	void Op_JMP(uint16_t src);
	void Op_JSR(uint16_t src);
	void Op_LDA(uint16_t src);
	
	void Op_LDX(uint16_t src);
	void Op_LDY(uint16_t src);
	void Op_LSR(uint16_t src); 	void Op_LSR_ACC(uint16_t src);
	void Op_NOP(uint16_t src);
	void Op_ORA(uint16_t src);
	
	void Op_PHA(uint16_t src);
	void Op_PHP(uint16_t src);
	void Op_PLA(uint16_t src);
	void Op_PLP(uint16_t src);
	void Op_ROL(uint16_t src); 	void Op_ROL_ACC(uint16_t src);
	
	void Op_ROR(uint16_t src);	void Op_ROR_ACC(uint16_t src);
	void Op_RTI(uint16_t src);
	void Op_RTS(uint16_t src);
	void Op_SBC(uint16_t src);
	void Op_SEC(uint16_t src);
	void Op_SED(uint16_t src);
	
	void Op_SEI(uint16_t src);
	void Op_STA(uint16_t src);
	void Op_STX(uint16_t src);
	void Op_STY(uint16_t src);
	void Op_TAX(uint16_t src);
	
	void Op_TAY(uint16_t src);
	void Op_TSX(uint16_t src);
	void Op_TXA(uint16_t src);
	void Op_TXS(uint16_t src);
	void Op_TYA(uint16_t src);
	
	void Op_ILLEGAL(uint16_t src);
	
	// IRQ, reset, NMI vectors
	static const uint16_t irqVectorH = 0xFFFF;
	static const uint16_t irqVectorL = 0xFFFE;
	static const uint16_t rstVectorH = 0xFFFD;
	static const uint16_t rstVectorL = 0xFFFC;
	static const uint16_t nmiVectorH = 0xFFFB;
	static const uint16_t nmiVectorL = 0xFFFA;
	
	// read/write callbacks
	typedef void (*BusWrite)(uint16_t, uint8_t);
	typedef uint8_t (*BusRead)(uint16_t);

    uint8_t Read(uint16_t addr);
    void Write(uint16_t addr,uint8_t data);

	BusRead _read;
	BusWrite _write;
	
	// stack operations
	inline void StackPush(uint8_t byte);
	inline uint8_t StackPop();
	
    std::string compstatline();

    bool _pcCHG;
    bool _braNOCHG;
    void setPC(uint16_t npc);
    uint64_t _inscount;

public:
	
	mos6502(BusRead r, BusWrite w);
	void NMI();
	void IRQ();
	void Reset();
	void Run(uint32_t n);
};

std::string Format( const char* formatstring, ... );
std::string rgb256(int r,int g,int b);
std::string rgb256bg(int r,int g,int b);

#define RESETX "\033[m"
