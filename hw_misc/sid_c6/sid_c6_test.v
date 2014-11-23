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
	output [3:0] C6_LED,
	output [7:0] SID_DATA,
	output [4:0] SID_ADDR,
	output SID_NOTRES,
	output SID_CLK,
	output SID_NOTCS
    );

reg [31:0] clk_counter;
reg [31:0] reg_counter;

reg [7:0] out_sid_data;
reg [4:0] out_sid_addr;
reg out_sid_res;
reg out_sid_cs;

assign SID_DATA = out_sid_data;
assign SID_ADDR = out_sid_addr;
assign SID_NOTRES = ! out_sid_res;
assign SID_NOTCS = ! out_sid_cs;

assign SID_CLK = clk_counter[2]; // 1mhz
assign REG_CLK = clk_counter[2]; // ~8hz

initial begin
	clk_counter <= 0;
	reg_counter <= 0;
end

always @(posedge C6_CLK_8MHZ)
   begin
		clk_counter <= clk_counter+1;
	end

always @(posedge REG_CLK)
   begin
		reg_counter <= reg_counter+1;
	end

assign C6_LED = reg_counter[19:16];

always @(posedge REG_CLK)
	case (reg_counter[23:0])
	 24'h000001: 
		begin
		out_sid_res <= 0;
		end
	 24'h000010: 
		begin
		out_sid_res <= 1;
		end
	endcase

always @(posedge REG_CLK)
	case (reg_counter[19:0])
	 20'h00001: 
		begin
		out_sid_data <= 0;
		out_sid_addr <= 0;
		out_sid_cs <= 1;
		end
	 20'h00010: 
		begin // master vol
		out_sid_addr <= 5'h18; 
		out_sid_data <= 8'h0f;
		end
	 20'h00012: 
		begin
		out_sid_cs <= 0;
		end
	 20'h00014: 
		begin
		out_sid_cs <= 1;
		end
	 20'h00016: 
		begin // frequency
		out_sid_addr <= 5'h01;
		out_sid_data <= 8'h20;
		end
	 20'h00018: 
		begin
		out_sid_cs <= 0;
		end
	 20'h0001a: 
		begin
		out_sid_cs <= 1;
		end
	 20'h0001c: 
		begin // atk / dcy
		out_sid_addr <= 5'h05; 
		out_sid_data <= 8'h4c;
		end
	 20'h0001e: 
		begin
		out_sid_cs <= 0;
		end
	 20'h00020: 
		begin
		out_sid_cs <= 1;
		end
	 20'h00022: 
		begin // sus / rel
		out_sid_addr <= 5'h06; 
		out_sid_data <= 8'hff;
		end
	 20'h00024: 
		begin
		out_sid_cs <= 0;
		end
	 20'h00026: 
		begin
		out_sid_cs <= 1;
		end
	 20'h00028: 
		begin // gate
		out_sid_addr <= 5'h04; 
		out_sid_data <= 8'h21;
		end
	 20'h0002a: 
		begin
		out_sid_cs <= 0;
		end
	 20'h0002c: 
		begin
		out_sid_cs <= 1;
		end
	 20'hf0000: 
		begin // gate
		out_sid_addr <= 5'h04; 
		out_sid_data <= 8'h20;
		end
	 20'hf0002: 
		begin
		out_sid_cs <= 0;
		end
	 20'hf0004: 
		begin
		out_sid_cs <= 1;
		end
	endcase
	
begin
end


endmodule
