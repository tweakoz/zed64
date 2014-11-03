`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    17:38:03 11/02/2014 
// Design Name: 
// Module Name:    c6_test 
// Project Name: 
// Target Devices: 
// Tool versions: 
// Description: 
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////
module c6_test( 
	input C6_CLK_8MHZ,
	output [7:0] SID_DATA,
	output [4:0] SID_ADDR,
	output SID_RES,
	output SID_CLK,
	output SID_CS
    );

reg [31:0] clk_counter;

reg [7:0] out_sid_data;
reg [4:0] out_sid_addr;
reg out_sid_res;
reg out_sid_cs;

initial begin
  out_sid_data <= 0;
  out_sid_addr <= 0;
  out_sid_res <= 0;
  out_sid_cs <= 1;
end

always @(posedge C6_CLK_8MHZ)
   begin
		clk_counter <= clk_counter+1;
	end

assign SID_CLK = clk_counter[24];
assign SID_DATA = out_sid_data;
assign SID_ADDR = out_sid_addr;
assign SID_RES = out_sid_res;
assign SID_CS = out_sid_cs;

endmodule
