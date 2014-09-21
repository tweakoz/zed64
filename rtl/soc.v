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

`define USE_M65CO2

module soc( input clk, // 100 mhz input clock
			output [15:0] led, // individ grn leds
			output [6:0] seg, // 7seg cathode
			output [3:0] an, // 7seg anode
			input btnNotCpuReset, // reset button (0==pushed)
			output [3:0] vgaR, vgaG, vgaB,
			output vgaHsync, vgaVsync,
			input RsRx,
			output RsTx );

reg [6:0] seg_reg;
reg [3:0] an_reg;

assign led[15:4] = {12'b0};

wire [15:0] CPU_AB;	// output reg
wire CPU_WE;		// wrte enable (output)
reg IRQ;			// interrupt request
reg NMI;			// non-maskable interrupt request
reg RDY;			// Ready signal. Pauses CPU when RDY=0 

wire reset = ! btnNotCpuReset;

///////////////////////////////////////////////////////////////////////////////// clock, reset, LED
///////////////////////////////////////////////////////////////////////////////

wire clk_1080p; // 1920x1080p@60 (147.500mhz)
wire clk_1024p; // 1280x1024@60 (107.273mhz)
wire clk_98mhz; // (req 100mhz)
wire clk_720p; // 1280x720@60 (73.750mhz)
wire clk_768p; // 1024x768@60 (65.556mhz)
wire clk_480p; // 640x480@60 (31.892mhz)
wire clk_ntscx2; // 14.39 mhz (req 14.1818)

wire clk_locked;

zed_clockgen the_clock( clk,
						clk_1080p,
						clk_1024p,
						clk_98mhz,
						clk_720p,
						clk_768p,
						clk_480p,
						clk_ntscx2,
						clk_reset,
						clk_locked );

assign clk_reset = reset;

reg [63:0] clock_counter;
always @(posedge clk_720p ) begin
   clock_counter <= reset ? 0 : clock_counter+1;
end
reg [63:0] ser_clock_counter;
always @(posedge clk_ntscx2 ) begin
   ser_clock_counter <= reset ? 0 : ser_clock_counter+1;
end

///////////////////////////////////////////////////////////////////////////////

wire cpu_clk = clock_counter[0];//21]; //[0];
wire lcd_clk = clock_counter[17];
wire pix_clk = clk_720p; //clock_counter[0];
wire ser_clk = ser_clock_counter[2];

///////////////////////////////////////////////////////////////////////////////
wire [15:0] VRAM_AB;	// video address bus
wire [7:0] VRAM_DO; 	// video data out

///////////////////////////////////////////////////////////////////////////////// interconnect
///////////////////////////////////////////////////////////////////////////////
wire [15:0] RAM_AB = CPU_AB;
wire [7:0] RAM_DI, RAM_DO;// = CPU_DO;
wire RAM_WE = CPU_WE;

/////////////////////////////////////////////////////////////////////////////////

wire ser_ld_tx_data = 1'b1;
wire ser_tx_enable = 1'b1;
wire ser_tx_empty;
reg [7:0] ser_tx_data = 8'h81;

wire [7:0] ser_rx_data;
wire ser_rx_enable = 1'b0;
wire ser_rx_empty;

uart the_uart (
	reset, // input
	ser_clk, // input
	ser_ld_tx_data, // input
	ser_tx_data, //input
	ser_tx_enable, // input
	RsTx, // output
	ser_tx_empty, // output
	ser_clk, // input
	uld_rx_data, // input
	ser_rx_data, // output
	ser_rx_enable, // input
	RsRx, // input
	ser_rx_empty // output
	);

/////////////////////////////////////////////////////////////////////////////////
// cpu, rom, ram
/////////////////////////////////////////////////////////////////////////////////

`ifdef USE_M65CO2

wire reset_out;
wire ph2o, ph1o;
wire vecpull;
wire bus_ena;
wire sync, ML;
wire [3:0] ram_ena;
wire nwr, noe;
wire [7:0] CPU_D;
wire [3:0] xa;
wire nwp, nwait;
wire nsel, sck, mosi, miso;
wire rdy = reset ? 1'bz : RDY;

M65C02 the_cpu( 
	.nRst(!reset),
	.nRstO(reset_out),
	.ClkIn(cpu_clk),
	.Phi2O(ph2o),
	.Phi1O(ph1o),
	.nSO(0),
	.nNMI(1),//NMI),
	.nIRQ(1),//IRQ),
	.nVP(vecpull), 
	.BE_In(1),//bus_ena),
	.Sync(sync),
	.nML(ML),
	.nCE(ram_ena),
	.RnW(CPU_WE),
	.nWr(nwr),
	.nOE(noe),
	.Rdy(rdy),
	.XA(xa),
	.A(CPU_AB),
	.DB(CPU_D),
	.nWP_In(nwp),
	.nWait(nwait),
	.LED(led[3:0]),
	.nSel(nsel),
	.SCk(sck),
	.MOSI(mosi),
	.MISO(miso) 
); 

//assign rdy = reset ? 1'z : RDY;

//////////////////////////////////////

bijunction bij( cpu_clk,
				!CPU_WE,
				RAM_DO,
				RAM_DI,
				CPU_D );

//////////////////////////////////////

`else

cpu the_cpu( cpu_clk, reset, CPU_AB, CPU_DI, CPU_DO, CPU_WE, vgaHsync, NMI, RDY );

`endif

rwport_byte_sram main_mem( cpu_clk, !CPU_WE, RAM_AB, RAM_DI, RAM_DO,
									pix_clk, VRAM_AB, VRAM_DO );
									
defparam main_mem.ADDRWIDTH = 16;
defparam main_mem.init_file = "rtl/code.txt";

/////////////////////////////////////////////////////////////////////////////////
// HexLed (debug)
/////////////////////////////////////////////////////////////////////////////////

x7seg led_display( lcd_clk, reset, CPU_AB, seg, an );

/////////////////////////////////////////////////////////////////////////////////

always @(posedge reset ) begin
	IRQ <= 0;
	NMI <= 0;
	RDY <= 0;
end

initial begin
	seg_reg <= 7'd0;
	an_reg <= 4'd0;
	ser_tx_data <= 8'd0;
	clock_counter <= 64'd0;
	ser_clock_counter <= 64'd0;
	IRQ <= 0;
	NMI <= 0;
	RDY <= 1;
end

/////////////////////////////////////////////////////////////////////////////////

vidcon the_vidcon( pix_clk, vgaR, vgaG, vgaB, vgaHsync, vgaVsync, VRAM_AB, VRAM_DO );

endmodule
