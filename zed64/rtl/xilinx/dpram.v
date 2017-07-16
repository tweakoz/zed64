///////////////////////////////////////////////////////////////////////////////
// dual port ram
// Copyright (c) 2017 Michael T. Mayers
// michael@tweakoz.com
///////////////////////////////////////////////////////////////////////////////

module DualPortRam (
    input a_clk,
    input a_wena,
    input [11:0] a_addr,
    inout [7:0] a_data,
    input b_clk,
    input b_wena,
    input [11:0] b_addr,
    inout [7:0] b_data
);

///////////////////////////////////////////////////////////////////////////////

reg [7:0] mem [0:4095];
reg [7:0] aout, bout;

always @(posedge a_clk) begin: DPRAM_A_WRITE
    if (a_wena) begin
        mem[a_addr] <= a_data;
    end
    else begin
        aout <= mem[a_addr];
    end
end

always @(posedge b_clk) begin: DPRAM_B_WRITE
    if (b_wena) begin
        mem[b_addr] <= b_data;
    end
    else begin
        bout <= mem[b_addr];
    end
end

assign a_data = a_wena ? 8'bz : mem[a_addr];
assign b_data = b_wena ? 8'bz : mem[b_addr];

///////////////////////////////////////////////////////////////////////////////

endmodule
