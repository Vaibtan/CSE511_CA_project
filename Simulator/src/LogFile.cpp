#include "LogFile.hpp"

void memstate(FILE* file, RISCV_cpu* cpu, pipeline* pipe) {
    fprintf(file, "Memory location  :   Data\n");
    mem_unit* mem_stage = pipe->memory;

    // Check if the program has completed (memory stage is done)
    if (mem_stage->done) {
        for (int i = 0; i < RISCV_MEM_SIZE; i++) {
            fprintf(file, "0x%x     :   0x%x\n", i, cpu->__bus->riscv_mem->rv32i_mem[i]);
        }

        // Print memory information for the specific address and data in the memory stage
        fprintf(file, "\nMemory Stage Information:\n");
        fprintf(file, "Address: 0x%x\n", mem_stage->addr);
        fprintf(file, "Data   : 0x%x\n", mem_stage->value);
    }
}



void RegValues(FILE* file, RISCV_cpu* cpu){
    fprintf(file, "PC: 0x%x\n", cpu->pc);
    fprintf(file, "The current value of registers:\n");
    for(int i=0;i<REG_LEN;i++){
        fprintf(file, "Register %d: 0x%x\n", i, cpu->x[i]);
    }
}

void pipelineValues(FILE* file, RISCV_cpu *cpu, pipeline* pipe) {
    
    fprintf(file, "\n");
    fprintf(file, "Pipeline State:\n");

    fprintf(file, "IF: Instr 0x%x\n", pipe->fetch->inst);
    fprintf(file, "ID: Instr 0x%x\n", pipe->decode->inst);
    fprintf(file, "EX: Instr 0x%x\n", pipe->execute->inst);
    fprintf(file, "MEM: Instr 0x%x\n", pipe->memory->inst);
    fprintf(file, "WB: Instr 0x%x\n", pipe->writeback->inst);
    
    fprintf(file, "\n");
    fprintf(file, "****************************************************************************\n");
}

void isStall(FILE* file, RISCV_cpu *cpu, pipeline* pipe){

    fprintf(file, "\nStall Status:\n");

    if (pipe->de_stall) {
        fprintf(file, "Decode stage is stalled\n");
    }

    if (pipe->ex_stall) {
        fprintf(file, "Execute stage is stalled\n");
    }
    fprintf(file, "\n");
}


void logValues(FILE* file, RISCV_cpu *cpu, pipeline* pipe) {

    fprintf(file, "Cycle %d:\n", pipe->cycle);
    fprintf(file, "\n");

    fprintf(file, "CPU Context:\n");

    RegValues(file, cpu);

    pipelineValues(file, cpu, pipe);

    isStall(file, cpu, pipe);
}
