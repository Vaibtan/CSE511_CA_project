#include "LogFile.hpp"

void memstate(FILE* file, RISCV_cpu* cpu){
    fprintf(file, "Memory location  :   Data\n");
    for(int i=0;i<RISCV_MEM_SIZE;i++){
        fprintf(file, "0x%x     :   0x%x\n", i, cpu->__bus->riscv_mem->rv32i_mem[i]);
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

    fprintf(file, "IF: Instr 0x%x\n", pipe->fetch->instr);
    fprintf(file, "ID: Instr 0x%x\n", pipe->decode->instr);
    fprintf(file, "EX: Instr 0x%x\n", pipe->execute->instr);
    fprintf(file, "MEM: Instr 0x%x\n", pipe->memory->instr);
    fprintf(file, "WB: Instr 0x%x\n", pipe->writeback->instr);
    
    fprintf(file, "\n");
    fprintf(file, "****************************************************************************\n");
}


void logValues(FILE* file, RISCV_cpu *cpu, pipeline* pipe) {

    fprintf(file, "Cycle %d:\n", pipe->cycle);
    fprintf(file, "\n");

    fprintf(file, "CPU Context:\n");

    RegValues(file, cpu);

    pipelineValues(file, cpu, pipe);
}
