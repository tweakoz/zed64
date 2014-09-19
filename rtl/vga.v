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

`define USE_720P

`ifdef USE_1080P

	`define HLINES 1920 
	`define VLINES 1080 

	`define HMAX 2200 
	`define VMAX 1125 

	`define HFP (`HLINES+88) 
	`define VFP (`VLINES+4) 

	`define HSP (`HFP+44) 
	`define VSP (`VFP+5)

	`define HSYNCINV 1`d0
	`define VSYNCINV 1`d0

`endif

`ifdef USE_720P 

	`define HLINES 1280 
	`define VLINES 720 

	`define HFP 1390
	`define VFP 725

	`define HSP 1430
	`define VSP 730

	`define HMAX 1650
	`define VMAX 750

	`define HSYNCINV 0
	`define VSYNCINV 0

`endif


`ifdef USE_768p

	`define HLINES 1024 
	`define VLINES 768 

	`define HMAX 1344 
	`define VMAX 806 

	`define HFP (`HLINES+24) 
	`define VFP (`VLINES+3) 

	`define HSP (`HFP+136) 
	`define VSP (`VFP+6)

	`define HSYNCINV 1
	`define VSYNCINV 1

`endif

`define PALETTE_ADDR 			16'h2E00
`define CHARCELL_ROWSEL_ADDR 	16'h3800
`define CHARCELL_COLSEL_ADDR 	16'h3A00
`define COLRCELL_ROWSEL_ADDR 	16'h3C00
`define COLRCELL_COLSEL_ADDR 	16'h3E00

`define CHARCELL_ADDR 16'h4000
`define FONT_ADDR 16'hE000

///////////////////////////////////////////////////////////////////////////////////////
// Timing Generator
///////////////////////////////////////////////////////////////////////////////////////

