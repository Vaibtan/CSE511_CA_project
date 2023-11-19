#include "membus.hpp"

u32 data_bus_ld(DATA_MEM_BUS* _bus, u32 addr, u32 size){
    return d_mem_ld((_bus->data_mem), addr,size);
}

void data_bus_st(DATA_MEM_BUS* _bus, u32 addr, u32 size, u32 value){
    d_mem_st((_bus->data_mem), addr, size, value);
}

u32 instr_bus_ld(INSTR_MEM_BUS* _bus, u32 addr, u32 size){
    return i_mem_ld(_bus->instr_mem,addr,size);
}
void instr_bus_st(INSTR_MEM_BUS* _bus, u32 addr, u32 size, u32 value){
    i_mem_st(_bus->instr_mem,addr,size,value);
}
DATA_MEM_BUS* data_bus_init(DATA_mem* mem){
    DATA_MEM_BUS* _bus = (DATA_MEM_BUS*)malloc(sizeof(DATA_MEM_BUS));
    _bus->data_mem = mem;
    return _bus;
}

INSTR_MEM_BUS* instr_bus_init(INSTR_mem* mem){
    INSTR_MEM_BUS* _bus = (INSTR_MEM_BUS*)malloc(sizeof(INSTR_MEM_BUS));
    _bus->instr_mem = mem;
    return _bus;
}

MEM_BUS* mem_bus_init(DATA_MEM_BUS* d_bus,INSTR_MEM_BUS* i_bus){
    MEM_BUS* __bus = (MEM_BUS*)malloc(sizeof(MEM_BUS));
    __bus->data_bus = d_bus;
    __bus->instr_bus = i_bus;
    return __bus;
}
