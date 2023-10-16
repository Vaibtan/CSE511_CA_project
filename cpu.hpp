#ifndef CPU_H
#define CPU_H

#include "instruction.hpp"
#include "pipeline.hpp"
#include "membus.hpp"
#include "ALU.hpp"

#define REG_LEN 32

struct RISCV_cpu{
    u32 x[REG_LEN];
    u32 pc;
    struct MEM_BUS* __bus;
    struct pipeline* __pipe;
    ALU* __alu;
};

RISCV_cpu* CPU_init();
void CPU_reset(RISCV_cpu *cpu);


void cpu_fetch(RISCV_cpu *cpu);
void cpu_decode(RISCV_cpu *cpu);
void cpu_execute(RISCV_cpu *cpu);
void cpu_memory(RISCV_cpu *cpu);
void cpu_writeback(RISCV_cpu*cpu);
void cpu_pc_update(RISCV_cpu*cpu);

#endif