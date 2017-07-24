/*
 * verilog model of 6502 CPU.
 *
 * (C) Arlet Ottens, <arlet@c-scape.nl>
 *
 * Feel free to use this code in any project (commercial or not), as long as you
 * keep this message, and the copyright notice. This code is provided "as is", 
 * without any warranties of any kind. 
 * 
 */

/*
 * Note that not all 6502 interface signals are supported (yet).  The goal
 * is to create an Acorn Atom model, and the Atom didn't use all signals on
 * the main board.
 *
 * The data bus is implemented as separate read/write buses. Combine them
 * on the output pads if external memory is required.
 */

module cpu6502( clk, reset, AB, DI, DO, WE, IRQ, NMI, RDY );

input clk;              // CPU clock 
input reset;            // reset signal
output reg [15:0] AB;   // address bus
input [7:0] DI;         // data in, read bus
output [7:0] DO;        // data out, write bus
output WE;              // write enable
input IRQ;              // interrupt request
input NMI;              // non-maskable interrupt request
input RDY;              // Ready signal. Pauses CPU when RDY=0 

cpu6502_inc FSMSTATE();

/*
 * internal signals
 */

reg  [15:0] PC;         // Program Counter 
reg  [7:0] ABL;         // Address Bus Register LSB
reg  [7:0] ABH;         // Address Bus Register MSB
wire [7:0] ADD;         // Adder Hold Register (registered in ALU)

reg  [7:0] DIHOLD;      // Hold for Data In
reg  DIHOLD_valid;      //
wire [7:0] DIMUX;       //

reg  [7:0] IRHOLD;      // Hold for Instruction register 
reg  IRHOLD_valid;      // Valid instruction in IRHOLD

reg  [7:0] AXYS[3:0];   // A, X, Y and S register file

reg  C = 0;             // carry flag (init at zero to avoid X's in ALU sim)
reg  Z = 0;             // zero flag
reg  I = 0;             // interrupt flag
reg  D = 0;             // decimal flag
reg  V = 0;             // overflow flag
reg  N = 0;             // negative flag
wire AZ;                // ALU Zero flag
wire AV;                // ALU overflow flag
wire AN;                // ALU negative flag
wire HC;                // ALU half carry

reg  [7:0] AI;          // ALU Input A
reg  [7:0] BI;          // ALU Input B
wire [7:0] DI;          // Data In
wire [7:0] IR;          // Instruction register
reg  [7:0] DO;          // Data Out 
reg  WE;                // Write Enable
reg  CI;                // Carry In
wire CO;                // Carry Out 
wire [7:0] PCH = PC[15:8];
wire [7:0] PCL = PC[7:0];

reg NMI_edge = 0;       // captured NMI edge

reg [1:0] regsel;                       // Select A, X, Y or S register
wire [7:0] regfile = AXYS[regsel];      // Selected register output

parameter 
        SEL_A    = 2'd0,
        SEL_S    = 2'd1,
        SEL_X    = 2'd2, 
        SEL_Y    = 2'd3;

/*
 * define some signals for watching in simulator output
 */


`ifdef DO_SIM
wire [7:0]   A = AXYS[SEL_A];           // Accumulator
wire [7:0]   X = AXYS[SEL_X];           // X register
wire [7:0]   Y = AXYS[SEL_Y];           // Y register 
wire [7:0]   S = AXYS[SEL_S];           // Stack pointer 
`endif

