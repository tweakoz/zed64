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

cpu6502_inc INC();

reg [5:0] state;
assign outstate = state;

always @(posedge clk or posedge reset)
    if( reset )
        state <= INC.BRK0;
    else if( RDY ) case( state )
        INC.DECODE  : 
            casex ( IR )
                8'b0000_0000:   state <= INC.BRK0;
                8'b0010_0000:   state <= INC.JSR0;
                8'b0010_1100:   state <= INC.ABS0;  // BIT abs
                8'b0100_0000:   state <= INC.RTI0;  // 
                8'b0100_1100:   state <= INC.JMP0;
                8'b0110_0000:   state <= INC.RTS0;
                8'b0110_1100:   state <= INC.JMPI0;
                8'b0x00_1000:   state <= INC.PUSH0;
                8'b0x10_1000:   state <= INC.PULL0;
                8'b0xx1_1000:   state <= INC.REG;   // CLC, SEC, CLI, SEI 
                8'b1xx0_00x0:   state <= INC.FETCH; // IMM
                8'b1xx0_1100:   state <= INC.ABS0;  // X/Y abs
                8'b1xxx_1000:   state <= INC.REG;   // DEY, TYA, ... 
                8'bxxx0_0001:   state <= INC.INDX0;
                8'bxxx0_01xx:   state <= INC.ZP0;
                8'bxxx0_1001:   state <= INC.FETCH; // IMM
                8'bxxx0_1101:   state <= INC.ABS0;  // even E column
                8'bxxx0_1110:   state <= INC.ABS0;  // even E column
                8'bxxx1_0000:   state <= INC.BRA0;  // odd 0 column
                8'bxxx1_0001:   state <= INC.INDY0; // odd 1 column
                8'bxxx1_01xx:   state <= INC.ZPX0;  // odd 4,5,6,7 columns
                8'bxxx1_1001:   state <= INC.ABSX0; // odd 9 column
                8'bxxx1_11xx:   state <= INC.ABSX0; // odd C, D, E, F columns
                8'bxxxx_1010:   state <= INC.REG;   // <shift> A, TXA, ...  NOP
            endcase

        INC.ZP0     : state <= write_back ? INC.READ : INC.FETCH;

        INC.ZPX0    : state <= INC.ZPX1;
        INC.ZPX1    : state <= write_back ? INC.READ : INC.FETCH;

        INC.ABS0    : state <= INC.ABS1;
        INC.ABS1    : state <= write_back ? INC.READ : INC.FETCH;

        INC.ABSX0   : state <= INC.ABSX1;
        INC.ABSX1   : state <= (CO | store | write_back) ? INC.ABSX2 : INC.FETCH;
        INC.ABSX2   : state <= write_back ? INC.READ : INC.FETCH;

        INC.INDX0   : state <= INC.INDX1;
        INC.INDX1   : state <= INC.INDX2;
        INC.INDX2   : state <= INC.INDX3;
        INC.INDX3   : state <= INC.FETCH;

        INC.INDY0   : state <= INC.INDY1;
        INC.INDY1   : state <= INC.INDY2;
        INC.INDY2   : state <= (CO | store) ? INC.INDY3 : INC.FETCH;
        INC.INDY3   : state <= INC.FETCH;

        INC.READ    : state <= INC.WRITE;
        INC.WRITE   : state <= INC.FETCH;
        INC.FETCH   : state <= INC.DECODE;

        INC.REG     : state <= INC.DECODE;
        
        INC.PUSH0   : state <= INC.PUSH1;
        INC.PUSH1   : state <= INC.DECODE;

        INC.PULL0   : state <= INC.PULL1;
        INC.PULL1   : state <= INC.PULL2; 
        INC.PULL2   : state <= INC.DECODE;

        INC.JSR0    : state <= INC.JSR1;
        INC.JSR1    : state <= INC.JSR2;
        INC.JSR2    : state <= INC.JSR3;
        INC.JSR3    : state <= INC.FETCH; 

        INC.RTI0    : state <= INC.RTI1;
        INC.RTI1    : state <= INC.RTI2;
        INC.RTI2    : state <= INC.RTI3;
        INC.RTI3    : state <= INC.RTI4;
        INC.RTI4    : state <= INC.DECODE;

        INC.RTS0    : state <= INC.RTS1;
        INC.RTS1    : state <= INC.RTS2;
        INC.RTS2    : state <= INC.RTS3;
        INC.RTS3    : state <= INC.FETCH;

        INC.BRA0    : state <= cond_true ? INC.BRA1 : INC.DECODE;
        INC.BRA1    : state <= (CO ^ backwards) ? INC.BRA2 : INC.DECODE;
        INC.BRA2    : state <= INC.DECODE;

        INC.JMP0    : state <= INC.JMP1;
        INC.JMP1    : state <= INC.DECODE; 

        INC.JMPI0   : state <= INC.JMPI1;
        INC.JMPI1   : state <= INC.JMP0;

        INC.BRK0    : state <= INC.BRK1;
        INC.BRK1    : state <= INC.BRK2;
        INC.BRK2    : state <= INC.BRK3;
        INC.BRK3    : state <= INC.JMP0;

    endcase

