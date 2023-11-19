#include "utils.hpp"
#include "memory.hpp"

u32 d_mem_ld_8(DATA_mem* mem, u32 addr){
    return (u32) mem->rv32i_data_mem[addr - RISCV_MEM_BASE];
}
u32 d_mem_ld_16(DATA_mem* mem, u32 addr){
    return (u32) mem->rv32i_data_mem[addr - RISCV_MEM_BASE]
        |  (u32) mem->rv32i_data_mem[addr - RISCV_MEM_BASE + 1] << 8;
}
u32 d_mem_ld_32(DATA_mem* mem, u32 addr){
    return (u32) mem->rv32i_data_mem[addr - RISCV_MEM_BASE]
        |  (u32) mem->rv32i_data_mem[addr - RISCV_MEM_BASE + 1] << 8
        |  (u32) mem->rv32i_data_mem[addr - RISCV_MEM_BASE + 2] << 16 
        |  (u32) mem->rv32i_data_mem[addr - RISCV_MEM_BASE + 3] << 24;
}

u32 d_mem_ld(DATA_mem* mem, u32 addr, u32 sz_) {
    switch (sz_) {
        case 8:  return d_mem_ld_8(mem, addr);  break;
        case 16: return d_mem_ld_16(mem, addr); break;
        case 32: return d_mem_ld_32(mem, addr); break;
        default: ;
    }
    return 1;
}


void d_mem_st_8(DATA_mem* mem, u64 addr, u64 value) {
    mem->rv32i_data_mem[addr - RISCV_MEM_BASE] = (u8) (value & 0xff);
}

void mem_st_16(RISC_mem* mem, u64 addr, u64 value) {
    mem->rv32i_mem[addr - RISCV_MEM_BASE] = (u8) (value & 0xff);
    mem->rv32i_mem[addr - RISCV_MEM_BASE + 1] = (u8) ((value >> 8) & 0xff);
}

void mem_st_32(RISC_mem* mem, u32 addr, u64 value) {
    mem->rv32i_mem[addr - RISCV_MEM_BASE] = (u8) (value & 0xff);
    mem->rv32i_mem[addr - RISCV_MEM_BASE + 1] = (u8) ((value >> 8) & 0xff);
    mem->rv32i_mem[addr - RISCV_MEM_BASE + 2] = (u8) ((value >> 16) & 0xff);
    mem->rv32i_mem[addr - RISCV_MEM_BASE + 3] = (u8) ((value >> 24) & 0xff);
}

void mem_st(RISC_mem* mem, u32 addr, u32 sz_, u32 value) {
    switch (sz_) {
        case 8:  mem_st_8(mem, addr, value);  break;
        case 16: mem_st_16(mem, addr, value); break;
        case 32: mem_st_32(mem, addr, value); break;
        default: ;
    }
}

DATA_mem *data_init(){
    RISC_mem *mem = (RISC_mem*)malloc(sizeof(DATA_mem));
    if (mem == NULL) {
        fprintf(stderr, "[-] ERROR-> mem_init : malloc failed\n");
        exit(1);
    }
    return mem;
}

/**
RV32i_mem::RV32i_mem(size_t _sz){
    dat_x = new char[_sz];
}
RV32i_mem::~RV32i_mem(){
    delete[] dat_x;
}
*/