wire [7:0] P = { N, V, 2'b11, D, I, Z, C };

/*
 * instruction decoder/sequencer
 */

/*
 * control signals
 */

reg PC_inc;             // Increment PC
reg [15:0] PC_temp;     // intermediate value of PC 

reg [1:0] src_reg;      // source register index
reg [1:0] dst_reg;      // destination register index

reg index_y;            // if set, then Y is index reg rather than X 
reg load_reg;           // loading a register (A, X, Y, S) in this instruction
reg inc;                // increment
reg write_back;         // set if memory is read/modified/written 
reg load_only;          // LDA/LDX/LDY instruction
reg store;              // doing store (STA/STX/STY)
reg adc_sbc;            // doing ADC/SBC
reg compare;            // doing CMP/CPY/CPX
reg shift;              // doing shift/rotate instruction
reg rotate;             // doing rotate (no shift)
reg backwards;          // backwards branch
reg cond_true;          // branch condition is true
reg [2:0] cond_code;    // condition code bits from instruction
reg shift_right;        // Instruction ALU shift/rotate right 
reg alu_shift_right;    // Current cycle shift right enable
reg [3:0] op;           // Main ALU operation for instruction
reg [3:0] alu_op;       // Current cycle ALU operation 
reg adc_bcd;            // ALU should do BCD style carry 
reg adj_bcd;            // results should be BCD adjusted

wire [5:0] state;
wire [8*6-1:0] statename;

cpu6502_fsm fsm(clk,reset,
                IR,RDY,
                write_back,CO,store,
                cond_true, backwards,
                state, statename );

/* 
 * some flip flops to remember we're doing special instructions. These
 * get loaded at the DECODE state, and used later
 */
reg bit_ins;            // doing BIT instruction
reg plp;                // doing PLP instruction
reg php;                // doing PHP instruction 
reg clc;                // clear carry
reg sec;                // set carry
reg cld;                // clear decimal
reg sed;                // set decimal
reg cli;                // clear interrupt
reg sei;                // set interrupt
reg clv;                // clear overflow 
reg brk;                // doing BRK

reg res;                // in reset

/*
 * ALU operations
 */

parameter
        OP_OR  = 4'b1100,
        OP_AND = 4'b1101,
        OP_EOR = 4'b1110,
        OP_ADD = 4'b0011,
        OP_SUB = 4'b0111,
        OP_ROL = 4'b1011,
        OP_A   = 4'b1111;

/*
 * Program Counter Increment/Load. First calculate the base value in
 * PC_temp.
 */
always @*
    case( state )
        FSMSTATE.DECODE:         if( (~I & IRQ) | NMI_edge )
                            PC_temp = { ABH, ABL };
                        else
                            PC_temp = PC;


        FSMSTATE.JMP1,
        FSMSTATE.JMPI1,
        FSMSTATE.JSR3,
        FSMSTATE.RTS3,           
        FSMSTATE.RTI4:           PC_temp = { DIMUX, ADD };
                        
        FSMSTATE.BRA1:           PC_temp = { ABH, ADD };

        FSMSTATE.BRA2:           PC_temp = { ADD, PCL };

        FSMSTATE.BRK2:           PC_temp =      res ? 16'hfffc : 
                                  NMI_edge ? 16'hfffa : 16'hfffe;

        default:        PC_temp = PC;
    endcase

/*
 * Determine wether we need PC_temp, or PC_temp + 1
 */
always @*
    case( state )
        FSMSTATE.DECODE:         if( (~I & IRQ) | NMI_edge )
                            PC_inc = 0;
                        else
                            PC_inc = 1;

        FSMSTATE.ABS0,
        FSMSTATE.ABSX0,
        FSMSTATE.FETCH,
        FSMSTATE.BRA0,
        FSMSTATE.BRA2,
        FSMSTATE.BRK3,
        FSMSTATE.JMPI1,
        FSMSTATE.JMP1,
        FSMSTATE.RTI4,
        FSMSTATE.RTS3:           PC_inc = 1;

        FSMSTATE.BRA1:           PC_inc = CO ^~ backwards;

        default:        PC_inc = 0;
    endcase

/* 
 * Set new PC
 */
always @(posedge clk) 
    if( RDY )
        PC <= PC_temp + PC_inc;

/*
 * Address Generator 
 */

parameter
        ZEROPAGE  = 8'h00,
        STACKPAGE = 8'h01;

always @*
    case( state )
        FSMSTATE.ABSX1,
        FSMSTATE.INDX3,
        FSMSTATE.INDY2,
        FSMSTATE.JMP1,
        FSMSTATE.JMPI1,
        FSMSTATE.RTI4,
        FSMSTATE.ABS1:           AB = { DIMUX, ADD };

        FSMSTATE.BRA2,
        FSMSTATE.INDY3,
        FSMSTATE.ABSX2:          AB = { ADD, ABL };

        FSMSTATE.BRA1:           AB = { ABH, ADD };

        FSMSTATE.JSR0,
        FSMSTATE.PUSH1,
        FSMSTATE.RTS0,
        FSMSTATE.RTI0,
        FSMSTATE.BRK0:           AB = { STACKPAGE, regfile };

        FSMSTATE.BRK1,
        FSMSTATE.JSR1,
        FSMSTATE.PULL1,
        FSMSTATE.RTS1,
        FSMSTATE.RTS2,
        FSMSTATE.RTI1,
        FSMSTATE.RTI2,
        FSMSTATE.RTI3,
        FSMSTATE.BRK2:           AB = { STACKPAGE, ADD };
        
        FSMSTATE.INDY1,
        FSMSTATE.INDX1,
        FSMSTATE.ZPX1,
        FSMSTATE.INDX2:          AB = { ZEROPAGE, ADD };

        FSMSTATE.ZP0,
        FSMSTATE.INDY0:          AB = { ZEROPAGE, DIMUX };

        FSMSTATE.REG,
        FSMSTATE.READ,
        FSMSTATE.WRITE:          AB = { ABH, ABL };

        default:        AB = PC;
    endcase

/*
 * ABH/ABL pair is used for registering previous address bus state.
 * This can be used to keep the current address, freeing up the original
 * source of the address, such as the ALU or DI.
 */
always @(posedge clk)
    if( state != FSMSTATE.PUSH0 && state != FSMSTATE.PUSH1 && RDY && 
        state != FSMSTATE.PULL0 && state != FSMSTATE.PULL1 && state != FSMSTATE.PULL2 )
    begin
        ABL <= AB[7:0];
        ABH <= AB[15:8];
    end

/*
 * Data Out MUX 
 */
always @*
    case( state )
        FSMSTATE.WRITE:   DO <= ADD;

        FSMSTATE.JSR0,
        FSMSTATE.BRK0:    DO <= PCH;

        FSMSTATE.JSR1,
        FSMSTATE.BRK1:    DO <= PCL;

        FSMSTATE.PUSH1:   DO <= php ? P : ADD;

        FSMSTATE.BRK2:    DO <= (IRQ | NMI_edge) ? (P & 8'b1110_1111) : P;

        default: DO <= regfile;
    endcase

/*
 * Write Enable Generator
 */

always @*
    case( state )
        FSMSTATE.BRK0,   // writing to stack or memory
        FSMSTATE.BRK1,
        FSMSTATE.BRK2,
        FSMSTATE.JSR0,
        FSMSTATE.JSR1,
        FSMSTATE.PUSH1,
        FSMSTATE.WRITE:   WE = 1;

        FSMSTATE.INDX3,  // only if doing a STA, STX or STY
        FSMSTATE.INDY3,
        FSMSTATE.ABSX2,
        FSMSTATE.ABS1,
        FSMSTATE.ZPX1,
        FSMSTATE.ZP0:     WE = store;

        default: WE = 0;
    endcase

/*
 * register file, contains A, X, Y and S (stack pointer) registers. At each
 * cycle only 1 of those registers needs to be accessed, so they combined
 * in a small memory, saving resources.
 */

reg write_register;             // set when register file is written

always @*
    case( state )
        FSMSTATE.DECODE: write_register = load_reg & ~plp;

        FSMSTATE.PULL1, 
         FSMSTATE.RTS2, 
         FSMSTATE.RTI3,
         FSMSTATE.BRK3,
         FSMSTATE.JSR0,
         FSMSTATE.JSR2 : write_register = 1;

       default: write_register = 0;
    endcase

/*
 * BCD adjust logic
 */

always @(posedge clk)
    adj_bcd <= adc_sbc & D;     // '1' when doing a BCD instruction

reg [3:0] ADJL;
reg [3:0] ADJH;

// adjustment term to be added to ADD[3:0] based on the following
// adj_bcd: '1' if doing ADC/SBC with D=1
// adc_bcd: '1' if doing ADC with D=1
// HC     : half carry bit from ALU
always @* begin
    casex( {adj_bcd, adc_bcd, HC} )
         3'b0xx: ADJL = 4'd0;   // no BCD instruction
         3'b100: ADJL = 4'd10;  // SBC, and digital borrow
         3'b101: ADJL = 4'd0;   // SBC, but no borrow
         3'b110: ADJL = 4'd0;   // ADC, but no carry
         3'b111: ADJL = 4'd6;   // ADC, and decimal/digital carry
    endcase
end

// adjustment term to be added to ADD[7:4] based on the following
// adj_bcd: '1' if doing ADC/SBC with D=1
// adc_bcd: '1' if doing ADC with D=1
// CO     : carry out bit from ALU
always @* begin
    casex( {adj_bcd, adc_bcd, CO} )
         3'b0xx: ADJH = 4'd0;   // no BCD instruction
         3'b100: ADJH = 4'd10;  // SBC, and digital borrow
         3'b101: ADJH = 4'd0;   // SBC, but no borrow
         3'b110: ADJH = 4'd0;   // ADC, but no carry
         3'b111: ADJH = 4'd6;   // ADC, and decimal/digital carry
    endcase
end

/*
 * write to a register. Usually this is the (BCD corrected) output of the
 * ALU, but in case of the FSMSTATE.JSR0 we use the S register to temporarily store
 * the PCL. This is possible, because the S register itself is stored in
 * the ALU during those cycles.
 */
always @(posedge clk)
    if( write_register & RDY )
        AXYS[regsel] <= (state == FSMSTATE.JSR0) ? DIMUX : { ADD[7:4] + ADJH, ADD[3:0] + ADJL };

/*
 * register select logic. This determines which of the A, X, Y or
 * S registers will be accessed. 
 */

always @*  
    case( state )
        FSMSTATE.INDY1,
        FSMSTATE.INDX0,
        FSMSTATE.ZPX0,
        FSMSTATE.ABSX0  : regsel = index_y ? SEL_Y : SEL_X;


        FSMSTATE.DECODE : regsel = dst_reg; 

        FSMSTATE.BRK0,
        FSMSTATE.BRK3,
        FSMSTATE.JSR0,
        FSMSTATE.JSR2,
        FSMSTATE.PULL0,
        FSMSTATE.PULL1,
        FSMSTATE.PUSH1,
        FSMSTATE.RTI0,
        FSMSTATE.RTI3,
        FSMSTATE.RTS0,
        FSMSTATE.RTS2   : regsel = SEL_S;
        
        default: regsel = src_reg; 
    endcase

/*
 * ALU
 */

ALU ALU( .clk(clk),
         .op(alu_op),
         .right(alu_shift_right),
         .AI(AI),
         .BI(BI),
         .CI(CI),
         .BCD(adc_bcd & (state == FSMSTATE.FETCH)),
         .CO(CO),
         .OUT(ADD),
         .V(AV),
         .Z(AZ),
         .N(AN),
         .HC(HC),
         .RDY(RDY) );

/*
 * Select current ALU operation
 */

always @*
    case( state )
        FSMSTATE.READ:   alu_op = op;

        FSMSTATE.BRA1:   alu_op = backwards ? OP_SUB : OP_ADD; 

        FSMSTATE.FETCH,
        FSMSTATE.REG :   alu_op = op; 

        FSMSTATE.DECODE,
        FSMSTATE.ABS1:   alu_op = 1'bx;

        FSMSTATE.PUSH1,
        FSMSTATE.BRK0,
        FSMSTATE.BRK1,
        FSMSTATE.BRK2,
        FSMSTATE.JSR0,
        FSMSTATE.JSR1:   alu_op = OP_SUB;

     default:   alu_op = OP_ADD;
    endcase

/*
 * Determine shift right signal to ALU
 */

always @*
    if( state == FSMSTATE.FETCH || state == FSMSTATE.REG || state == FSMSTATE.READ )
        alu_shift_right = shift_right;
    else
        alu_shift_right = 0;

/*
 * Sign extend branch offset.  
 */

always @(posedge clk)
    if( RDY )
        backwards <= DIMUX[7];

/* 
 * ALU A Input MUX 
 */

always @*
    case( state )
        FSMSTATE.JSR1,
        FSMSTATE.RTS1,
        FSMSTATE.RTI1,
        FSMSTATE.RTI2,
        FSMSTATE.BRK1,
        FSMSTATE.BRK2,
        FSMSTATE.INDX1:  AI = ADD;

        FSMSTATE.REG,
        FSMSTATE.ZPX0,
        FSMSTATE.INDX0,
        FSMSTATE.ABSX0,
        FSMSTATE.RTI0,
        FSMSTATE.RTS0,
        FSMSTATE.JSR0,
        FSMSTATE.JSR2,
        FSMSTATE.BRK0,
        FSMSTATE.PULL0,
        FSMSTATE.INDY1,
        FSMSTATE.PUSH0,
        FSMSTATE.PUSH1:  AI = regfile;

        FSMSTATE.BRA0,
        FSMSTATE.READ:   AI = DIMUX;

        FSMSTATE.BRA1:   AI = ABH;       // don't use PCH in case we're 

        FSMSTATE.FETCH:  AI = load_only ? 0 : regfile;

        FSMSTATE.DECODE,
        FSMSTATE.ABS1:   AI = 8'hxx;     // don't care

        default:  AI = 0;
    endcase


/*
 * ALU B Input mux
 */

always @*
    case( state )
         FSMSTATE.BRA1,
         FSMSTATE.RTS1,
         FSMSTATE.RTI0,
         FSMSTATE.RTI1,
         FSMSTATE.RTI2,
         FSMSTATE.INDX1,
         FSMSTATE.READ,
         FSMSTATE.REG,
         FSMSTATE.JSR0,
         FSMSTATE.JSR1,
         FSMSTATE.JSR2,
         FSMSTATE.BRK0,
         FSMSTATE.BRK1,
         FSMSTATE.BRK2,
         FSMSTATE.PUSH0, 
         FSMSTATE.PUSH1,
         FSMSTATE.PULL0,
         FSMSTATE.RTS0:  BI = 8'h00;
        
         FSMSTATE.BRA0:  BI = PCL;

         FSMSTATE.DECODE,
         FSMSTATE.ABS1:  BI = 8'hxx;

         default:       BI = DIMUX;
    endcase

/*
 * ALU CI (carry in) mux
 */

always @*
    case( state )
        FSMSTATE.INDY2,
        FSMSTATE.BRA1,
        FSMSTATE.ABSX1:  CI = CO;

        FSMSTATE.DECODE,
        FSMSTATE.ABS1:   CI = 1'bx;

        FSMSTATE.READ,
        FSMSTATE.REG:    CI = rotate ? C :
                     shift ? 0 : inc;

        FSMSTATE.FETCH:  CI = rotate  ? C : 
                     compare ? 1 : 
                     (shift | load_only) ? 0 : C;

        FSMSTATE.PULL0,
        FSMSTATE.RTI0,
        FSMSTATE.RTI1,
        FSMSTATE.RTI2,
        FSMSTATE.RTS0,
        FSMSTATE.RTS1,
        FSMSTATE.INDY0,
        FSMSTATE.INDX1:  CI = 1; 

        default:        CI = 0;
    endcase

/*
 * Processor Status Register update
 *
 */

/*
 * Update C flag when doing ADC/SBC, shift/rotate, compare
 */
always @(posedge clk )
    if( shift && state == FSMSTATE.WRITE ) 
        C <= CO;
    else if( state == FSMSTATE.RTI2 )
        C <= DIMUX[0];
    else if( ~write_back && state == FSMSTATE.DECODE ) begin
        if( adc_sbc | shift | compare )
            C <= CO;
        else if( plp )
            C <= ADD[0];
        else begin
            if( sec ) C <= 1;
            if( clc ) C <= 0;
        end
    end

/*
 * Update Z, N flags when writing A, X, Y, Memory, or when doing compare
 */

always @(posedge clk) 
    if( state == FSMSTATE.WRITE ) 
        Z <= AZ;
    else if( state == FSMSTATE.RTI2 )
        Z <= DIMUX[1];
    else if( state == FSMSTATE.DECODE ) begin
        if( plp )
            Z <= ADD[1];
        else if( (load_reg & (regsel != SEL_S)) | compare | bit_ins )
            Z <= AZ;
    end

always @(posedge clk)
    if( state == FSMSTATE.WRITE )
        N <= AN;
    else if( state == FSMSTATE.RTI2 )
        N <= DIMUX[7];
    else if( state == FSMSTATE.DECODE ) begin
        if( plp )
            N <= ADD[7];
        else if( (load_reg & (regsel != SEL_S)) | compare )
            N <= AN;
    end else if( state == FSMSTATE.FETCH && bit_ins ) 
        N <= DIMUX[7];

/*
 * Update I flag
 */

always @(posedge clk)
    if( state == FSMSTATE.BRK3 )
        I <= 1;
    else if( state == FSMSTATE.RTI2 )
        I <= DIMUX[2];
    else if( state == FSMSTATE.REG ) begin
        if( sei ) I <= 1;
        if( cli ) I <= 0;
    end else if( state == FSMSTATE.DECODE )
        if( plp ) I <= ADD[2];

/*
 * Update D flag
 */
always @(posedge clk ) 
    if( state == FSMSTATE.RTI2 )
        D <= DIMUX[3];
    else if( state == FSMSTATE.DECODE ) begin
        if( sed ) D <= 1;
        if( cld ) D <= 0;
        if( plp ) D <= ADD[3];
    end

/*
 * Update V flag
 */
always @(posedge clk )
    if( state == FSMSTATE.RTI2 ) 
        V <= DIMUX[6];
    else if( state == FSMSTATE.DECODE ) begin
        if( adc_sbc ) V <= AV;
        if( clv )     V <= 0;
        if( plp )     V <= ADD[6];
    end else if( state == FSMSTATE.FETCH && bit_ins ) 
        V <= DIMUX[6];

/*
 * Instruction decoder
 */

/*
 * IR register/mux. Hold previous DI value in IRHOLD in FSMSTATE.PULL0 and FSMSTATE.PUSH0
 * states. In these states, the IR has been prefetched, and there is no
 * time to read the IR again before the next decode.
 */

always @(posedge clk )
    if( reset )
        IRHOLD_valid <= 0;
    else if( RDY ) begin
        if( state == FSMSTATE.PULL0 || state == FSMSTATE.PUSH0 ) begin
            IRHOLD <= DIMUX;
            IRHOLD_valid <= 1;
        end else if( state == FSMSTATE.DECODE )
            IRHOLD_valid <= 0;
    end

assign IR = (IRQ & ~I) | NMI_edge
          ? 8'h00 
          : IRHOLD_valid ? IRHOLD : DIMUX;

always @(posedge clk )
    if( RDY )
        DIHOLD <= DI;

assign DIMUX = ~RDY ? DIHOLD : DI;

/*
 * Additional control signals
 */

always @(posedge clk)
     if( reset )
         res <= 1;
     else if( state == FSMSTATE.DECODE )
         res <= 0;

always @(posedge clk)
     if( state == FSMSTATE.DECODE && RDY )
        casex( IR )
                8'b0xx01010,    // ASLA, ROLA, LSRA, RORA
                8'b0xxxxx01,    // ORA, AND, EOR, ADC
                8'b100x10x0,    // DEY, TYA, TXA, TXS
                8'b1010xxx0,    // LDA/LDX/LDY 
                8'b10111010,    // TSX
                8'b1011x1x0,    // LDX/LDY
                8'b11001010,    // DEX
                8'b1x1xxx01,    // LDA, SBC
                8'bxxx01000:    // DEY, TAY, INY, INX
                                load_reg <= 1;

                default:        load_reg <= 0;
        endcase

always @(posedge clk)
     if( state == FSMSTATE.DECODE && RDY )
        casex( IR )
                8'b1110_1000,   // INX
                8'b1100_1010,   // DEX
                8'b101x_xx10:   // LDX, TAX, TSX
                                dst_reg <= SEL_X;

                8'b0x00_1000,   // PHP, PHA
                8'b1001_1010:   // TXS
                                dst_reg <= SEL_S;

                8'b1x00_1000,   // DEY, DEX
                8'b101x_x100,   // LDY
                8'b1010_x000:   // LDY #imm, TAY
                                dst_reg <= SEL_Y;

                default:        dst_reg <= SEL_A;
        endcase

always @(posedge clk)
     if( state == FSMSTATE.DECODE && RDY )
        casex( IR )
                8'b1011_1010:   // TSX 
                                src_reg <= SEL_S; 

                8'b100x_x110,   // STX
                8'b100x_1x10,   // TXA, TXS
                8'b1110_xx00,   // INX, CPX
                8'b1100_1010:   // DEX
                                src_reg <= SEL_X; 

                8'b100x_x100,   // STY
                8'b1001_1000,   // TYA
                8'b1100_xx00,   // CPY
                8'b1x00_1000:   // DEY, INY
                                src_reg <= SEL_Y;

                default:        src_reg <= SEL_A;
        endcase

always @(posedge clk) 
     if( state == FSMSTATE.DECODE && RDY )
        casex( IR )
                8'bxxx1_0001,   // INDY
                8'b10x1_x110,   // LDX/STX zpg/abs, Y
                8'bxxxx_1001:   // abs, Y
                                index_y <= 1;

                default:        index_y <= 0;
        endcase


always @(posedge clk)
     if( state == FSMSTATE.DECODE && RDY )
        casex( IR )
                8'b100x_x1x0,   // STX, STY
                8'b100x_xx01:   // STA
                                store <= 1;

                default:        store <= 0;

        endcase

always @(posedge clk )
     if( state == FSMSTATE.DECODE && RDY )
        casex( IR )
                8'b0xxx_x110,   // ASL, ROL, LSR, ROR
                8'b11xx_x110:   // DEC/INC 
                                write_back <= 1;

                default:        write_back <= 0;
        endcase


always @(posedge clk )
     if( state == FSMSTATE.DECODE && RDY )
        casex( IR )
                8'b101x_xxxx:   // LDA, LDX, LDY
                                load_only <= 1;
                default:        load_only <= 0;
        endcase

always @(posedge clk )
     if( state == FSMSTATE.DECODE && RDY )
        casex( IR )
                8'b111x_x110,   // INC 
                8'b11x0_1000:   // INX, INY
                                inc <= 1;

                default:        inc <= 0;
        endcase

always @(posedge clk )
     if( (state == FSMSTATE.DECODE || state == FSMSTATE.BRK0) && RDY )
        casex( IR )
                8'bx11x_xx01:   // SBC, ADC
                                adc_sbc <= 1;

                default:        adc_sbc <= 0;
        endcase

always @(posedge clk )
     if( (state == FSMSTATE.DECODE || state == FSMSTATE.BRK0) && RDY )
        casex( IR )
                8'b011x_xx01:   // ADC
                                adc_bcd <= D;

                default:        adc_bcd <= 0;
        endcase

always @(posedge clk )
     if( state == FSMSTATE.DECODE && RDY )
        casex( IR )
                8'b0xxx_x110,   // ASL, ROL, LSR, ROR (abs, absx, zpg, zpgx)
                8'b0xxx_1010:   // ASL, ROL, LSR, ROR (acc)
                                shift <= 1;

                default:        shift <= 0;
        endcase

always @(posedge clk )
     if( state == FSMSTATE.DECODE && RDY )
        casex( IR )
                8'b11x0_0x00,   // CPX, CPY (imm/zp)
                8'b11x0_1100,   // CPX, CPY (abs)
                8'b110x_xx01:   // CMP 
                                compare <= 1;

                default:        compare <= 0;
        endcase

always @(posedge clk )
     if( state == FSMSTATE.DECODE && RDY )
        casex( IR )
                8'b01xx_xx10:   // ROR, LSR
                                shift_right <= 1;

                default:        shift_right <= 0; 
        endcase

always @(posedge clk )
     if( state == FSMSTATE.DECODE && RDY )
        casex( IR )
                8'b0x1x_1010,   // ROL A, ROR A
                8'b0x1x_x110:   // ROR, ROL 
                                rotate <= 1;

                default:        rotate <= 0; 
        endcase

always @(posedge clk )
     if( state == FSMSTATE.DECODE && RDY )
        casex( IR )
                8'b00xx_xx10:   // ROL, ASL
                                op <= OP_ROL;

                8'b0010_x100:   // BIT zp/abs   
                                op <= OP_AND;

                8'b01xx_xx10:   // ROR, LSR
                                op <= OP_A;

                8'b1000_1000,   // DEY
                8'b1100_1010,   // DEX 
                8'b110x_x110,   // DEC 
                8'b11xx_xx01,   // CMP, SBC
                8'b11x0_0x00,   // CPX, CPY (imm, zpg)
                8'b11x0_1100:   op <= OP_SUB;

                8'b010x_xx01,   // EOR
                8'b00xx_xx01:   // ORA, AND
                                op <= { 2'b11, IR[6:5] };
                
                default:        op <= OP_ADD; 
        endcase

always @(posedge clk )
     if( state == FSMSTATE.DECODE && RDY )
        casex( IR )
                8'b0010_x100:   // BIT zp/abs   
                                bit_ins <= 1;

                default:        bit_ins <= 0; 
        endcase

/*
 * special instructions
 */
always @(posedge clk )
     if( state == FSMSTATE.DECODE && RDY ) begin
        php <= (IR == 8'h08);
        clc <= (IR == 8'h18);
        plp <= (IR == 8'h28);
        sec <= (IR == 8'h38);
        cli <= (IR == 8'h58);
        sei <= (IR == 8'h78);
        clv <= (IR == 8'hb8);
        cld <= (IR == 8'hd8);
        sed <= (IR == 8'hf8);
        brk <= (IR == 8'h00);
     end

always @(posedge clk)
    if( RDY )
        cond_code <= IR[7:5];

always @*
    case( cond_code )
            3'b000: cond_true = ~N;
            3'b001: cond_true = N;
            3'b010: cond_true = ~V;
            3'b011: cond_true = V;
            3'b100: cond_true = ~C;
            3'b101: cond_true = C;
            3'b110: cond_true = ~Z;
            3'b111: cond_true = Z;
    endcase


reg NMI_1 = 0;          // delayed NMI signal

always @(posedge clk)
    NMI_1 <= NMI;

always @(posedge clk )
    if( NMI_edge && state == FSMSTATE.BRK3 )
        NMI_edge <= 0;
    else if( NMI & ~NMI_1 )
        NMI_edge <= 1;

endmodule
