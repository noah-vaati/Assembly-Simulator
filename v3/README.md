# Noah Redden, 7841009

# About

This version adds a simulated hard drive using DMA. This takes the form of a file called `hd`. If there is no file called `hd`, it is generated at run time.

 # Compiling and Running

Using the `make` command will compile files into executable `simulator`.
Can be run using `./simulator (compiled asm) (dat file)`

`make clean` will remove the compiled simulator.


# Sample Program

Sample Program `testA3.asm` and accompanying `testA3.dat` are provided.
The program initializes by moving values 0, 1, 2 to registers `r1`,`r2`,`r3`, respectively, to be used in DMA transfers.
This program first writes to the hard drive the first 5 words (Except the first 3 which are reserved) in the data memory. 
This will use two interupts to acomplish, since burst is set to 4 words.

Then, the program reads from address 24 on the hard drive 5 words and writes them to memory address 5.
Assuming `testA.dat` is used, this will write words `FFFF` to the memory. 
Again, this will use two interrupts to acomplish.

The program finally does a polling check on the status register.
This is done with a branch that loops to check if the status register equals 0.
If we get greater than 0, we loop. Otherwise, program reaches the end and ungracefully terminates.
