zed64
=====

MetroComputer for an fpga.  Retro, modernized, Verilog.

Not sure where this is going to go. It's on a whim.
First thoughts are:

/////////////////////////////////////////////////////////

1. Multicore SMP 6502
     * local ram for fast access
     * Shared mem for communication
     * Bank switching 
     * at least 1 core will be connected to the hsync generator, this would allow it to take the place of a copper
     
2. Modernized VPU inpired by the Vic-II / OCS / SNES
     * Up to 1080p @ 60hz
     * Multiple playfields (with blending)
     * Virtual CharBuffer (for cpu friendly scrolling)

3. Modernized APU - inspired by the sid and various pro synthesizers
     * Digital Modular
     * Wavetable Scanning Oscillators
       * Various A-Rate modulation schemes, AM/RingMod, FM, etc..
       * Loop point modulation
     * Nice filters (SVF)
     * Insert FX - delay/echo, flange, chorus, distortion, etc..
     * Envelopes
     * FUNS (ala Kurzweil VAST)
     * Modulation Matrix
     * Parameter Morphing
     * MIDI ports ?

/////////////////////////////////////////////////////////

Reference FPGA boards:
Digilent Nexys4

/////////////////////////////////////////////////////////
// Building
/////////////////////////////////////////////////////////

1. **./make.py prep**      downloads dependencies, makes build folders, generates verilog
2. **./make.py synver**      re-generates verilog
3. **./make.py synvhd**      re-generates VHDL (not working yet... ;<)
4. **./make.py ise**      launches ise with zed64 project
5. **./make.py sim**      iverilog simulate (generates a png bitmap via iverilogs VPI interface)
6. **./make.py simv**     iverilog simulate (generates a vcd wave file)
7. **..make.py cc65test** simulate cpu (WIP) eventually will be fleshed out to a whole system simulator

After the first time you run make prep, a *zed.build.ini* file will be generated. Edit this to reflect your environment.

