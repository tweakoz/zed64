module bijunction(	input wire clk, inp_sel,
					input wire  [7:0] inp,
					output wire [7:0] out,
					inout wire  [7:0] iop );

reg [7:0] inp_reg, out_reg;

assign iop	= !inp_sel
			? inp_reg
			: 8'bZ;

assign out = !inp_sel
		   ? 8'bZ
		   : out_reg;

always @ (posedge clk) begin
	out_reg <= iop;
	inp_reg <= inp;
end

endmodule

