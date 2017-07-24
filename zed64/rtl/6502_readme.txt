6502 verilog model
cloned from https://github.com/Arlet/verilog-6502
(sha a11631082b20e8a0746cd5cc4ae8bbe68bff79c4)

A Verilog HDL version of the old MOS 6502 CPU.

Note: the 6502 core assumes a synchronous memory. This means that valid
data (DI) is expected on the cycle *after* valid address. This allows
direct connection to (Xilinx) block RAMs. When using asynchronous memory,
I suggest registering the address/control lines for glitchless output signals.

Have fun. 
