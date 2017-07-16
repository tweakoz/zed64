module nexys4_top(
	input sys_reset,
	input sys_clk,
	input but_center,
	output [3:0] vgaR, vgaG, vgaB,
	output vgaH, vgaV,
	output vgaHout,
	output pclkout,
	output rgbled1_b, rgbled2_b
	 );

	reg [11:0] mline_hdisp;
	reg [11:0] mline_hsyncstart;
	reg [11:0] mline_hsyncend;
	reg [11:0] mline_htotal;
	reg [0:0] mline_hsyncinvert;
	reg [11:0] mline_vdisp;
	reg [11:0] mline_vsyncstart;
	reg [11:0] mline_vsyncend;
	reg [11:0] mline_vtotal;
	reg [0:0] mline_vsyncinvert;

	wire pixclk_148mhz;
	wire pixclk_108mhz;
	wire pixclk_74mhz;
	wire pixclk_40mhz;
	wire pixclk_36mhz;
	wire pixclk_27mhz;
	wire pixclk_13mhz;
	
	wire act_reset = ! sys_reset;
	
	wire sys_clock_buffered;

IBUFG clkin1_buf
   (.O (sys_clock_buffered),
    .I (sys_clk));

zed_pll_vesa ZEDPLLVESA( 
	sys_clock_buffered,
	pixclk_108mhz, // 108mhz
	pixclk_36mhz, // 36
	pixclk_40mhz, // 40 
	pixclk_27mhz, // 27
	pixclk_13mhz, // 13.5
	act_reset );

zed_pll_hdtv ZEDPLLHDTV( 
	sys_clock_buffered,
	pixclk_148mhz, // 148.5
	pixclk_74mhz, // 74.25
	act_reset );

wire cpuclk = pixclk_148mhz;

///////////////////////////////////
// select pixel clock
///////////////////////////////////

reg [12:0] mode_select; 
wire [1:0] msel2;
assign msel2 = mode_select[1:0];

assign rgbled1_b = msel2[0];
assign rgbled2_b = msel2[1];

// 00 - 74mhz - 720p
// 01 - 108mhz - 1024p
// 10 - 27mhz - ntscp
// 11 - 40mhz - 600p

wire clkmux = msel2[1] ? ( msel2[0] ? pixclk_40mhz : pixclk_27mhz )
                       : ( msel2[0] ? pixclk_108mhz : pixclk_74mhz );

//synthesis attribute CLOCK_SIGNAL of clkmux is "TRUE" 
//synthesis attribute CLOCK_SIGNAL of pclkout is "TRUE" 

assign  pclkout = clkmux;

always @(posedge but_center, posedge act_reset) begin: VIDCON_VTOP_PG_ADDRGEN0
	mode_select <= act_reset ? 0 : mode_select+1;
end

wire [11:0] out_hdisp;
wire [11:0] out_hstart;
wire [11:0] out_hend;
wire [11:0] out_htotal;
wire [0:0] out_hsi;

wire [11:0] out_vdisp;
wire [11:0] out_vstart;
wire [11:0] out_vend;
wire [11:0] out_vtotal;
wire [0:0] out_vsi;

modeline_hdisp rom_hdisp(out_hdisp,msel2);
modeline_hstart rom_hstart(out_hstart,msel2);
modeline_hend rom_hend(out_hend,msel2);
modeline_htot rom_htotal(out_htotal,msel2);
modeline_hsi rom_hsi(out_hsi,msel2);

modeline_vdisp rom_vdisp(out_vdisp,msel2);
modeline_vstart rom_vstart(out_vstart,msel2);
modeline_vend rom_vend(out_vend,msel2);
modeline_vtot rom_vtotal(out_vtotal,msel2);
modeline_vsi rom_vsi(out_vsi,msel2);

wire mline_latch = but_center|act_reset;
always @(negedge mline_latch) begin : MODESELECT2

	mline_hsyncinvert <= out_hsi;
	mline_hdisp <= out_hdisp;
	mline_hsyncstart <= out_hstart;
	mline_hsyncend <= out_hend;
	mline_htotal <= out_htotal;

	mline_vsyncinvert <= out_vsi;
	mline_vdisp <= out_vdisp;
	mline_vsyncstart <= out_vstart;
	mline_vsyncend <= out_vend;
	mline_vtotal <= out_vtotal;

end

///////////////////////////////////
// memmap
// 0x7000-0x7FFF font ram
// 0x8000-0xBFFF attr ram 
// 0xC000-0xFFFF cell ram 
///////////////////////////////////

wire [15:0] cpu_addr;
wire [7:0] cpu_data;

wire [15:0] chip_addr;
wire [7:0] chip_data;

///////////////////////////////////

wire [11:0] font_addrbus;
wire [7:0] font_databus;
chargen CH(chip_addr[11:0],chip_data);

reg [7:0] font_datareg;
always @(posedge clkmux) begin : LOADROW
    font_datareg[7:0] <= font_databus[7:0];
end

///////////////////////////////////
// dpram ports (a: cpu b: hw)

wire [7:0] dpa_out;

DualPortRam dpa(  cpuclk,
                  1'b0, //a_wena,
                  cpu_addr[11:0], //a_addr
                  cpu_data, //a_data,
                  clkmux, //b_clk,
                  1'b0, //b_wena,
                  chip_addr[11:0], //b_addr,
                  font_databus ); //b_data );

///////////////////////////////////
	
wire hwrap;

vidcon V(
    act_reset,
	pclkout,
    mline_hdisp,
    mline_hsyncstart,
    mline_hsyncend,
    mline_htotal,
    mline_hsyncinvert,
    mline_vdisp,
    mline_vsyncstart,
    mline_vsyncend,
    mline_vtotal,
    mline_vsyncinvert,
	chip_addr,
	chip_data,
    vgaR,
    vgaG,
    vgaB,
    vgaH,
    vgaV,
    hwrap
     );

assign vgaHout = vgaH; 
endmodule