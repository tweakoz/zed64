module nexys4_top(
	input sys_reset,
	input sys_clk,
	output [3:0] vgaR, vgaG, vgaB,
	output vgaH, vgaV );

	wire [15:0] vram_addr;
	reg [7:0] vram_data;

	wire [6:0] dot_clock;
	wire clocks_locked;
	
	wire act_reset = ! sys_reset;
	
zed_clockgen ZEDCLOCK(
	sys_clk,
	dot_clock[0],
	dot_clock[1],
	dot_clock[2],
	dot_clock[3],
	dot_clock[4],
	dot_clock[5],
	dot_clock[6],
	act_reset,
	clocks_locked );
	
vidcon V(
    act_reset,
    dot_clock[4], // 65 mhz
	vram_addr,
	vram_data,
    vgaR,
    vgaG,
    vgaB,
    vgaH,
    vgaV
);

endmodule
