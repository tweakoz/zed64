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

`define HLINES 1024 
`define HMAX 1344 
`define HFP 1024 
`define HSP (1024+136) 

`define VLINES 768 
`define VMAX 806 
`define VFP 768 
`define VSP (768+6)

///////////////////////////////////////////////////////////////////////////////////////
module frame_controller( input pixel_clk,
								 output [10:0] hcount,
								 output [10:0] vcount,
								 output reg hsync_pulse,
								 output reg vsync_pulse,
								 output reg hblank_region,
								 output reg vblank_region,
								 output reg frame_inc );

	///////////////////
	reg [10:0] hcounter;
	reg [10:0] vcounter;
	assign hcount = hcounter;
	assign vcount = vcounter;
	///////////////////
	always @(posedge pixel_clk) begin
		hcounter = (hcounter == `HMAX) 
		         ? 0 
					: (hcounter + 1);
		if(hcounter == (`HMAX-4)) 
			vcounter = (vcounter == `VMAX)
		            ? 0 
					   : (vcounter + 1);
		hsync_pulse <= ! (hcounter >= `HFP && hcounter < `HSP);
		vsync_pulse <= ! (vcounter >= `VFP && vcounter < `VSP);
		frame_inc <= ! (vcounter >= `VFP && vcounter < `VSP);
		hblank_region <= (hcounter >= `HLINES);
		vblank_region <= (vcounter >= `VLINES);
	end
	///////////////////
	initial begin
		hcounter <= 0;
		vcounter <= 0;
	end

endmodule

//////////////////////////////////////////////////////

module line_buffer( input pixel_clock,
						  input hsync_pulse,
						  input [10:0] hcount,
						  input [10:0] vcount,
						  output reg [15:0] VRAM_ADDR,
				        input wire [7:0] VRAM_DIN,
						  input wire fetch, // 1==fetch, 0==display
                    output reg [3:0] R,
						  output reg [3:0] G,
						  output reg [3:0] B );

wire WE;
wire [9:0] WLADDR;
wire [7:0] WLIN;
wire [7:0] WLOUT;

wire [9:0] xpos = (hcount<`HLINES) ? hcount[9:0] : (`HLINES-1);
wire [9:0] ypos = (vcount<`VLINES) ? vcount[9:0] : (`VLINES-1);

//////////////////////////////////////////////////////////
// RAM to hold decoded pixels for line
//////////////////////////////////////////////////////////

single_port_byte_sram line_ram(
	pixel_clock, WE, WLADDR, WLIN, WLOUT );
defparam line_ram.ADDRWIDTH = 10;
	
//////////////////////////////////////////////////////////
// ADDRESS GEN
//////////////////////////////////////////////////////////

wire [2:0] char_pixrow = ypos[2:0]; // pixel row within a character for this scanline
wire [2:0] x_state = hcount[2:0];		// xpos based state machine (8 states)
reg [11:0] bg_color;		// background color
reg [11:0] fg_color;		// foreground color
reg [7:0] pix8;
wire [2:0] pixbit = (3'd6-x_state);
wire pixel = pix8[pixbit];

reg [16:0] vram_offset;
// TODO wire [5:0] yoffset = calc y offset for CharCode lookup

wire [6:0] xbyteoffset = xpos[9:3] + {5'd0,1'd1};
wire [6:0] ycellpos = ypos[9:3];

always @(posedge pixel_clock) begin
	
   vram_offset = {2'd0,ycellpos,7'd0} + {4'd0,xbyteoffset};

	case( x_state )
		3'd2: // Gen CharCode ADDR
		    VRAM_ADDR = 16'h8000 + vram_offset[15:0];				
		3'd4: // Read CharCode, Gen PixRow ADDR
			 VRAM_ADDR = 16'hc000 + {5'd0,VRAM_DIN,char_pixrow};	 
		3'd6: // Read PixRow
			 pix8 = VRAM_DIN;										
	endcase

	R <= pixel ? fg_color[11:8] : bg_color[11:8];
	G <= pixel ? fg_color[7:4]  : bg_color[7:4];
	B <= pixel ? fg_color[3:0]  : bg_color[3:0];

end

initial begin
 pix8 <= 0;
 vram_offset <= 0;
 bg_color <= 12'h007;
 fg_color <= 12'hff0;
end

endmodule

//////////////////////////////////////////////////////

module vga( input pixel_clock,
				output [3:0] R,
				output [3:0] G,
				output [3:0] B,
				output hsync_pulse, 
				output vsync_pulse,
				output [15:0] VAB,
				input [7:0] VIN );

wire [10:0] hcount;
wire [10:0] vcount;
wire frame_inc;
wire hblank;
reg [31:0] frame_counter;
reg [63:0] line_counter;
wire [3:0] lR0, lG0, lB0, lR1, lG1, lB1;
wire [15:0] lAD0, lAD1;
assign lbsel = line_counter[0];

assign VAB = lAD0; //vlbsel ? lAD0 : lAD1;

line_buffer lbuf0( pixel_clock,
						hsync_pulse,
						hcount, vcount,
						lAD0, VIN,
						lbsel,
						lR0, lG0, lB0 );

frame_controller fc0(   pixel_clock,
								hcount, vcount,
								hsync_pulse, vsync_pulse,
								hblank_region, vblank_region,
								frame_inc );

always @(posedge frame_inc) begin
	frame_counter <= frame_counter+1;
end

always @(posedge hblank_region) begin
	line_counter <= line_counter+1;
end

wire blanking_region = (hblank_region||vblank_region);

assign R = blanking_region ? 4'h0 : lR0;
assign G = blanking_region ? 4'h0 : lG0;
assign B = blanking_region ? 4'h0 : lB0;

initial begin
	frame_counter <= 0;
	line_counter <= 0;
end

endmodule
