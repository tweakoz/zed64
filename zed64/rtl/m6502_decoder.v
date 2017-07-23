//======================================================================
//
// m6502_decoder.v
// ---------------
// Instruction decode logic for the MOS 6502 compatible CPU core.
// Note: This is not going to be a cycle accurate model.
//
//
// Author: Joachim Strombergson
// Copyright (c) 2016, Secworks Sweden AB
// All rights reserved.
//
// LICENSE: see m6502_license.txt
//
//======================================================================

module m6502_decoder(
                     input wire [7 : 0]  opcode,

                     output wire [2 : 0] instr_len,
                     output wire [2 : 0] opa,
                     output wire [2 : 0] opb,
                     output wire [2 : 0] alu_op,
                     output wire [2 : 0] destination,
                     output wire         update_carry,
                     output wire         update_zero,
                     output wire         update_overflow
                    );


  //----------------------------------------------------------------
  // Internal constant and parameter definitions.
  //----------------------------------------------------------------
  // M6502 Opcodes:
  localparam OP_BRK     = 8'h00;
  localparam OP_CLC     = 8'h18;
  localparam OP_SEC     = 8'h38;
  localparam OP_JMP     = 8'h4c;
  localparam OP_CLI     = 8'h58;
  localparam OP_RTS     = 8'h60;
  localparam OP_SEI     = 8'h78;
  localparam OP_TXA     = 8'h8a;
  localparam OP_LDA_IMM = 8'ha9;
  localparam OP_TAX     = 8'haa;
  localparam OP_INY     = 8'hc8;
  localparam OP_DEX     = 8'hca;
  localparam OP_INX     = 8'he8;
  localparam OP_NOP     = 8'hea;

  // Symbolic names for ALU operands.
  localparam OP_AREG  = 3'h0;
  localparam OP_XREG  = 3'h1;
  localparam OP_YREG  = 3'h2;
  localparam OP_ONE   = 3'h7;

  // Symbolic names for ALU operations.
  localparam ALU_NONE = 3'h0;
  localparam ALU_ADC  = 3'h1;

  // Symbolic names for destination codes.
  localparam DEST_MEM  = 3'h0;
  localparam DEST_AREG = 3'h1;
  localparam DEST_XREG = 3'h2;
  localparam DEST_YREG = 3'h3;


  //----------------------------------------------------------------
  // Registers including update variables and write enable.
  //----------------------------------------------------------------


  //----------------------------------------------------------------
  // Wires.
  //----------------------------------------------------------------
  reg [2 : 0] ilen;
  reg [2 : 0] dest;
  reg [2 : 0] alu;
  reg [2 : 0] a;
  reg [2 : 0] b;
  reg         carry;
  reg         zero;
  reg         overflow;


  //----------------------------------------------------------------
  // Concurrent connectivity for ports etc.
  //----------------------------------------------------------------
  assign instr_len       = ilen;
  assign destination     = dest;
  assign alu_op          = alu;
  assign opa             = a;
  assign opb             = b;
  assign update_carry    = carry;
  assign update_zero     = zero;
  assign update_overflow = overflow;


  //----------------------------------------------------------------
  // decoder
  // Instruction decoder. Based on the opcode provides info about
  // the operation to perform
  //----------------------------------------------------------------
  always @*
    begin : decoder
      ilen     = 3'h0;
      dest     = DEST_MEM;
      alu      = ALU_NONE;
      carry    = 0;
      zero     = 0;
      overflow = 0;


      case (opcode)
        OP_LDA_IMM:
          begin
            ilen = 3'h2;
            dest = DEST_AREG;
            alu  = ALU_NONE;
          end

        OP_INY:
          begin
            ilen = 3'h1;
            a    = OP_YREG;
            b    = OP_ONE;
            alu  = ALU_ADC;
            dest = DEST_YREG;
          end

        OP_INX:
          begin
            ilen = 3'h1;
            a    = OP_XREG;
            b    = OP_ONE;
            alu  = ALU_ADC;
            dest = DEST_XREG;
          end

        default:
          begin
          end
      endcase // case (opcode)
    end // decoder

endmodule // 6502_decoder

//======================================================================
// EOF m6502_decoder.v
//======================================================================
