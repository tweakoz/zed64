`timescale 1ns / 1ps

////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer:
//
// Create Date:   13:50:28 11/23/2014
// Design Name:   sidtest_top
// Module Name:   /projects/toz/zed64/hw_misc/sid_c6/sidtestbench.v
// Project Name:  sid_c6
// Target Device:  
// Tool versions:  
// Description: 
//
// Verilog Test Fixture created by ISE for module: sidtest_top
//
// Dependencies:
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
////////////////////////////////////////////////////////////////////////////////

module sidtestbench;

	// Inputs
	reg C6_CLK_8MHZ;
	reg BTN_0;
	reg BTN_1;

	// Outputs
	wire [7:0] SID_DATA;
	wire [4:0] SID_ADDR;
	wire SID_NOTRES;
	wire SID_CLK;
	wire SID_NOTCS;
	wire [3:0] C6_LED;
	wire PWM_OUT;
	
	// Instantiate the Unit Under Test (UUT)
	sidtest_top uut (
		.C6_CLK_8MHZ(C6_CLK_8MHZ), 
		.BTN_0(BTN_0), 
		.BTN_1(BTN_1), 
		.SID_DATA(SID_DATA), 
		.SID_ADDR(SID_ADDR), 
		.SID_NOTRES(SID_NOTRES), 
		.SID_CLK(SID_CLK), 
		.SID_NOTCS(SID_NOTCS), 
		.C6_LED(C6_LED),
		.PWM_OUT(PWM_OUT)
	);

	initial begin

		// Wait 10 sid cycles for global reset to finish
		C6_CLK_8MHZ = 0;
		BTN_0 = 0;
		BTN_1 = 0;

		// Initialize Inputs
		#2000;
		BTN_0 = 1;

		// Wait 10 sid cycles for global reset to finish
		#10000;
		BTN_0 = 0;
        
		// Add stimulus here

	end
      
	always #125 C6_CLK_8MHZ = ~C6_CLK_8MHZ;

endmodule

