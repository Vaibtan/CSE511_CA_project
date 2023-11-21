   
============
RISC V-CPU
============

This is a cycle accurate simlator for a 5 staged pipelined RISC-V CPU.

1) How to run?

	a) Write the assembly testcases Input/Input.txt and the corresponding binary output in Input/binary.txt.
	b)To run this code, open the terminal in the repository directory and type command 'make' (For Linux Environment)
	c) The binary output is generated in Assembler/Output/Output.bin
	d) The assembler is evaluated in the file Assembler/Result/Assembler_Evaluation.txt
	e) Logs Generated from simulator are placed in logs/logfile.log
	f) Plot figures will be generated in figure folder


2) File Description:

=> There are 2 files to implement the assembler and 9 files to implement the simulator.
=> The assembler files are "Assembler.py" and "Assembler_Evaluator.py"
=> The simulator files are "ALU.cpp", "LRU_Cache.cpp",  "LogFile.cpp", "cache_bus.cpp", "cpu.cpp", "membus.cpp", "memory.cpp", "pipeline.cpp", "simulator.cpp"

2) Code Overview:

=> Assembler.py : 		This file converts the assembly code in Input/Input.txt to binary code in Assembler/Output/Output.bin
=> Assembler_Evaluator.py: 	This file checks the output generated from Assembler.py by comparing it against Input/binary.txt and stores the result in Assembler/Result/Assembler_Evaluation.txt

=> ALU.cpp : This file contains implementaion of Alu for the cpu
=> LRU_Cache.cpp : This file contains the raw implementation of the Least Replacement Policy cache for cpu
=> LogFile.cpp : This file contains the implementation of the log file writing component of the simulator
=> cache_bus.cpp : This file is supposed to have cache bus for cpu
=> cpu.cpp : This file contains  the raw cpu
=> membus.cpp : This file  contains the memory bus, which connects memory to cache
=> memory.cpp : THis file conatains the implmentation of the memory
=> pipeline.cpp : This file contains the buffers and logic for implmentation of cycle accurate simulator
=> simulator.cpp : This is main file for the simulator
