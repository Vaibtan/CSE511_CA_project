#include "cpu.hpp"

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

void logValues(FILE* file, RISCV_cpu *cpu, pipeline* pipe) {
    fprintf(file, "Cycle %d:\n", pipe->cycle);
    fprintf(file, "\n");
    fprintf(file, "CPU Context:\n");
    RegValues(file, cpu);
    fprintf(file, "Memory state:\n");
    memstate(file, cpu);
    fprintf(file, "\n");
    fprintf(file, "****************************************************************************\n");
}
