`timescale 1ns / 1ps

module sidtest_top(
	input C6_CLK_8MHZ,
	input BTN_0,
	input BTN_1,
	output [7:0] SID_DATA,
	output [4:0] SID_ADDR,
	output SID_NOTRES,
	output SID_CLK,
	output SID_NOTCS,
	output [3:0] C6_LED,
	output PWM_OUT
    );


//////////////////////////////////////////////////

wire sid_reset = BTN_0;
wire dcm_reset = 0;

//////////////////////////////////////////////////
// instantiate the DSP

wire locked;
wire dcm_clk;
wire dcm_neg_clk;
	
clock_2_phase_128mhz dcm( 	.CLK_IN1(C6_CLK_8MHZ),		// 8mhz
				  			.CLK_OUT1(dcm_clk), 		// 128 mhz
				  			.CLK_OUT2(dcm_neg_clk), 	// 128 mhz
				  			.RESET(dcm_reset),
				  			.LOCKED(locked) );

wire [7:0] io_addr, io_data;
wire io_wr_ena;

//////////////////////////////////////////////////

reg [31:0] clk_counter;

always @(posedge dcm_clk)
	clk_counter <= clk_counter+1;

//////////////////////////////////////////////////

assign sid_clock = clk_counter[7];
reg [31:0] sid_cycle_counter;

always @(posedge sid_clock) // 1mhz
	sid_cycle_counter <= sid_reset ? 0 : sid_cycle_counter+1;	

dsp_top the_dsp( sid_reset, sid_clock,
				 dcm_clk, dcm_neg_clk, 
				 PWM_OUT,
				 io_addr, io_data, io_wr_ena );

//////////////////////////////////////////////////

//wire [15:0] sid_subcyc = sid_cycle_counter[15:0]; // 1mhz (1/16sec)
//wire [15:0] sid_subcyc = sid_cycle_counter[19:4]; // 1mhz/16 == 64Khz (1 sec loop)
wire [15:0] sid_subcyc = sid_cycle_counter[20:5]; // 1mhz/32 == 32Khz (2 sec loop)
//wire [15:0] sid_subcyc = sid_cycle_counter[21:6]; // 1m/64 == 16Khz (4sec loop)
//wire [7:0] sid_subcyc = sid_cycle_counter[21:14]; // 1m/256 == 4Khz (16sec loop)
//wire [15:0] sid_subcyc = sid_cycle_counter[27:20]; // 1m/1m == 1hz

assign C6_LED[3:0] = sid_subcyc[15:12];

//////////////////////////////////////////////////
	
assign SID_NOTRES = (sid_reset==0);
assign SID_NOTCS = (io_wr_ena==0);
assign SID_ADDR[4:0] = io_addr[4:0];
assign SID_DATA[7:0] = io_data[7:0];
assign SID_CLK = sid_clock;

//////////////////////////////////////////////////

initial begin
	sid_cycle_counter <= 0;
	clk_counter <=0;
	end



endmodule
