#ifndef CPU_H
#define CPU_H
#include "utils.hpp"
#include "memory.hpp"

#define REG_LEN 32

typedef struct MEM_BUS {
    struct RISC_mem riscv_mem;
} MEM_BUS;


struct RISCV_cpu{
    u32 x[REG_LEN];
    u32 pc;
    struct MEM_BUS __bus;
};

uint64_t mem_bus_ld(MEM_BUS* __bus, u64 addr, u64 size);
void mem_bus_st(MEM_BUS* __bus, u64 addr, u64 size, u64 value);

/**
class RISCV_cpu{
private:

protected:
    RISC_mem *memrx__;
    u32 x[REG_LEN]{};
    u32 pc{};

public:
    explicit RISCV_cpu(RISC_mem *mem) { this->memrx__ = mem; }
    void reset_cpu();
    u32 get_pc() const { return pc; }
    void init(u32 pc_strt) { this->pc = pc_strt; }
    void single_step();
};
*/

#endif