`ifdef DO_SIM

/*
 * easy to read names in simulator output
 */
reg [8*6-1:0] statename;

assign outstatename = statename;

always @*
    case( state )
            INC.DECODE: statename = "DECODE";
            INC.REG:    statename = "REG";
            INC.ZP0:    statename = "ZP0";
            INC.ZPX0:   statename = "ZPX0";
            INC.ZPX1:   statename = "ZPX1";
            INC.ABS0:   statename = "ABS0";
            INC.ABS1:   statename = "ABS1";
            INC.ABSX0:  statename = "ABSX0";
            INC.ABSX1:  statename = "ABSX1";
            INC.ABSX2:  statename = "ABSX2";
            INC.INDX0:  statename = "INDX0";
            INC.INDX1:  statename = "INDX1";
            INC.INDX2:  statename = "INDX2";
            INC.INDX3:  statename = "INDX3";
            INC.INDY0:  statename = "INDY0";
            INC.INDY1:  statename = "INDY1";
            INC.INDY2:  statename = "INDY2";
            INC.INDY3:  statename = "INDY3";
            INC.READ:  statename = "READ";
            INC.WRITE:  statename = "WRITE";
            INC.FETCH:  statename = "FETCH";
            INC.PUSH0:  statename = "PUSH0";
            INC.PUSH1:  statename = "PUSH1";
            INC.PULL0:  statename = "PULL0";
            INC.PULL1:  statename = "PULL1";
            INC.PULL2:  statename = "PULL2";
            INC.JSR0:   statename = "JSR0";
            INC.JSR1:   statename = "JSR1";
            INC.JSR2:   statename = "JSR2";
            INC.JSR3:   statename = "JSR3";
            INC.RTI0:   statename = "RTI0";
            INC.RTI1:   statename = "RTI1";
            INC.RTI2:   statename = "RTI2";
            INC.RTI3:   statename = "RTI3";
            INC.RTI4:   statename = "RTI4";
            INC.RTS0:   statename = "RTS0";
            INC.RTS1:   statename = "RTS1";
            INC.RTS2:   statename = "RTS2";
            INC.RTS3:   statename = "RTS3";
            INC.BRK0:   statename = "BRK0";
            INC.BRK1:   statename = "BRK1";
            INC.BRK2:   statename = "BRK2";
            INC.BRK3:   statename = "BRK3";
            INC.BRA0:   statename = "BRA0";
            INC.BRA1:   statename = "BRA1";
            INC.BRA2:   statename = "BRA2";
            INC.JMP0:   statename = "JMP0";
            INC.JMP1:   statename = "JMP1";
            INC.JMPI0:  statename = "JMPI0";
            INC.JMPI1:  statename = "JMPI1";
    endcase

//always @( PC )
//      $display( "%t, PC:%04x IR:%02x A:%02x X:%02x Y:%02x S:%02x C:%d Z:%d V:%d N:%d P:%02x", $time, PC, IR, A, X, Y, S, C, Z, V, N, P );

`endif

endmodule