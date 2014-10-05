`timescale 1ns/1ps

module tb_top_ise;

reg reset;
reg cpu_clk; // 100mhz
reg pix_clk; // 65mhz

wire [15:0] cpu_addr = 0;
wire [7:0] cpu_data_out = 0;
wire [7:0] cpu_data_in;
wire cpu_wena = 0;
wire cpu_din;
wire cpu_dout = 0;

wire [3:0] R,G,B;
wire H,V; // hsync, vsync
wire [15:0] vpu_addr;
wire [7:0] vram_data_in; // from vram
wire [7:0] vram_data_out = 0; // to vram

wire [11:0] char_addr;
wire [7:0] char_data;

///////////////////////////////////////////////////

cpu the_cpu(
		reset, cpu_clk,
		cpu_addr,
		cpu_data_in,
		cpu_data_out );

vidcon the_vpu(
    reset, pix_clk,
    vpu_addr, vram_data_in,
	R,G,B,H,V
);

dpram dpr(
		cpu_clk, cpu_wena,
		cpu_addr, cpu_data_in, cpu_data_out,
		pix_clk, 1'b0,
		vpu_addr, 8'b0, vram_data_in );

chargen cg( char_addr, char_data );



///////////////////////////////////////////////////

initial begin
    reset = 1;
    cpu_clk = 0;
    pix_clk = 0;    	

	#50
	reset = 0;
end

always # 5 cpu_clk = ~cpu_clk; //100 mhz

always # 7.69 pix_clk = ~pix_clk; // 65 mhz

endmodule
