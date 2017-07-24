/*
 * Microcode state machine
 */

module cpu6502_fsm( input clk, 
                    input reset,
                    input [7:0] IR,
                    input RDY,
                    input write_back,
                    input CO,
                    input store,
                    input cond_true,
                    input backwards,
                    output [5:0] outstate,
`ifdef DO_SIM
                    output [8*6-1:0] outstatename
`endif
                     );

cpu6502_inc FSMSTATE();

reg [5:0] state;
assign outstate = state;

always @(posedge clk or posedge reset)
    if( reset )
        state <= FSMSTATE.BRK0;
    else if( RDY ) case( state )
        FSMSTATE.DECODE  : 
            casex ( IR )
                8'b0000_0000:   state <= FSMSTATE.BRK0;
                8'b0010_0000:   state <= FSMSTATE.JSR0;
                8'b0010_1100:   state <= FSMSTATE.ABS0;  // BIT abs
                8'b0100_0000:   state <= FSMSTATE.RTI0;  // 
                8'b0100_1100:   state <= FSMSTATE.JMP0;
                8'b0110_0000:   state <= FSMSTATE.RTS0;
                8'b0110_1100:   state <= FSMSTATE.JMPI0;
                8'b0x00_1000:   state <= FSMSTATE.PUSH0;
                8'b0x10_1000:   state <= FSMSTATE.PULL0;
                8'b0xx1_1000:   state <= FSMSTATE.REG;   // CLC, SEC, CLI, SEI 
                8'b1xx0_00x0:   state <= FSMSTATE.FETCH; // IMM
                8'b1xx0_1100:   state <= FSMSTATE.ABS0;  // X/Y abs
                8'b1xxx_1000:   state <= FSMSTATE.REG;   // DEY, TYA, ... 
                8'bxxx0_0001:   state <= FSMSTATE.INDX0;
                8'bxxx0_01xx:   state <= FSMSTATE.ZP0;
                8'bxxx0_1001:   state <= FSMSTATE.FETCH; // IMM
                8'bxxx0_1101:   state <= FSMSTATE.ABS0;  // even E column
                8'bxxx0_1110:   state <= FSMSTATE.ABS0;  // even E column
                8'bxxx1_0000:   state <= FSMSTATE.BRA0;  // odd 0 column
                8'bxxx1_0001:   state <= FSMSTATE.INDY0; // odd 1 column
                8'bxxx1_01xx:   state <= FSMSTATE.ZPX0;  // odd 4,5,6,7 columns
                8'bxxx1_1001:   state <= FSMSTATE.ABSX0; // odd 9 column
                8'bxxx1_11xx:   state <= FSMSTATE.ABSX0; // odd C, D, E, F columns
                8'bxxxx_1010:   state <= FSMSTATE.REG;   // <shift> A, TXA, ...  NOP
            endcase

        FSMSTATE.ZP0     : state <= write_back ? FSMSTATE.READ : FSMSTATE.FETCH;

        FSMSTATE.ZPX0    : state <= FSMSTATE.ZPX1;
        FSMSTATE.ZPX1    : state <= write_back ? FSMSTATE.READ : FSMSTATE.FETCH;

        FSMSTATE.ABS0    : state <= FSMSTATE.ABS1;
        FSMSTATE.ABS1    : state <= write_back ? FSMSTATE.READ : FSMSTATE.FETCH;

        FSMSTATE.ABSX0   : state <= FSMSTATE.ABSX1;
        FSMSTATE.ABSX1   : state <= (CO | store | write_back) ? FSMSTATE.ABSX2 : FSMSTATE.FETCH;
        FSMSTATE.ABSX2   : state <= write_back ? FSMSTATE.READ : FSMSTATE.FETCH;

        FSMSTATE.INDX0   : state <= FSMSTATE.INDX1;
        FSMSTATE.INDX1   : state <= FSMSTATE.INDX2;
        FSMSTATE.INDX2   : state <= FSMSTATE.INDX3;
        FSMSTATE.INDX3   : state <= FSMSTATE.FETCH;

        FSMSTATE.INDY0   : state <= FSMSTATE.INDY1;
        FSMSTATE.INDY1   : state <= FSMSTATE.INDY2;
        FSMSTATE.INDY2   : state <= (CO | store) ? FSMSTATE.INDY3 : FSMSTATE.FETCH;
        FSMSTATE.INDY3   : state <= FSMSTATE.FETCH;

        FSMSTATE.READ    : state <= FSMSTATE.WRITE;
        FSMSTATE.WRITE   : state <= FSMSTATE.FETCH;
        FSMSTATE.FETCH   : state <= FSMSTATE.DECODE;

        FSMSTATE.REG     : state <= FSMSTATE.DECODE;
        
        FSMSTATE.PUSH0   : state <= FSMSTATE.PUSH1;
        FSMSTATE.PUSH1   : state <= FSMSTATE.DECODE;

        FSMSTATE.PULL0   : state <= FSMSTATE.PULL1;
        FSMSTATE.PULL1   : state <= FSMSTATE.PULL2; 
        FSMSTATE.PULL2   : state <= FSMSTATE.DECODE;

        FSMSTATE.JSR0    : state <= FSMSTATE.JSR1;
        FSMSTATE.JSR1    : state <= FSMSTATE.JSR2;
        FSMSTATE.JSR2    : state <= FSMSTATE.JSR3;
        FSMSTATE.JSR3    : state <= FSMSTATE.FETCH; 

        FSMSTATE.RTI0    : state <= FSMSTATE.RTI1;
        FSMSTATE.RTI1    : state <= FSMSTATE.RTI2;
        FSMSTATE.RTI2    : state <= FSMSTATE.RTI3;
        FSMSTATE.RTI3    : state <= FSMSTATE.RTI4;
        FSMSTATE.RTI4    : state <= FSMSTATE.DECODE;

        FSMSTATE.RTS0    : state <= FSMSTATE.RTS1;
        FSMSTATE.RTS1    : state <= FSMSTATE.RTS2;
        FSMSTATE.RTS2    : state <= FSMSTATE.RTS3;
        FSMSTATE.RTS3    : state <= FSMSTATE.FETCH;

        FSMSTATE.BRA0    : state <= cond_true ? FSMSTATE.BRA1 : FSMSTATE.DECODE;
        FSMSTATE.BRA1    : state <= (CO ^ backwards) ? FSMSTATE.BRA2 : FSMSTATE.DECODE;
        FSMSTATE.BRA2    : state <= FSMSTATE.DECODE;

        FSMSTATE.JMP0    : state <= FSMSTATE.JMP1;
        FSMSTATE.JMP1    : state <= FSMSTATE.DECODE; 

        FSMSTATE.JMPI0   : state <= FSMSTATE.JMPI1;
        FSMSTATE.JMPI1   : state <= FSMSTATE.JMP0;

        FSMSTATE.BRK0    : state <= FSMSTATE.BRK1;
        FSMSTATE.BRK1    : state <= FSMSTATE.BRK2;
        FSMSTATE.BRK2    : state <= FSMSTATE.BRK3;
        FSMSTATE.BRK3    : state <= FSMSTATE.JMP0;

    endcase

