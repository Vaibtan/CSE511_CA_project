#include "LogFile.hpp"

void memstate(FILE* file, RISCV_cpu* cpu){
    fprintf(file, "Memory location  :   Data\n");
    REP(__i, 0, RISCV_MEM_SIZE){ fprintf(file, "0x%x  :  0x%x\n", __i, cpu->__bus->riscv_mem->rv32i_mem[__i]); }
}

void RegValues(FILE* file, RISCV_cpu* cpu){
    fprintf(file, "PC: 0x%x\n", cpu->pc);
    fprintf(file, "The current value of registers:\n");
    REP(__i, 0, REG_LEN){ fprintf(file, "Register %d: 0x%x\n", __i, cpu->x[__i]); }
}

void logValues(FILE* file, RISCV_cpu *cpu, pipeline* pipe) {
    fprintf(file, "Cycle %d:\n", pipe->cycle);
    fprintf(file, "\n");
    fprintf(file, "CPU Context:\n");
    RegValues(file, cpu);
    // fprintf(file, "Memory state:\n");
    // memstate(file, cpu);
    fprintf(file, "\n");
    fprintf(file, "****************************************************************************\n");
}
