//////////////////////////////////////////////////////////////////////
// 
//	Zed64 MetroComputer
//
//  Unless a module otherwise marked,
//   Copyright 2014, Michael T. Mayers (michael@tweakoz.com
//   Provided under the Creative Commons Attribution License 3.0
//	  Please see https://creativecommons.org/licenses/by/3.0/us/legalcode
//   
//////////////////////////////////////////////////////////////////////

`timescale 1ps / 1ps

////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer:
//
// Create Date:   13:48:56 09/13/2014
// Design Name:   soc
// Module Name:   /projects/toz/toz64/rtl/testbench.v
// Project Name:  toz64
// Target Device:  
// Tool versions:  
// Description: 
//
// Verilog Test Fixture created by ISE for module: soc
//
// Dependencies:
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
////////////////////////////////////////////////////////////////////////////////

module testbench();

	// Inputs
	reg clk;
	reg btnCpuReset;
	reg RsRx;

	// Outputs
	wire [15:0] led;
	wire [6:0] seg;
	wire [3:0] an;
	wire [3:0] vgaR;
	wire [3:0] vgaG;
	wire [3:0] vgaB;
	wire vgaHsync;
	wire vgaVsync;
	wire RsTx;

	// Instantiate the Unit Under Test (UUT)
	soc uut (
		.clk(clk), 
		.led(led), 
		.seg(seg), 
		.an(an), 
		.btnNotCpuReset(btnCpuReset), 
		.vgaR(vgaR), 
		.vgaG(vgaG), 
		.vgaB(vgaB), 
		.vgaHsync(vgaHsync), 
		.vgaVsync(vgaVsync), 
		.RsRx(RsRx), 
		.RsTx(RsTx)
	);

	initial begin
		// Initialize Inputs
		clk = 0;
		btnCpuReset = 1;
		RsRx = 0;

		// Wait 100 ns for global reset to finish
		#20000000;
		//btnCpuReset = 0;

		// Wait 100 ns for global reset to finish
		#500000;
        
		// Add stimulus here
		//btnCpuReset = 1;


	end

      always #10000 clk = ~clk;

endmodule

