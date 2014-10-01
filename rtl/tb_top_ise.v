module tb_top_ise;

reg reset;
reg clock;
wire [3:0] R,G,B;
wire [15:0] vram_addr;
wire [7:0] vram_data;
wire [11:0] char_addr;
wire [7:0] char_data;

initial begin
    reset = 1;
    clock = 0;
	#50
	reset = 0;
end

always # 5 clock = ~clock;

chargen cg( char_addr, char_data );

vidcon dut(
    reset,
    clock,
	R,G,B
);

endmodule
