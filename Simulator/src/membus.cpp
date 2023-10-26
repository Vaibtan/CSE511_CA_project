#include "membus.hpp"

u32 mem_bus_ld(MEM_BUS* __bus, u32 addr, u32 size){
    return mem_ld((__bus->riscv_mem), addr,size);
}

void mem_bus_st(MEM_BUS* __bus, u32 addr, u32 size, u32 value){
    mem_st((__bus->riscv_mem), addr, size, value);
}

MEM_BUS* mem_bus_init(RISC_mem* mem){
    MEM_BUS* __bus = (MEM_BUS*)malloc(sizeof(MEM_BUS));
    __bus->riscv_mem = mem;
    return __bus;
}