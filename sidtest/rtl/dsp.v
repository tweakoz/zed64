`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////

module pwm(input reset, input pwm_clk, input inp_clk, input [11:0] dac_in, output pwm_out);

	reg [11:0] inp_latch;
	reg [12:0] pwm_accum;

	initial begin
		inp_latch <= 0;
		pwm_accum <= 0;
	end
	
	always @(posedge inp_clk)
			inp_latch <= dac_in;

	always @(posedge pwm_clk) 
		if( reset )
			pwm_accum <= 0;
		else
			pwm_accum <= (pwm_accum[11:0] + inp_latch);

	assign pwm_out = pwm_accum[12];

endmodule

////////////////////////////////////////////////////////////////////////////
/*
Tdouble MoogVCF::run(double aud_in, double fc, double res)
{
  double f = fc * 1.16; // .116
  double fb = res * (1.0 - 0.15 * f * f);
  double in0 = aud_in-(out4 * fb);
  in0 *= 0.35013 * (f*f)*(f*f); //(*=.35*.116^4)
  out1 = in0 + 0.3 * in1 + (1 - f) * out1; // Pole 1
  in1  = in0s;
  out2 = out1 + 0.3 * in2 + (1 - f) * out2;  // Pole 2
  in2  = out1;
  out3 = out2 + 0.3 * in3 + (1 - f) * out3;  // Pole 3
  in3  = out2;
  out4 = out3 + 0.3 * in4 + (1 - f) * out4;  // Pole 4
  in4  = out3;
  return out4;
}*/

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

module rom( in_clock, in_addr, out_data );
////////
parameter ADDRWIDTH = 4;
parameter DATAWIDTH = 24;
parameter init_file = "yo";

////////
input   wire    in_clock;
input   wire    [ADDRWIDTH-1:0]  in_addr;
output  reg     [DATAWIDTH-1:0]  out_data;
////////
// Shared memory
////////
reg [DATAWIDTH-1:0] rom [(2**ADDRWIDTH)-1:0];
////////
initial begin
    $readmemh(init_file, rom);
end
////////
always @(posedge in_clock) begin
    out_data <= rom[in_addr];
end
////////
endmodule

///////////////////////////////////////////////////////////////////////////////
/// SINGLE_PORT_RAM ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

module single_port_ram( in_clock, in_writeenable, in_addr, in_data, out_data );

parameter ADDRWIDTH = 4;
parameter DATAWIDTH = 36; // 72

input   wire    in_clock;
input   wire    in_writeenable;
input   wire    [ADDRWIDTH-1:0]  in_addr;
input   wire    [DATAWIDTH-1:0]  in_data;
output  reg     [DATAWIDTH-1:0]  out_data;
integer i;

// Shared memory
reg [DATAWIDTH-1:0] mem [(2**ADDRWIDTH)-1:0];
 
initial begin
	out_data = 0;
	for( i=0; i<2**ADDRWIDTH; i=i+1 ) begin
		mem[i] = 0;
	end
end

// Port A
always @(posedge in_clock) begin
    out_data <= mem[in_addr];
    if(in_writeenable ) begin
        out_data[DATAWIDTH-1:0] <= in_data[DATAWIDTH-1:0];
        mem[in_addr][DATAWIDTH-1:0] <= in_data[DATAWIDTH-1:0];
    end
end
endmodule

///////////////////////////////////////////////////////////////////////////////
/// DSP_CORE //////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

module dsp_core(	input ireset, 
					input sample_clk, 
					input dsp_clk, 		// 128mhz
					input dsp_180_clk,  // 128mhz 180 phase
					output [17:0] dsp_result,
					output [7:0] io_addr,
					output [7:0] io_data,
					output io_wr_ena
				);

	reg [17:0] dsp_res_reg;
	
	assign dsp_result = dsp_res_reg;
	wire sid_clock = sample_clk;

	/////////////////////////////////////////
	// clock phase generator
	/////////////////////////////////////////

	reg dsp_clk_div2, dsp_clk_180_div2, dsp_clk_180_div2n;

	always @(posedge dsp_clk)
		dsp_clk_div2 <= reset ? 0 : ! dsp_clk_div2;

	always @(posedge dsp_180_clk) begin
		dsp_clk_180_div2 <= reset ? 0 : ! dsp_clk_180_div2;
        dsp_clk_180_div2n <= reset ? 1 : ! dsp_clk_180_div2n;
        end

	/////////////////////////////////////////

	//reg [71:0] IF_accum, DE_accum, EX_accum, IO_accum, WB_accum;
	reg [7:0] IF_PC, DE_PC, EX_PC, IO_PC, WB_PC;
	reg reset;

	//always @(posedge dsp_180_clk) begin
	//	WB_accum <= IO_accum;
		//IO_accum <= EX_accum;
		//EX_accum <= DE_accum;
		//DE_accum <= IF_accum;
	//	end

	/////////////////////////////////////////
	// DSP ROM	
	/////////////////////////////////////////

	reg [31:0] dsp_rom [0:127];
	reg [31:0] IF_instruction_word;
	reg [31:0] DE_instruction_word;
	reg [31:0] EX_instruction_word;
	reg [31:0] IO_instruction_word;
	reg [31:0] WB_instruction_word;

	always @(posedge dsp_clk) begin
		WB_instruction_word <= IO_instruction_word;
		IO_instruction_word <= EX_instruction_word;
		EX_instruction_word <= DE_instruction_word;
		DE_instruction_word <= IF_instruction_word;
		IF_instruction_word <= dsp_rom[IF_PC];
		reset <= ireset;
		end

	wire [7:0] IF_opcode = IF_instruction_word[31:24];
	wire [7:0] DE_opcode = DE_instruction_word[31:24];
	wire [7:0] EX_opcode = EX_instruction_word[31:24];
	wire [7:0] IO_opcode = IO_instruction_word[31:24];
	wire [7:0] WB_opcode = WB_instruction_word[31:24];

	wire [7:0] IF_opclass = IF_instruction_word[31:28];
	wire [7:0] DE_opclass = DE_instruction_word[31:28];
	wire [7:0] EX_opclass = EX_instruction_word[31:28];
	wire [7:0] IO_opclass = IO_instruction_word[31:28];
	wire [7:0] WB_opclass = WB_instruction_word[31:28];

	wire [3:0] IF_opcencdataA = IF_instruction_word[7:4];
	wire [3:0] DE_opcencdataA = DE_instruction_word[7:4];
	wire [3:0] EX_opcencdataA = EX_instruction_word[7:4];
	wire [3:0] IO_opcencdataA = IO_instruction_word[7:4];
	wire [3:0] WB_opcencdataA = WB_instruction_word[7:4];

	wire [3:0] IF_opcencdataB = IF_instruction_word[3:0];
	wire [3:0] DE_opcencdataB = DE_instruction_word[3:0];
	wire [3:0] EX_opcencdataB = EX_instruction_word[3:0];
	wire [3:0] IO_opcencdataB = IO_instruction_word[3:0];
	wire [3:0] WB_opcencdataB = WB_instruction_word[3:0];

	wire [3:0] IF_opcencdataDST = IF_instruction_word[23:20];
	wire [3:0] DE_opcencdataDST = DE_instruction_word[23:20];
	wire [3:0] EX_opcencdataDST = EX_instruction_word[23:20];
	wire [3:0] IO_opcencdataDST = IO_instruction_word[23:20];
	wire [3:0] WB_opcencdataDST = WB_instruction_word[23:20];

	wire [23:0] IF_ins_data = IF_instruction_word[23:0];
	wire [23:0] DE_ins_data = DE_instruction_word[23:0];
	wire [23:0] EX_ins_data = EX_instruction_word[23:0];
	wire [23:0] IO_ins_data = IO_instruction_word[23:0];
	wire [23:0] WB_ins_data = WB_instruction_word[23:0];

	/////////////////////////////////////////////////
	// process addrgen, latch registers for operations
	/////////////////////////////////////////////////

	reg [24:0] R_MUL_OPA, R_MUL_OPC;
	reg [17:0] R_MUL_OPB, R_MUL_OPD;
	reg [35:0] R[0:7];
	wire [42:0] prod_ab, prod_cd;
	reg [42:0] product_ab, product_cd;

	assign #22 prod_ab = {7'd0,R_MUL_OPA} * R_MUL_OPB; // OPTIMIZE ME!!!
	assign #22 prod_cd = {7'd0,R_MUL_OPC} * R_MUL_OPD; // OPTIMIZE ME!!!

	always @(posedge dsp_180_clk) begin
		if(DE_opcode==8'h50) begin
			R_MUL_OPA <= R[DE_opcencdataA[2:0]];
			R_MUL_OPB <= R[DE_opcencdataB[2:0]];
		end
		if(DE_opcode==8'h51) begin
			R_MUL_OPC <= R[DE_opcencdataA[2:0]];
			R_MUL_OPD <= R[DE_opcencdataB[2:0]];
		end
	end

	always @(posedge dsp_clk)
		case( WB_opcode )
			8'h50: product_ab <= prod_ab;
			8'h51: product_cd <= prod_cd;
		endcase

	/////////////////////////////////////////////////
	/////////////////////////////////////////////////
	// DE : decode
	/////////////////////////////////////////////////
	/////////////////////////////////////////////////

	reg [35:0] DE_R_ALU_OPA, DE_R_ALU_OPB;
	reg [35:0] EX_R_ALU_OPA, EX_R_ALU_OPB;

	wire lthan = EX_R_ALU_OPA<EX_R_ALU_OPB;
	wire gthan = EX_R_ALU_OPA>EX_R_ALU_OPB;
	wire equals = EX_R_ALU_OPA==EX_R_ALU_OPB;

	always @(posedge dsp_180_clk) begin
		EX_R_ALU_OPA <= DE_R_ALU_OPA;
		EX_R_ALU_OPB <= DE_R_ALU_OPB;
		if( DE_opcode[7:4]==4'h4 ) begin
			DE_R_ALU_OPA <= R[DE_opcencdataA[2:0]];
			DE_R_ALU_OPB <= R[DE_opcencdataB[2:0]];
			end
	end

	////////////////////////////////////////////////
	// data hazard check
	////////////////////////////////////////////////

	wire ifins_is_mut =  ((IF_opclass>=4'h4) // will this instruction mutate a register?
					  && (IF_opclass<=4'h7)) // alu/mul/shft/comp ?
	                  || (IF_opclass==4'h1)  // load ?
	                  || (IF_opcode==8'h32); // pop ?

	wire ifins_might_have_deps = (    (IF_opclass>=4'h4) 
							  	   && (IF_opclass<=4'h7) // alu/mul/shft/comp ?
							     )
							     ||
							     (	  (IF_opcode>=8'hf3) // jz ?
							   	   && (IF_opcode<=8'hf4) // jnz
							   	 );

	reg deins_is_mut, exins_is_mut, ioins_is_mut;

	always @(posedge dsp_180_clk) begin
		ioins_is_mut <= exins_is_mut;
		exins_is_mut <= deins_is_mut;
		deins_is_mut <= ifins_is_mut;
	end
	
	wire ifde_pot_data_hazard = deins_is_mut && (IF_PC!=DE_PC) && ((IF_opcencdataA==DE_opcencdataDST) || (IF_opcencdataB==DE_opcencdataDST));
	wire ifex_pot_data_hazard = exins_is_mut && (IF_PC!=EX_PC) && ((IF_opcencdataA==EX_opcencdataDST) || (IF_opcencdataB==EX_opcencdataDST));
	wire ifio_pot_data_hazard = ioins_is_mut && (IF_PC!=IO_PC) && ((IF_opcencdataA==IO_opcencdataDST) || (IF_opcencdataB==IO_opcencdataDST));
	wire hazard = ifins_might_have_deps & (ifde_pot_data_hazard|ifex_pot_data_hazard|ifio_pot_data_hazard);

	/////////////////////////////////////////////////
	/////////////////////////////////////////////////
	// process PC/REGISTER/STACK
	/////////////////////////////////////////////////
	/////////////////////////////////////////////////

	reg [35:0] stack[0:63];
	reg [35:0] PC_stack[0:15];
	reg [5:0] stack_index;
	reg [5:0] PC_stack_index;

	//wire WAIT_FINISHED = (WB_PC==IF_PC);
	wire STALL_FINISHED = (WB_PC==IF_PC);
	wire JREG_IS_ZERO = (R[IF_opcencdataDST]==0);
	wire WAIT_COND = (R[IF_opcencdataB]==0);
	wire FIRST_INS_CLK = (DE_PC!=IF_PC);

	always @(posedge dsp_180_clk) begin
		WB_PC <= IO_PC;
		IO_PC <= EX_PC;
		EX_PC <= DE_PC;
		DE_PC <= IF_PC;
		case( IF_opcode )
			8'hf3: begin // JZ
				IF_PC <= STALL_FINISHED // stall until jeq at last stage
					  ? (JREG_IS_ZERO ? IF_ins_data[7:0] : IF_PC+1)
					  : IF_PC; // stall
				end
			8'hf4: begin // JNZ
				IF_PC <= STALL_FINISHED // stall until jeq at last stage
					  ? (JREG_IS_ZERO ? IF_PC+1 : IF_ins_data[7:0] )
					  : IF_PC; // stall
				end
			8'hf0: begin // CALL
				PC_stack[PC_stack_index] <= IF_PC;
				PC_stack_index <= PC_stack_index+1;
				IF_PC <= IF_ins_data[7:0]; 
				end
			8'hf1: begin // RET
				IF_PC <= (PC_stack[PC_stack_index-1])+1;
				PC_stack_index <= PC_stack_index-1;
				end
			8'hf5: begin // WAIT
				IF_PC <= (sample_clk==IF_opcencdataB[0]) ? IF_PC+1 : IF_PC;
				end
			8'hf2: IF_PC <= IF_ins_data[7:0]; // jmp
			8'hff: IF_PC <= 0; // reset
			// TODO delay slot/stall when dynamically branching
			8'h40: IF_PC <= reset ? 0 : (hazard ? IF_PC : IF_PC+1); // inc
			8'h41: IF_PC <= reset ? 0 : (hazard ? IF_PC : IF_PC+1); // inc
			8'h42: IF_PC <= reset ? 0 : (hazard ? IF_PC : IF_PC+1); // inc
			8'h50: IF_PC <= reset ? 0 : (hazard ? IF_PC : IF_PC+1); // inc
			8'h51: IF_PC <= reset ? 0 : (hazard ? IF_PC : IF_PC+1); // inc
			default: IF_PC <= reset ? 0 : IF_PC+1; // inc
		endcase
	end


	/////////////////////////////////////////////////
	/////////////////////////////////////////////////
	// EX : process opcodes
	/////////////////////////////////////////////////
	/////////////////////////////////////////////////

	reg [35:0] alu_reg;
	reg [35:0] DE_lod_reg, EX_lod_reg, IO_lod_reg;
	reg [35:0] shf_reg;
	reg [35:0] cmp_reg;

	////////////////////////////////////////////////

	always @(posedge dsp_180_clk) begin
		case( EX_opcode )
			8'h40: alu_reg <= DE_R_ALU_OPA+DE_R_ALU_OPB; 				// add
			//////////////////////////////////////////////////////////////
			8'h41: alu_reg <= DE_R_ALU_OPA-DE_R_ALU_OPB;					// sub
			//////////////////////////////////////////////////////////////
			//8'h42: product <= {36'd0,R_OPA} * {36'd0,R_OPB};			// mul
			//8'h42: alu_reg <= {R_OPA} * {R_OPB};			// mul
			//////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////
			8'h42: alu_reg <= DE_R_ALU_OPA&DE_R_ALU_OPB; 	// and
			8'h43: alu_reg <= DE_R_ALU_OPA|DE_R_ALU_OPB; 	// or
			8'h44: alu_reg <= DE_R_ALU_OPA^DE_R_ALU_OPB; 	// xor
			8'h45: alu_reg <= ~DE_R_ALU_OPA; 				// negate
		endcase
	end //always @(posedge dsp_180_clk) begin

	//////////////////////////////////////////////////////////////

	wire [2:0] EX_RDST = EX_opcencdataDST[2:0];
	wire [35:0] EX_REG_DST = R[EX_RDST];

	always @(posedge dsp_180_clk) begin
		case( EX_opcode )
			//////////////////////////////////////////////////////////////
			8'h60:													// shiftL
				begin
				  case( EX_opcencdataB )
					4'h0: shf_reg <= { EX_REG_DST[34:0], 1'd0 };		// shiftL1 
					4'h1: shf_reg <= { EX_REG_DST[33:0], 2'd0 };		// shiftL2
					//4'h2: shf_reg <= { EX_REG_DST[32:0], 3'd0 };		// shiftL3
					4'h3: shf_reg <= { EX_REG_DST[31:0], 4'd0 };		// shiftL4
					//4'h4: shf_reg <= { EX_REG_DST[30:0], 5'd0 };		// shiftL5
					//4'h5: shf_reg <= { EX_REG_DST[29:0], 6'd0 };		// shiftL6
					//4'h6: shf_reg <= { EX_REG_DST[28:0], 7'd0 };		// shiftL7
					4'h7: shf_reg <= { EX_REG_DST[27:0], 8'd0 };		// shiftL8
					//4'h8: shf_reg <= { EX_REG_DST[23:0], 12'd0 };		// shiftL12
					4'h9: shf_reg <= { EX_REG_DST[19:0], 16'd0 };		// shiftL16
					4'ha: shf_reg <= { EX_REG_DST[17:0], 18'd0 };		// shiftL18
					4'hb: shf_reg <= { EX_REG_DST[11:0], 24'd0 };		// shiftL24
					4'hc: shf_reg <= { EX_REG_DST[6:0], 29'd0 };		// shiftL32
				  endcase
				end
			//////////////////////////////////////////////////////////////
			8'h61:													// shiftR
				begin
				  case( EX_opcencdataB )
					4'h0: shf_reg <= { 1'd0 , EX_REG_DST[35:1] };		// shiftR1
					4'h1: shf_reg <= { 2'd0 , EX_REG_DST[35:2] };		// shiftR2
					//4'h2: shf_reg <= { 3'd0 , EX_REG_DST[35:3] };		// shiftR3
					4'h3: shf_reg <= { 4'd0 , EX_REG_DST[35:4] };		// shiftR4
					//4'h4: shf_reg <= { 5'd0 , EX_REG_DST[35:5] };		// shiftR5
					//4'h5: shf_reg <= { 6'd0 , EX_REG_DST[35:6] };		// shiftR6
					//4'h6: shf_reg <= { 7'd0 , EX_REG_DST[35:7] };		// shiftR7
					4'h7: shf_reg <= { 8'd0 , EX_REG_DST[35:8] };		// shiftR8
					//4'h8: shf_reg <= { 12'd0, EX_REG_DST[35:12] };	// shiftR12
					4'h9: shf_reg <= { 16'd0, EX_REG_DST[35:16] }; 	// shiftR16
					4'ha: shf_reg <= { 18'd0, EX_REG_DST[35:18] }; 	// shiftR18
					4'hb: shf_reg <= { 24'd0, EX_REG_DST[35:24] }; 	// shiftR24
					4'hc: shf_reg <= { 29'd0, EX_REG_DST[35:29] }; 	// shiftR32
				  endcase
				end
		endcase
	end //always @(posedge dsp_180_clk) begin

	//////////////////////////////////////////////////////////////

	always @(posedge dsp_180_clk) begin
		case( EX_opcode )
			//////////////////////////////////////////////////////////////
			8'h70: cmp_reg <= { 35'd0, lthan }; 		// <
			8'h71: cmp_reg <= { 35'd0, gthan }; 		// >
			8'h72: cmp_reg <= { 35'd0, equals }; 		// ==
		endcase
	end 
			
	/////////////////////////////////////////////////
	/////////////////////////////////////////////////
	// IO : IO / MEM
	/////////////////////////////////////////////////
	/////////////////////////////////////////////////

	wire [35:0] mem_inp_data;
	reg [35:0] reg_load;
	wire mem_latch_reg = (EX_opcode==8'h20); // st.mem
	wire mem_write_ena = (IO_opcode==8'h20); // st.mem
	wire [3:0] mem_addr = IO_opcencdataB[3:0];

	wire [2:0] IO_RB = IO_opcencdataB[2:0];
	wire [2:0] IO_RDST = IO_opcencdataDST[2:0];
	wire [35:0] IO_REG_B = R[IO_RB];
	wire [35:0] IO_REG_DST = R[IO_RDST];

	reg [35:0] mem_output_IO;

	single_port_ram dsp_ram( dsp_180_clk, mem_write_ena, mem_addr, mem_output_IO, mem_inp_data );

	always @(posedge mem_latch_reg)
		mem_output_IO <= R[EX_opcencdataDST[2:0]];

	reg [7:0] IO_ADDR, IO_DATA;

	always @(posedge dsp_180_clk) begin
		if( IO_opcode==8'h21) begin // st.io
			IO_DATA <= IO_REG_B[7:0];
			IO_ADDR <= IO_REG_DST[7:0];
		end
	end

	assign io_addr = IO_ADDR;
	assign io_data = IO_DATA;
	assign io_wr_ena = (IO_ADDR!=255);

	/////////////////////////////////////////////////
	/////////////////////////////////////////////////
	// WB : WriteBack
	/////////////////////////////////////////////////
	/////////////////////////////////////////////////

	always @(posedge dsp_180_clk)
		case ( WB_opcode[7:0] )
			8'h10: R[WB_opcencdataDST] <= mem_inp_data; // ld.mem
			8'h11: R[WB_opcencdataDST] <= mem_inp_data; // ld.memr
			8'h12: R[WB_opcencdataDST] <= {18'd0,WB_ins_data[17:0]}; // ld.i18
			8'h13: R[WB_opcencdataDST][17:0] <= WB_ins_data[17:0]; // ld.l18
			8'h14: R[WB_opcencdataDST][35:18] <= WB_ins_data[17:0]; // ld.u18
			8'h16: R[WB_opcencdataDST] <= product_ab[35:0]; // ld.prod.ab.lo
			8'h17: R[WB_opcencdataDST] <= product_ab[42:7]; // ld.prod.ab.hi
			8'h18: R[WB_opcencdataDST] <= product_cd[35:0]; // ld.prod.cd.lo
			8'h19: R[WB_opcencdataDST] <= product_cd[42:7]; // ld.prod.cd.hi
			//////////////////////////////////////////////////////////////
			8'h15: 										// ld.var
				begin
				  if( WB_ins_data[19:0] == 20'd0 )
					R[WB_opcencdataDST] <= 36'd0;
				  //if( ins_data[23:0] == 24'd1 )
					//accum <= { fc, 8'd0 }; 	// fc48
				  else if( WB_ins_data[19:0] == 19'd2 )
					R[WB_opcencdataDST] <= 24'd100;
				  else 
					R[WB_opcencdataDST] <= 36'd0;
				end
			//////////////////////////////////////////////////////////////
			8'h32: R[WB_opcencdataDST] <= stack[stack_index-1]; // POP
			8'h41: R[WB_opcencdataDST] <= alu_reg; // alu
			8'h42: R[WB_opcencdataDST] <= alu_reg; // alu
			8'h60: R[WB_opcencdataDST] <= shf_reg; // shifter
			8'h61: R[WB_opcencdataDST] <= shf_reg; // shifter
			8'h70: R[WB_opcencdataDST] <= cmp_reg; // comparison				
			8'h71: R[WB_opcencdataDST] <= cmp_reg; // comparison				
			8'h72: R[WB_opcencdataDST] <= cmp_reg; // comparison				
		endcase

	always @(posedge dsp_clk)
		case( WB_opcode[7:0] )
			8'h31: stack_index <= reset ? 0 : stack_index+1; // push -> reg
			8'h32: stack_index <= reset ? 0 : stack_index-1; // pop -> reg
			default: stack_index <= reset ? 0 : stack_index;
		endcase

	always @(posedge dsp_clk)
		case( WB_opcode )
			8'h31: stack[stack_index] <= R[WB_opcencdataDST];
		endcase

	/////////////////////////////////////////////////

	always @(posedge dsp_clk)
		dsp_res_reg <= { 10'd0, IF_PC };

	/////////////////////////////////////////////////

	initial begin
		dsp_clk_div2 = 0;
		dsp_clk_180_div2 = 0;
		dsp_clk_180_div2n = 1;
		///////////////////////////////////////////////// 
		$readmemh("../test.hex",dsp_rom);
		///////////////////////////////////////////////// 
		IO_ADDR = 8'hff;
		IO_DATA = 8'd0;
		IF_PC = 0;
		stack_index = 0;
		PC_stack_index = 0;
		IF_instruction_word = 32'hffffffff;
		DE_instruction_word = 32'hffffffff;
	end

	/////////////////////////////////////////////////

endmodule

///////////////////////////////////////////////////////////////////////////////
/// DSP_TOP //////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

module dsp_top( input reset, 
				input sid_clock,
				input dcm_clk,
				input dcm_neg_clk,
				output pwm_out,
				output [7:0] io_addr,
				output [7:0] io_data,
				output io_wr_ena
 );
 
	reg [31:0] pwm_counter;
	
	always @(posedge dcm_clk)
		pwm_counter = reset ? 0 : pwm_counter+1;
		
	///////////////////////////////	

	reg [11:0] dac_output_latch;
		
	assign sample_clk = pwm_counter[12]; // 62.5khz
	assign update_clk = pwm_counter[20]; // 256 hz
		
	wire [17:0] dsp_result;

	dsp_core the_core(	reset, sid_clock, dcm_clk, dcm_neg_clk,
						dsp_result, io_addr, io_data, io_wr_ena );

	initial begin
		dac_output_latch <= 0;
	end

	always @(posedge sample_clk)
		begin
			dac_output_latch <= dsp_result[17:6];
		end
	///////////////////////////////

	pwm the_pwm( reset, dcm_clk, sample_clk, dac_output_latch, pwm_out );

endmodule


////////////////////////////////////////////////////////////////////////////