module frame_controller(	input pixel_clk,
							output [10:0] hcount, vcount,
							output reg hsync_pulse, vsync_pulse,
							output reg hblank_region, vblank_region,
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
		hsync_pulse <= (hcounter >= `HFP && hcounter < `HSP) ^ `HSYNCINV;
		vsync_pulse <= (vcounter >= `VFP && vcounter < `VSP) ^ `VSYNCINV;
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

///////////////////////////////////////////////////////////////////////////////////////
// Pixel Generator
///////////////////////////////////////////////////////////////////////////////////////

module pixgen(	input pixel_clock,
				input hsync_pulse,
				input [10:0] hcount, vcount,
				output reg [15:0] VRAM_ADDR,
				input wire [7:0] VRAM_DIN,
                output reg [3:0] R, G, B );
		
	////////////////////////
	// ADDRESS GENERATION
	////////////////////////

	wire normalized_hsync = hsync_pulse  ^ `HSYNCINV;
	wire hsync_fetch = normalized_hsync; // TODO - end fetch near end of hblank
	
	// calc x and y pixel position

	wire [10:0] xpos = (hcount<`HLINES) ? hcount[10:0] : (`HLINES-1);
	wire [10:0] ypos = (vcount<`VLINES) ? vcount[10:0] : (`VLINES-1);

	reg [15:0] row_startL;
	reg [15:0] row_startH;
	reg [15:0] row_start;

	// calc cell

	wire [7:0] xcellpos = xpos[10:3];
	wire [7:0] ycellpos = ypos[10:3];

	// calc cell address

	//wire [17:0] y_byteoffset_mult = ycellpos * 11'd`HLINES;
	wire [15:0] y_byteoffset = row_start; //y_byteoffset_mult[17:2];
	wire [8:0] x_byteoffset = xcellpos;// + 1'd1;

	// calc cell pixrow

	wire [2:0] char_pixrow = ypos[2:0]; // pixel row within a character for this scanline

	////////////////////////
	// VRAM FETCH, PIXEL GEN
	////////////////////////
			
	wire [3:0] x_state = {hsync_fetch,hcount[2:0]};	// xpos based state machine (8 states)
	wire [2:0] pixbit = (3'd6-x_state[2:0]);
	reg [16:0] char_cell_vram_offset;
	reg [7:0] pal0L, pal0H, pal1L, pal1H;
	reg [11:0] bg_color; // background color
	reg [11:0] fg_color; // foreground color
	reg [7:0] pix8;
	wire pixel = pix8[pixbit];
	reg [7:0] hsync_fetch_cycle;
	
	always @(posedge pixel_clock) begin
		
	   char_cell_vram_offset = (y_byteoffset[15:0] + x_byteoffset[8:0]);

		hsync_fetch_cycle <= hsync_fetch 
		                   ? hsync_fetch_cycle+1 
						   : 0;
		
		case( hsync_fetch_cycle )
			//////////////////////////////////////////////////
			// pixel fetch
			//////////////////////////////////////////////////
			8'd0:
				case( x_state )
					4'd2: // Gen CharCode ADDR
						VRAM_ADDR <= `CHARCELL_ADDR + char_cell_vram_offset[15:0];				
					4'd4: // Read CharCode, Gen PixRow ADDR
						VRAM_ADDR <= `FONT_ADDR + {5'd0,VRAM_DIN,char_pixrow};	 
					4'd6: // Read PixRow
						pix8 <= VRAM_DIN;
					endcase
			//////////////////////////////////////////////////
			// hsync fetch
			//////////////////////////////////////////////////
			8'd1: // sched fetch row_startL
				VRAM_ADDR <= `CHARCELL_ROWSEL_ADDR + { ycellpos, 1'd0 };				
			8'd2: // sched fetch row startH
				VRAM_ADDR <= `CHARCELL_ROWSEL_ADDR + { ycellpos, 1'd1 };				
			8'd3: begin // sched fetch palette
				VRAM_ADDR <= `PALETTE_ADDR;				
			    row_startL <= VRAM_DIN;		
				end
			8'd4: begin // sched fetch palette
				VRAM_ADDR <= `PALETTE_ADDR+1;				
			    row_startH <= VRAM_DIN;
			    end
			8'd5: begin
				VRAM_ADDR <= `PALETTE_ADDR+2;				
			    row_start <= { row_startH, row_startL };
			    pal0L <= VRAM_DIN;
			    end
			8'd6: begin
				VRAM_ADDR <= `PALETTE_ADDR+3;				
			    pal0H <= VRAM_DIN;
			    end
			8'd7: begin
			    pal1L <= VRAM_DIN;
			    bg_color <= { pal0H[7:0], pal0L[7:4] };
			    end
			8'd8:
			    pal1H <= VRAM_DIN;
			8'd9:
			    fg_color <= { pal1H[7:0], pal1L[7:4] };

		endcase
		
		// PIXEL GENERATION //

		R <= pixel ? fg_color[11:8] : bg_color[11:8];
		G <= pixel ? fg_color[7:4]  : bg_color[7:4];
		B <= pixel ? fg_color[3:0]  : bg_color[3:0];

	end

	////////////////////////

	initial begin
	 pix8 <= 0;
	 char_cell_vram_offset <= 0;
	 bg_color <= 12'h007;
	 fg_color <= 12'hff0;
	 hsync_fetch_cycle <= 0;
	 row_startL <= 0;
	 row_startH <= 0;
	 row_start <= 0;
	end

endmodule

//////////////////////////////////////////////////////
// Video Controller Top
//////////////////////////////////////////////////////

module vidcon(  input pixel_clock,
				output [3:0] R, G, B,
				output hsync_pulse, vsync_pulse,
				output [15:0] VAB,
				input [7:0] VIN );

	///////////////////////////
	// frame timing
	///////////////////////////

	wire [10:0] hcount;
	wire [10:0] vcount;
	wire frame_inc;

	frame_controller fc0(   pixel_clock,
							hcount, vcount,
							hsync_pulse, vsync_pulse,
							hblank_region, vblank_region,
							frame_inc );

	reg [31:0] frame_counter;
	reg [63:0] line_counter;

	always @(posedge frame_inc) begin
		frame_counter <= frame_counter+1;
	end

	always @(posedge hblank_region) begin
		line_counter <= line_counter+1;
	end

	wire blanking_region = (hblank_region||vblank_region);

	///////////////////////////
	// pixel gen
	///////////////////////////

	wire [3:0] lR0, lG0, lB0;
	wire [15:0] lAD0;

	pixgen pg0(	pixel_clock,
				hsync_pulse,
				hcount, vcount,
				lAD0, VIN,
				lR0, lG0, lB0 );

	assign R = blanking_region ? 4'h0 : lR0;
	assign G = blanking_region ? 4'h0 : lG0;
	assign B = blanking_region ? 4'h0 : lB0;

	assign VAB = lAD0; //vlbsel ? lAD0 : lAD1;

	///////////////////////////
	
	initial begin
		frame_counter <= 0;
		line_counter <= 0;
	end

endmodule
