#include "membus.hpp"

u32 data_bus_ld(DATA_MEM_BUS* _bus, u32 addr, u32 size){
    return d_mem_ld((_bus->data_mem), addr,size);
}

void data_bus_st(DATA_MEM_BUS* _bus, u32 addr, u32 size, u32 value){
    d_mem_st((_bus->data_mem), addr, size, value);
}

DATA_MEM_BUS* data_bus_init(DATA_mem* mem){
    DATA_MEM_BUS* _bus = (DATA_MEM_BUS*)malloc(sizeof(DATA_MEM_BUS));
    _bus->data_mem = mem;
    return _bus;
}

