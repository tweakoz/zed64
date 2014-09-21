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

