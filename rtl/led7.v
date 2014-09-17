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

module counter  #(parameter N = 4) ( clr, clk, q );

input wire clr;
input wire clk;
output reg [N-1:0] q;

// N-bit counter
always @( posedge clk or posedge clr ) begin
 if (clr == 1)
	q <= 0;
 else
  q <= q + 1;
end
endmodule 

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////

module mux44 ( x, s, z );

input wire [15:0] x;
input wire [1:0] s;
output reg [3:0] z;

always @(*)
 case (s)
	0: z = x[3:0];
	1: z = x[7:4];
	2: z = x[11:8];
	3: z = x[15:12];
	default : z = x[3:0];
 endcase
endmodule 

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////

module hex7seg( digit, seg_out );

input wire [3:0] digit;
output reg [6:0] seg_out;

	always @(*) begin
	  case( digit )
		4'h0 : seg_out = 7'b1000000;
		4'h1 : seg_out = 7'b1111001;
		4'h2 : seg_out = 7'b0100100;
		4'h3 : seg_out = 7'b0110000;
		4'h4 : seg_out = 7'b0011001;
		4'h5 : seg_out = 7'b0010010;
		4'h6 : seg_out = 7'b0000010;
		4'h7 : seg_out = 7'b1111000;
		4'h8 : seg_out = 7'b0000000;
		4'h9 : seg_out = 7'b0011000;
		4'ha : seg_out = 7'b0001000;
		4'hb : seg_out = 7'b0000011;
		4'hc : seg_out = 7'b1000110;
		4'hd : seg_out = 7'b0100001;
		4'he : seg_out = 7'b0000110;
		4'hf : seg_out = 7'b0001110;
	  endcase
	end

endmodule

//////////////////////////////////////////////////////////////////////
// Example 10b: x7seg
//  (from digilent's intro to digital design pdf)
//////////////////////////////////////////////////////////////////////

module x7seg ( cclk, clr, x, seg_out, anode );

input wire cclk;
input wire clr;
input wire [15:0] x;
output wire [6:0] seg_out;
output wire [3:0] anode;

wire nq0;
wire nq1;
wire [3:0] digit;
wire [1:0] q;
assign nq1 = ~(q[1]);
assign nq0 = ~(q[0]);
assign anode[0] = q[0] | q[1];
assign anode[1] = nq0 | q[1];
assign anode[2] = q[0] | nq1;
assign anode[3] = nq0 | nq1;

hex7seg U1( .seg_out(seg_out),.digit(digit) );

mux44 U2 ( .s({q[1:0]}),.x(x),.z(digit));

counter #( .N(2)) U3( .clk(cclk),.clr(clr),.q(q[1:0]));

endmodule 

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
