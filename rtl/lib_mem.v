//////////////////////////////////////////////////////////////////////
// 
//  Zed64 MetroComputer
//
//  Unless a module otherwise marked,
//   Copyright 2014, Michael T. Mayers (michael@tweakoz.com
//   Provided under the Creative Commons Attribution License 3.0
//    Please see https://creativecommons.org/licenses/by/3.0/us/legalcode
//   
//////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// memory abstraction (should be inferrable on altera and xilinx)
/////////////////////////////////////////////////////////////////////////////
module single_port_rom( in_clock, in_addr, out_data );
////////
parameter ADDRWIDTH = 10;
parameter DATAWIDTH = 32;
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

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

module single_port_byte_sram( in_clock, in_writeenable, in_addr, in_data, out_data );

parameter ADDRWIDTH = 10;
parameter init_file = "yo";

input   wire    in_clock;
input   wire    in_writeenable;
input   wire    [ADDRWIDTH-1:0]  in_addr;
input   wire    [8-1:0]  in_data;
output  reg     [8-1:0]  out_data;

// Shared memory
reg [8-1:0] mem [(2**ADDRWIDTH)-1:0];
 
//initial begin
 //   $readmemh(init_file,  mem);
//end

// Port A
always @(posedge in_clock) begin
    out_data <= mem[in_addr];
    if(in_writeenable ) begin
        out_data[7:0] <= in_data[7:0];
        mem[in_addr][7:0] <= in_data[7:0];
    end
end
endmodule
//

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

module rwport_byte_sram( in_aclock, in_awriteenable, in_aaddr, in_adata, out_adata,
								 in_bclock, in_baddr, out_bdata );

parameter ADDRWIDTH = 10;
parameter init_file = "rtl/code.txt";

input   wire    in_aclock;
input   wire    in_bclock;
input   wire    in_awriteenable;
input   wire    [ADDRWIDTH-1:0]  in_aaddr;
input   wire    [ADDRWIDTH-1:0]  in_baddr;
input   wire    [8-1:0]  in_adata;
output  reg     [8-1:0]  out_adata;
output  reg     [8-1:0]  out_bdata;

// Shared memory
reg [8-1:0] mem [(2**ADDRWIDTH)-1:0];
 
 integer i;
 
initial begin

	out_adata <= 0;
	out_bdata <= 0;

`ifdef XILINX_ISIM 

	for( i=0; i<2**ADDRWIDTH; i=i+1 )
		mem[i]=8'h0;

    mem[16'hfffe] = 8'h0; // reset vector
    mem[16'hffff] = 8'h0; // reset vector

`endif

    $readmemh(init_file,  mem);

end



// Port A
always @(posedge in_aclock) begin
    out_adata <= mem[in_aaddr];
    if(in_awriteenable ) begin
        out_adata[7:0] <= in_adata[7:0];
        mem[in_aaddr][7:0] <= in_adata[7:0];
    end
end

// Port B
always @(posedge in_bclock)
    out_bdata <= mem[in_baddr];

endmodule

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

module vram( in_aclock, in_awriteenable, in_aaddr, in_adata, out_adata,
								 in_bclock, in_baddr, out_bdata );

parameter ADDRWIDTH = 10;
parameter init_file = "yo";

input   wire    in_aclock, in_bclock;
input   wire    in_awriteenable;
input   wire    [ADDRWIDTH-1:0]  in_aaddr, in_baddr;
input   wire    [8-1:0]  in_adata;
output  reg     [8-1:0]  out_adata, out_bdata;

// Shared memory
reg [8-1:0] mem [(2**ADDRWIDTH)-1:0];
 
// Port A
always @(posedge in_aclock) begin
    out_adata <= mem[in_aaddr];
    if(in_awriteenable ) begin
        out_adata[7:0] <= in_adata[7:0];
        mem[in_aaddr][7:0] <= in_adata[7:0];
    end
end

// Port B
always @(posedge in_bclock)
    out_bdata <= mem[in_baddr];

endmodule

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

module single_port_multibyte_sram( in_clock, in_writeenable, in_addr, in_data, in_byteenable, out_data );

parameter ADDRWIDTH = 10;