`ifdef DO_SIM

/*
 * easy to read names in simulator output
 */
reg [8*6-1:0] statename;

assign outstatename = statename;

always @*
    case( state )
            FSMSTATE.DECODE: statename = "DECODE";
            FSMSTATE.REG:    statename = "REG";
            FSMSTATE.ZP0:    statename = "ZP0";
            FSMSTATE.ZPX0:   statename = "ZPX0";
            FSMSTATE.ZPX1:   statename = "ZPX1";
            FSMSTATE.ABS0:   statename = "ABS0";
            FSMSTATE.ABS1:   statename = "ABS1";
            FSMSTATE.ABSX0:  statename = "ABSX0";
            FSMSTATE.ABSX1:  statename = "ABSX1";
            FSMSTATE.ABSX2:  statename = "ABSX2";
            FSMSTATE.INDX0:  statename = "INDX0";
            FSMSTATE.INDX1:  statename = "INDX1";
            FSMSTATE.INDX2:  statename = "INDX2";
            FSMSTATE.INDX3:  statename = "INDX3";
            FSMSTATE.INDY0:  statename = "INDY0";
            FSMSTATE.INDY1:  statename = "INDY1";
            FSMSTATE.INDY2:  statename = "INDY2";
            FSMSTATE.INDY3:  statename = "INDY3";
            FSMSTATE.READ:  statename = "READ";
            FSMSTATE.WRITE:  statename = "WRITE";
            FSMSTATE.FETCH:  statename = "FETCH";
            FSMSTATE.PUSH0:  statename = "PUSH0";
            FSMSTATE.PUSH1:  statename = "PUSH1";
            FSMSTATE.PULL0:  statename = "PULL0";
            FSMSTATE.PULL1:  statename = "PULL1";
            FSMSTATE.PULL2:  statename = "PULL2";
            FSMSTATE.JSR0:   statename = "JSR0";
            FSMSTATE.JSR1:   statename = "JSR1";
            FSMSTATE.JSR2:   statename = "JSR2";
            FSMSTATE.JSR3:   statename = "JSR3";
            FSMSTATE.RTI0:   statename = "RTI0";
            FSMSTATE.RTI1:   statename = "RTI1";
            FSMSTATE.RTI2:   statename = "RTI2";
            FSMSTATE.RTI3:   statename = "RTI3";
            FSMSTATE.RTI4:   statename = "RTI4";
            FSMSTATE.RTS0:   statename = "RTS0";
            FSMSTATE.RTS1:   statename = "RTS1";
            FSMSTATE.RTS2:   statename = "RTS2";
            FSMSTATE.RTS3:   statename = "RTS3";
            FSMSTATE.BRK0:   statename = "BRK0";
            FSMSTATE.BRK1:   statename = "BRK1";
            FSMSTATE.BRK2:   statename = "BRK2";
            FSMSTATE.BRK3:   statename = "BRK3";
            FSMSTATE.BRA0:   statename = "BRA0";
            FSMSTATE.BRA1:   statename = "BRA1";
            FSMSTATE.BRA2:   statename = "BRA2";
            FSMSTATE.JMP0:   statename = "JMP0";
            FSMSTATE.JMP1:   statename = "JMP1";
            FSMSTATE.JMPI0:  statename = "JMPI0";
            FSMSTATE.JMPI1:  statename = "JMPI1";
    endcase

//always @( PC )
//      $display( "%t, PC:%04x IR:%02x A:%02x X:%02x Y:%02x S:%02x C:%d Z:%d V:%d N:%d P:%02x", $time, PC, IR, A, X, Y, S, C, Z, V, N, P );

`endif

endmodule