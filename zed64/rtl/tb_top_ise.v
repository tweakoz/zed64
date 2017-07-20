///////////////////////////////////////////////////////////////////////////////
// Zed64
// Copyright (C) 2015-2017 - Michael T. Mayers (michael@tweakoz.com)
// Licensed under the GPL (v3)
//  see https://www.gnu.org/licenses/gpl-3.0.txt
///////////////////////////////////////////////////////////////////////////////

`timescale 1ns/1ns

module tb_top_ise;

reg sys_reset;
reg sys_clk;
reg but_center;
wire [3:0] vgaR, vgaG, vgaB;
wire vgaH, vgaV;
wire vgaHout;
wire vgaVout;
wire rgbled1_b, rgbled2_b;

nexys4_top n4t( sys_reset,sys_clk,but_center,
                vgaR,vgaG,vgaB,vgaH,vgaV,
                vgaHout,vgaVout,rgbled1_b,rgbled2_b );

///////////////////////////////////////////////////

initial begin

`ifdef DO_DUMP 
    #3400
    $display("starting dump");

    $dumpfile("n4.vcd");
    $dumpvars(0,sys_clk);
    $dumpvars(0,sys_reset);
    $dumpvars(0,but_center);
    $dumpvars(0,n4t.act_reset);
    $dumpvars(0,n4t.pixclk_13mhz);
    $dumpvars(0,n4t.pixclk_27mhz);
    $dumpvars(0,n4t.pixclk_36mhz);
    $dumpvars(0,n4t.pixclk_40mhz);
    $dumpvars(0,n4t.pixclk_74mhz);
    $dumpvars(0,n4t.pixclk_108mhz);
    $dumpvars(0,n4t.pclkout);
    $dumpvars(0,n4t.msel2);
    $dumpvars(0,n4t.V);
    $dumpvars(0,vgaR);
    $dumpvars(0,vgaH);
    $dumpvars(0,n4t.font_addrbus);
    $dumpvars(0,n4t.font_databus);
    $dumpvars(0,n4t.is_linestart);
`endif

end

initial begin
    sys_reset = 1;
    sys_clk = 0;
    but_center = 0;

    $display("begin vidcon sim");

    #40
    sys_reset = 0;

	#50
	sys_reset = 1;

end

always # 5 sys_clk = ~sys_clk; //100 mhz

`ifdef DO_VPI 

initial begin
    $display("doing VPI SIM!");
end

always @(posedge n4t.V.vblank) begin: ONVGAV
    $finish;
end

always @(posedge n4t.pclkout) begin: ONPCLK
    $vpi_zed(n4t.chip_addr,n4t.chip_data,n4t.mline_hdisp,n4t.is_blank,vgaH,vgaV,vgaR,vgaG,vgaB);
end

`else 

initial begin
    #14400
    $finish;
end
`endif

endmodule
