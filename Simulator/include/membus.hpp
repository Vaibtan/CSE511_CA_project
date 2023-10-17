#ifndef MEMBUS_H
#define MEMBUS_H
#include "utils.hpp"
#include "memory.hpp"

typedef struct MEM_BUS {
    struct RISC_mem* riscv_mem;
};

uint64_t mem_bus_ld(MEM_BUS* __bus, u64 addr, u64 size);
void mem_bus_st(MEM_BUS* __bus, u64 addr, u64 size, u64 value);
MEM_BUS* mem_bus_init(RISC_mem* mem);

#endif