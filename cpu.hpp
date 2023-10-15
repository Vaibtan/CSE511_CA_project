#ifndef CPU_H
#define CPU_H
#include "utils.hpp"
#include "memory.hpp"

#define REG_LEN 32


struct RISCV_cpu{
    u32 x[REG_LEN];
    u32 pc;
};

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