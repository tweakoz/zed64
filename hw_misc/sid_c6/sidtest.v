`timescale 1ns / 1ps

module sidtest_top(
	input C6_CLK_8MHZ,
	input BTN_0,
	input BTN_1,
	output [7:0] SID_DATA,
	output [4:0] SID_ADDR,
	output SID_NOTRES,
	output SID_CLK,
	output SID_NOTCS,
	output [3:0] C6_LED
    );

//////////////////////////////////////////////////

reg [7:0] sid_data;
reg [4:0] sid_addr;
reg sid_cs;
reg sid_reset;

//////////////////////////////////////////////////

reg [31:0] clk_counter;

always @(posedge C6_CLK_8MHZ)
	clk_counter <= clk_counter+1;

//////////////////////////////////////////////////

assign sid_clock = clk_counter[2];
reg [31:0] sid_cycle_counter;

always @(posedge sid_clock) // 1mhz
	sid_cycle_counter <= sid_reset ? 0 : sid_cycle_counter+1;	

//////////////////////////////////////////////////

//wire [15:0] sid_subcyc = sid_cycle_counter[15:0]; // 1mhz (1/16sec)
//wire [15:0] sid_subcyc = sid_cycle_counter[19:4]; // 1mhz/16 == 64Khz (1 sec loop)
wire [15:0] sid_subcyc = sid_cycle_counter[20:5]; // 1mhz/32 == 32Khz (2 sec loop)
//wire [15:0] sid_subcyc = sid_cycle_counter[21:6]; // 1m/64 == 16Khz (4sec loop)
//wire [7:0] sid_subcyc = sid_cycle_counter[21:14]; // 1m/256 == 4Khz (16sec loop)
//wire [15:0] sid_subcyc = sid_cycle_counter[27:20]; // 1m/1m == 1hz

assign C6_LED[3:0] = sid_subcyc[15:12];

reg [15:0] frq0;

reg [15:0] fm0_frq, fm1_frq, pwm_frq, swp0_frq, swp1_frq, swp2_frq, swp3_frq, wavs_frq;
reg [39:0] fm0_phase, fm1_phase, pwm_phase, swp0_phase, swp1_phase, swp2_phase, swp3_phase, wavs_phase;

reg [11:0] pulse_width;
reg [10:0] fcutoff;

always @(posedge sid_clock) begin

	//////////////////////////////////////

	sid_reset <= BTN_0;

	//////////////////////////////////////
	// set frqs
	//////////////////////////////////////
	
	swp0_frq = 16'h0001;
	swp1_frq = 16'h0003;
	swp2_frq = 16'h0119;
	swp3_frq = 16'h0017;

	fm0_frq<=(swp0_phase[32:17]);
	fm1_frq <= (swp1_phase[38:23]);
	pwm_frq <= 16'h001;

	pulse_width <= pwm_phase[23:8];

	//frq0<=16'h0735+(fm0_phase[23:8]);
	case(fm0_phase[19:16])
		4'h0:	frq0<=16'd0125;
		4'h1:	frq0<=16'd0250;
		4'h2:	frq0<=16'd0500;
		4'h3:	frq0<=16'd0500;
		4'h4:	frq0<=16'd1000;
		4'h5:	frq0<=16'd1000;
		4'h6:	frq0<=16'd2000;
		4'h7:	frq0<=16'd4000;
		4'h8:	frq0<=16'd1000;
		4'h9:	frq0<=16'd4000;
		4'ha:	frq0<=16'd2000;
		4'hb:	frq0<=16'd1000;
		4'hc:	frq0<=16'd1000;
		4'hd:	frq0<=16'd0500;
		4'he:	frq0<=16'd0250;
		4'hf:	frq0<=16'd0125;
		endcase
	
	wavs_frq<=16'h0923+(fm1_phase[23:8]>>5);

	case(swp1_phase[26:24])
		3'd0: fcutoff <= swp1_phase[25:16];
		3'd1: fcutoff <= swp1_phase[22:15];
		3'd2: fcutoff <= swp1_phase[22:14];
		3'd3: fcutoff <= swp1_phase[21:13];
		3'd4: fcutoff <= swp1_phase[20:12];
		3'd5: fcutoff <= swp1_phase[19:10];
		3'd6: fcutoff <= swp1_phase[18:8];
		3'd7: fcutoff <= swp1_phase[13:5];
	endcase
	
	//////////////////////////////////////
	// integrate phasors
	//////////////////////////////////////

	fm0_phase <= sid_reset ? 0 : fm0_phase+{24'd0,fm0_frq};
	fm1_phase <= sid_reset ? 0 : fm1_phase+{24'd0,fm1_frq};
	wavs_phase <= sid_reset ? 0 : (wavs_phase+{24'd0,wavs_frq});
	pwm_phase <= sid_reset ? 0 : (pwm_phase+{24'd0,pwm_frq});
	swp0_phase<=sid_reset ? 0 : (swp0_phase+{24'd0,swp0_frq});
	swp1_phase<=sid_reset ? 40'h00000 : (swp1_phase+{24'd0,swp1_frq});
	swp2_phase<=sid_reset ? 0 : (swp2_phase+{24'd0,swp2_frq});
	swp3_phase<=sid_reset ? 0 : (swp3_phase+{24'd0,swp3_frq});

	//////////////////////////////////////

	sid_cs<=(sid_subcyc[0]); // odd cycles chipselect
	//sid_addr<=sid_subcyc[4:0]; // master volume == 15
	//sid_data<=sid_subcyc[7:0];
	
	//////////////////////////////////////
	if( sid_subcyc == 16'h0012 ) begin
			sid_addr<=5'h18; // master volume == 15
			sid_data<=8'h1f;
			end
	else if( sid_subcyc == 16'h14 ) begin
			sid_addr<=5'h0;
			sid_data<=8'h0;	// fLO = FF
			end
	else if( sid_subcyc == 16'h0016 ) begin
			sid_addr<=5'h1;
			sid_data<=8'h0;	// fHI = 1f
			end
	else if( sid_subcyc == 16'h18 ) begin
			sid_addr<=5'h5;
			sid_data<=8'h0f;	// AD = FF
			end
	else if( sid_subcyc == 16'h001a ) begin
			sid_addr<=5'h6;
			sid_data<=8'hff;	// SR = FF
			end
	else if( sid_subcyc == 16'h001c ) begin
			sid_addr<=5'h2;
			sid_data<=8'h7f;  // pwlo
			end
	else if( sid_subcyc == 16'h001e ) begin
			sid_addr<=5'h3;
			//sid_data<=8'h7f;  // pwlo
			end
	else if( sid_subcyc == 16'h0020 ) begin
			sid_addr<=5'h4;
			sid_data<=8'h11;	// gate on (tri)
			end
	else if( sid_subcyc == 12'h22 ) begin
			sid_addr<=5'h17;
			sid_data<=8'hff;  // reson=15, filter all
			end
	else if( sid_subcyc == 16'h0022 ) begin
			sid_addr<=5'h18;
			sid_data<=8'hf;  // gate off(tri)
			end
	else if( sid_subcyc == 16'hf000 ) begin
			sid_addr<=5'h4;
			sid_data<=8'h10;  // gate off(tri)
			end
	else if( sid_subcyc == 16'hf002 ) begin
			sid_addr<=5'h18;
			sid_data<=8'h1f;  // gate off(tri)
			end
	//////////////////////////////////////////
	// arate voice update
	//////////////////////////////////////////
	else if( sid_subcyc > 16'h0034 ) begin
		case(sid_subcyc[2:0])
				/////////////////////////////////////////
				// frq LO
				/////////////////////////////////////////
				3'd0: begin
					sid_addr<=5'h0;
					sid_data<=frq0[7:0];  
					end
				/////////////////////////////////////////
				// frq HI
				/////////////////////////////////////////
				3'd2: begin
					sid_addr<=5'h1;
					sid_data<=frq0[15:8];
					end
				/////////////////////////////////////////
				// fc LO
				/////////////////////////////////////////
				3'd4: begin 
					sid_addr<=5'h15;
					sid_data<=fcutoff[2:0];
					end
				/////////////////////////////////////////
				// fc HI
				/////////////////////////////////////////
				3'd6: begin 
					sid_addr<=5'h16;
					sid_data<=fcutoff[10:3];
					end
				/////////////////////////////////////////
				// fc HI
				/////////////////////////////////////////
				//3'd7: begin 
					//sid_addr<=5'h4;
					//sid_data<=BTN_1 ? 8'h20 : 8'h10;  // gate off(tri)
					//end
				/////////////////////////////////////////
				// pwmod LO
				/////////////////////////////////////////
				//3'd2: begin 
				//	sid_addr<=5'h2;
				//	sid_data<=pulse_width[7:0];
				//	end
				/////////////////////////////////////////
				// pwmod HI
				/////////////////////////////////////////
				//3'd3: begin 
				//	sid_addr<=5'h3;
				//	sid_data<=pulse_width[11:8];
				//	end
				/////////////////////////////////////////
				// waveform
				/////////////////////////////////////////
				//3'd4: begin
					//sid_addr<=5'h4;
					//sid_data<=(wavs_phase[23]) ? 8'h10 : 8'h20;
					//end
				endcase
			end
			//auxo_frq<=16'h1000;
	//////////////////////////////////////////			
	
end // always @(posedge sid_clock)
	
assign SID_NOTRES = (sid_reset==0);
assign SID_NOTCS = (sid_cs==0);
assign SID_ADDR[4:0] = sid_addr[4:0];
assign SID_DATA[7:0] = sid_data[7:0];
assign SID_CLK = sid_clock;

//////////////////////////////////////////////////

initial begin
	sid_cycle_counter <= 0;
	clk_counter <=0;
	fm0_phase<=0;
	fm1_phase<=0;
	pwm_phase<=0;
  wavs_phase<=0;
  swp0_phase<=0;
  swp1_phase<=0;
  swp2_phase<=0;
  swp3_phase<=0;
  fcutoff <= 0;
	end


endmodule
