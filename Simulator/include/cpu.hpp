#ifndef CPU_H
#define CPU_H

#include "utils.hpp"
#include "instruction.hpp"
#include "pipeline.hpp"
#include "membus.hpp"
#include "ALU.hpp"

#define REG_LEN 32
#define MM_REG_LEN 5
#define MM_BASE_ADDR 0x10000 // 65536 (since mem is 64kb)

struct RISCV_cpu{
    u32 x[REG_LEN];
    u32 mem_map_reg[MM_REG_LEN];
    u32 pc;
    struct MEM_BUS* __bus;
    struct pipeline* __pipe;
    ALU* __alu;
    int memory_instr_counter;
    int register_instr_counter;
};

RISCV_cpu* CPU_init();
void CPU_reset(RISCV_cpu *cpu);


void cpu_fetch(RISCV_cpu *cpu,u32 new_pc,bool early_exit);
void cpu_decode(RISCV_cpu *cpu);
void cpu_execute(RISCV_cpu *cpu);
void cpu_memory(RISCV_cpu *cpu);
void cpu_writeback(RISCV_cpu*cpu);
u32 cpu_pc_update(RISCV_cpu*cpu);
//needed for initiall loading of instructions
void cpu_st(RISCV_cpu* cpu, u32 addr, u32 size, u32 value);

#endif
