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
    #0
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
    $dumpvars(0,n4t.cpu_clk);
    $dumpvars(0,n4t.cpu_wr);
    $dumpvars(0,n4t.cpu_addr);
    $dumpvars(0,n4t.cpu_data);
    $dumpvars(0,n4t.cpu_datar);
    $dumpvars(0,n4t.cpu_dataw);
    $dumpvars(0,n4t.CPU.PC);
    $dumpvars(0,n4t.CPU.PC_temp);
    $dumpvars(0,n4t.CPU.PC_inc);
    $dumpvars(0,n4t.CPU.IR);
    $dumpvars(0,n4t.CPU.IRHOLD);
    $dumpvars(0,n4t.CPU.DI);
    $dumpvars(0,n4t.CPU.DIMUX);
    $dumpvars(0,n4t.CPU.A);
    $dumpvars(0,n4t.CPU.X);
    $dumpvars(0,n4t.CPU.Y);
    $dumpvars(0,n4t.CPU.state);
    $dumpvars(0,n4t.CPU.statename);
    $dumpvars(0,vgaR);
    $dumpvars(0,vgaH);
    $dumpvars(0,n4t.is_linestart);
`endif

end

initial begin
    sys_reset = 1;
    sys_clk = 0;
    but_center = 0;

    $display("begin vidcon sim");

    n4t.dpa.mem[16'h0200] <= 8'ha2; // ldx
    n4t.dpa.mem[16'h0201] <= 8'h00; // #$00
    n4t.dpa.mem[16'h0202] <= 8'ha9; // lda
    n4t.dpa.mem[16'h0203] <= 8'h00; // #$00
    n4t.dpa.mem[16'h0204] <= 8'hc6; // dec
    n4t.dpa.mem[16'h0205] <= 8'hff; // $ff
    n4t.dpa.mem[16'h0206] <= 8'h4c; // jmp
    n4t.dpa.mem[16'h0207] <= 8'h00; // $00
    n4t.dpa.mem[16'h0208] <= 8'h02; //  00
    n4t.dpa.mem[16'h0100] <= 23; // w
    n4t.dpa.mem[16'h0111] <= 15; // o
    n4t.dpa.mem[16'h0112] <= 18; // r
    n4t.dpa.mem[16'h0113] <= 12; // l
    n4t.dpa.mem[16'h0114] <= 4; // d

    n4t.dpa.mem[16'h00ff] <= 8'h80; // d

    n4t.dpa.mem[16'hfffa] <= 0; // NMI
    n4t.dpa.mem[16'hfffb] <= 0; // NMI
    n4t.dpa.mem[16'hfffc] <= 8'h00; // RESET
    n4t.dpa.mem[16'hfffd] <= 8'h02; // RESET
    n4t.dpa.mem[16'hfffe] <= 8'h00; // IRQ/BRK
    n4t.dpa.mem[16'hffff] <= 8'h04; // IRQ/BRK

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
