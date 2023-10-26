all:
	python3 Assembler/Assembler.py
	python3 Assembler/Assembler_Evaluator.py
	cd Simulator && $(MAKE) clean
	cd Simulator && $(MAKE) all 
clean:
	cd Simulator && $(MAKE) clean
	rm -f Output/Output.bin
	rm -f Assembler/Result/Assembler_Evaluation.txt