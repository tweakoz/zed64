///////////////////////////////////////////////////////////////////////////////
// Zed64
// Copyright (C) 2015-2017 - Michael T. Mayers (michael@tweakoz.com)
// Licensed under the GPL (v3)
//  see https://www.gnu.org/licenses/gpl-3.0.txt
///////////////////////////////////////////////////////////////////////////////

module vidcon (
    input reset,
    input pixel_clock,
    input [11:0] hdisp,
    input [11:0] hsyncstart,
    input [11:0] hsyncend,
    input [11:0] htotal,
    input hsyncinvert,
    input [11:0] vdisp,
    input [11:0] vsyncstart,
    input [11:0] vsyncend,
    input [11:0] vtotal,
    input vsyncinvert,
    output reg [15:0] vram_adr_out,
    input [7:0] vram_dat_in,
    output [3:0] out_red,
    output [3:0] out_grn,
    output [3:0] out_blu,
    output out_hs,
    output out_vs,
    output out_isblank,
    output out_linestart
);

/////////////////////////////////////////////////////////////////
// frame processor
/////////////////////////////////////////////////////////////////

reg [11:0] hcount;
reg [11:0] hcountS1;
reg [11:0] hcountS2;
reg [11:0] hcountS3;
reg [11:0] hcountS4;
reg [11:0] hcountS5;
reg [11:0] hcountS6;
reg [11:0] hcountS7;
reg [11:0] hcountS8;

reg hwrap;
wire [11:0] hcountP1 = (hcount + 1);
reg [11:0] hcountPW;

wire ipixel_clock = ! pixel_clock;

always @(negedge pixel_clock, posedge reset) begin: FRAME_HGEN
    hwrap <= reset ? 1 : (hcountPW == htotal);
    hcountPW <= reset ? 0 : (hcount+3);
    hcount <= (reset|hwrap) ? 0 : hcountP1;
    hcountS1 <= reset ? 0 : hcount;
    hcountS2 <= reset ? 0 : hcountS1;
    hcountS3 <= reset ? 0 : hcountS2;
    hcountS4 <= reset ? 0 : hcountS3;
    hcountS5 <= reset ? 0 : hcountS4;
    hcountS6 <= reset ? 0 : hcountS5;
    hcountS7 <= reset ? 0 : hcountS6;
    hcountS8 <= reset ? 0 : hcountS7;
end

wire phase0 = ((hcount&7)==0)&&(hcount<hdisp)&&ipixel_clock;
wire phase1 = ((hcountS1&7)==0)&&(hcountS1<hdisp)&&ipixel_clock;
wire phase2 = ((hcountS2&7)==0)&&(hcountS2<hdisp)&&ipixel_clock;
wire phase3 = ((hcountS3&7)==0)&&(hcountS3<hdisp)&&ipixel_clock;
wire phase4 = ((hcountS4&7)==0)&&(hcountS4<hdisp)&&ipixel_clock;
wire phase5 = ((hcountS5&7)==0)&&(hcountS5<hdisp)&&ipixel_clock;
wire fphase5 = ((hcountS5&7)==0)&&(hcountS5<hdisp);
wire phase6 = ((hcountS6&7)==0)&&(hcountS6<hdisp)&&ipixel_clock;
wire phase6b = ((hcountS6&7)==0)&&(hcountS6<hdisp)&&pixel_clock;
wire phase7 = ((hcountS7&7)==0)&&(hcountS7<hdisp)&&ipixel_clock;
assign out_linestart = ((hcount)==1);

reg [11:0] vcount;
wire [11:0] fc_vcp1 = (vcount + 1);
wire [11:0] fc_vcp2 = (vcount + 2);
reg vwrap;
wire [11:0] fc_get_vc = vwrap ? 0 : fc_vcp1;

always @(posedge hwrap, posedge reset) begin: FRAME_VGEN
    vwrap <= reset ? 0 : (fc_vcp2 == vtotal);
end

always @(negedge out_linestart, posedge reset) begin: FRAME_VGEN2
    vcount <= reset ? 12'b111111111111 : fc_get_vc;
end

wire [11:0] hcountPulse = (hcountS8 + 1);
wire hpulse = ((hcountPulse > hsyncstart) & (hcountPulse <= hsyncend)) ^ hsyncinvert;
wire vpulse = ((fc_vcp2 >= vsyncstart) & (fc_vcp2 < vsyncend)) ^ vsyncinvert;
wire hblank = (hcountS8 >= hdisp) & (hcountS8 <= htotal);
wire vblank = (vcount > vdisp) & (vcount <= vtotal);
wire is_blanking = hblank | vblank;

assign out_isblank = is_blanking;

/////////////////////////////////////////////////////////////////
// pixel processor
/////////////////////////////////////////////////////////////////

wire [2:0] pg_pixbit = 7-hcountS7[2:0];
reg [7:0] charcell;
reg [7:0] pg_pix8;
reg character_pixel;
reg [11:0] charcell_addr;
wire [2:0] cell_row = vcount[2:0];

always @(negedge phase4, posedge reset) begin: CHARCELL_ADDRGEN
    charcell_addr <= reset ? 0 : hdisp*(vcount/8)+(hcount/8);
end

wire p5edge = phase5 | phase7;

always @(posedge p5edge, posedge reset) begin: CHARCELL_ADDROUT
    vram_adr_out <= reset ? 0 
                          : fphase5 ? {4'b1000,charcell_addr}
                                    : {5'b01110,charcell[7:0], cell_row};
end
always @(negedge phase6, posedge reset) begin: CHARCELL_READ
    charcell <= (reset|hwrap) ? 0 : vram_dat_in;
end
always @(negedge phase7, posedge reset) begin: FETCH_CHARCELL_ROW
    pg_pix8 <= reset ? 0 : vram_dat_in;
end
always @(negedge pixel_clock) begin: PIXLATCH
    character_pixel <= pg_pix8[pg_pixbit];
end

wire [3:0] pg_R = 4'b1111; //hcountS8[6:3];
wire [3:0] pg_G = vcount[6:3];
wire [3:0] pg_B = {hcountS8[9:8], vcount[9:8]};

wire [3:0] pix_r = character_pixel ? pg_R : 0;
wire [3:0] pix_g = character_pixel ? pg_G : 0;
wire [3:0] pix_b = character_pixel ? pg_B : 0;

/////////////////////////////////////////////////////////////////

// enforce black pixels in blanking regions
assign out_red = is_blanking ? 0 : pix_r;
assign out_grn = is_blanking ? 0 : pix_g;
assign out_blu = is_blanking ? 0 : pix_b;

assign out_hs = hpulse;
assign out_vs = vpulse;

endmodule
