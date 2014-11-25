module nexys4_top(
	input sys_reset,
	input sys_clk,
	output [3:0] vgaR, vgaG, vgaB,
	output vgaH, vgaV );

	wire [15:0] vpu_addr;
	wire [7:0] vpu_data_in;

	wire [6:0] dot_clock;
	wire clocks_locked;
	
	wire act_reset = ! sys_reset;
	
    wire cpu_clk = dot_clock[0];
    wire cpu_wena;
    wire [15:0] cpu_addr;
    wire [7:0] cpu_data_out;
    wire [7:0] cpu_data_in;



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
	
wire pix_clk = dot_clock[0];

dpram dpr(
        cpu_clk, cpu_wena,
        cpu_addr, cpu_data_in, cpu_data_out,
        pix_clk, 1'b0,
        vpu_addr, 8'b0, vpu_data_in );

cpu the_cpu(
        reset, cpu_clk,
        cpu_addr,
        cpu_data_in,
        cpu_data_out );

vidcon V(
    act_reset,
    dot_clock[4], // 65 mhz
	vpu_addr,
	vpu_data_in,
    vgaR,
    vgaG,
    vgaB,
    vgaH,
    vgaV
);

endmodule
