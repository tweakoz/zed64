zed64
=====

MetroComputer for an fpga.  Retro, modernized, Verilog.

Not sure where this is going to go. It's on a whim.
First thoughts are:

/////////////////////////////////////////////////////////

1. Multicore SMP 6502
     > 100mhz (perhaps synchronous with dot clock)
     local ram for fast access
     Shared mem for communication
     Bank switching 
     at least 1 core will be connected to the hsync generator 
       this would allow it to take the place of a copper

2. Modernized Vic-II / Agnus / SNES like hybrid VPU (supporting up to 1080p @ 60hz)
     Multiple playfields (with blending)
     Virtual CharBuffer (for cpu friendly scrolling)

3. Modernized APU - inspired by the sid and various pro synthesizers
     Many voices
     Wavetable Scanning Oscillators
       Various arate modulation schemes, AM/RingMod, FM, etc..
       Loop point modulation
     Nice filters
     Insert FX - delay/echo, flange, chorus, distortion, etc..
     MIDI ports ?

/////////////////////////////////////////////////////////

Reference FPGA boards:
Digilent Nexys4
Digilent Xybo ?