input   wire    in_clock;
input   wire    in_writeenable;
input   wire    [ADDRWIDTH-1:0]  in_addr;
input   wire    [32-1:0]  in_data;
input   wire    [3:0] in_byteenable;
output  reg     [32-1:0]  out_data;

// Shared memory
reg [32-1:0] mem [(2**ADDRWIDTH)-1:0];
 
// Port A
always @(posedge in_clock) begin
    out_data <= mem[in_addr];
    if(in_writeenable && in_byteenable[0]) begin
        out_data[7:0] <= in_data[7:0];
        mem[in_addr][7:0] <= in_data[7:0];
    end
    if(in_writeenable && in_byteenable[1]) begin
        out_data[15:8] <= in_data[15:8];
        mem[in_addr][15:8] <= in_data[15:8];
    end
    if(in_writeenable && in_byteenable[2]) begin
        out_data[23:16] <= in_data[23:16];
        mem[in_addr][23:16] <=in_data[23:16];
    end
    if(in_writeenable && in_byteenable[3]) begin
        out_data[31:24] <= in_data[31:24];
        mem[in_addr][31:24] <= in_data[31:24];
    end
end
endmodule
//

/////////////////////////////////////////////////////////////////////////////
// Memory registers
/////////////////////////////////////////////////////////////////////////////

/*! \brief Contains the microcode ROM and D0-D7, A0-A7 registers.
 *
 * The memory_registers module contains:
 *  - data and address registers (D0-D7, A0-A7) implemented as an on-chip RAM.
 *  - the microcode implemented as an on-chip ROM.
 *
 * Currently this module contains <em>altsyncram</em> instantiations
 * from Altera Megafunction/LPM library.
 */
module memory_registers(
    input clock,
    input reset_n,

    // 0000,0001,0010,0011,0100,0101,0110: A0-A6, 0111: USP, 1111: SSP
    input [3:0] An_address,
    input [31:0] An_input,
    input An_write_enable,
    output [31:0] An_output,

    output reg [31:0] usp,

    input [2:0] Dn_address,
    input [31:0] Dn_input,
    input Dn_write_enable,
    // 001: byte, 010: word, 100: long
    input [2:0] Dn_size,
    output [31:0] Dn_output,

    input [8:0] micro_pc,
    output [87:0] micro_data
);

wire An_ram_write_enable    = (An_address == 4'b0111) ? 1'b0 : An_write_enable;

wire [31:0] An_ram_output;
assign An_output            = (An_address == 4'b0111) ? usp : An_ram_output;

wire [3:0] dn_byteena       = (Dn_size[0] == 1'b1) ? 4'b0001 :
                              (Dn_size[1] == 1'b1) ? 4'b0011 :
                              (Dn_size[2] == 1'b1) ? 4'b1111 :
                              4'b0000;

always @(posedge clock or negedge reset_n) begin
    if(reset_n == 1'b0)                                 usp <= 32'd0;
    else if(An_address == 4'b0111 && An_write_enable)   usp <= An_input;
end

// Register set An implemented as RAM.

 single_port_sram an_ram_inst(
    .in_clock (clock),
    .in_addr (An_address[2:0]),    
    .in_writeenable (An_ram_write_enable),
    .in_data (An_input),
    .out_data (An_ram_output),
	 .in_byteenable (4'b1111)
  );
defparam 
    an_ram_inst.ADDRWIDTH           = 3;

// Register set Dn implemented as RAM.
single_port_sram dn_ram_inst(
    .in_clock (clock),
    .in_addr (Dn_address),    
    .in_byteenable (dn_byteena),
    .in_writeenable (Dn_write_enable),
    .in_data (Dn_input),
    .out_data (Dn_output)
);
defparam 
    dn_ram_inst.ADDRWIDTH           = 3;

// Microcode ROM
single_port_rom micro_rom_inst(
    .in_clock (clock),
    .in_addr (micro_pc),
    .out_data (micro_data)
);
defparam
    //micro_rom_inst.operation_mode   = "ROM",
    micro_rom_inst.DATAWIDTH = 88,
    micro_rom_inst.ADDRWIDTH = 9,
    micro_rom_inst.init_file = "ao68000_microcode.x";
	 
endmodule
