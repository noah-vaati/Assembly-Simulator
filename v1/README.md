Compiling and Running
====================
start.cpp can be compiled using the `make` command.
It can be run by typing `./start test#.o test#.dat`.

A few files are included for testing:

`test1.asm` + `test1.dat`
---------
`test1.asm` is a program that goes through the fibonacci sequence and saving the values to memory.
It does this by saving the first two values (0 & 1), then it saves the sum of the previous two values (which were saved in registers) and saves them to the next spot in data memory. 
This is looped 30 times.
`test1.dat` is an empty file.

`test5.asm` + `test5.dat`
---------
`test5.asm` is a program that searches the data memory for a specific word, specified at address `0x0`. 
If it is found, a branch function moves to the end of the code, where the location is saved to address `0x0`.
Address `0x1` specifies a limit on how many times we can loop back and look at the next word in sequence.
If it is not found before this, -2 is saved to address `0x0`.
`test5.asm` terminates with output `Illegal address 0400 detected with instruction a40c at address 0006` with input `test5.dat` because `r3` did not contain a valid address.
