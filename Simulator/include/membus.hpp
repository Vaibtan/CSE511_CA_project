#ifndef MEMBUS_H
#define MEMBUS_H
#include "utils.hpp"
#include "memory.hpp"

struct MEM_BUS {
    struct RISC_mem* riscv_mem;
};

u32 mem_bus_ld(MEM_BUS* __bus, u32 addr, u32 size);
void mem_bus_st(MEM_BUS* __bus, u32 addr, u32 size, u32 value);
MEM_BUS* mem_bus_init(RISC_mem* mem);

#endif