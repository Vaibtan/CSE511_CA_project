#ifndef MEMORY_H
#define MEMORY_H
#include "utils.hpp"

//add_ptr TAKES AN ADDRESS FROM RISCV MEM_SPACE AND RETURNS A PTR. TO THE POS. OF VALUE IN THE MEM. ;

#define RISCV_MEM_SIZE 1024 * 1024      //MEMORY_SIZE OF 1MB
#define RISCV_MEM_BASE 0x80000000

typedef struct RISC_mem{
    u8 rv32i_mem[RISCV_MEM_SIZE];
} RISC_mem;


u64 mem_ld(RISC_mem* mem, u32 addr, u32 sz_);
void mem_st(RISC_mem* mem, u32 addr, u32 sz_, u32 value);

/**
class RISC_mem {
protected:
    [[nodiscard]] virtual char *addr_ptr(u32 __addr) const = 0;
public:
    virtual u32 re_32(u32 __addr)  {return *(u32*)(addr_ptr(__addr)); }
    virtual u16 re_16(u32 __addr)  {return *((u16*)(addr_ptr(__addr)));}
    virtual u8  re_8(u32 __addr)  {return *addr_ptr(__addr);}

    virtual void wrt_32(u32 val__, u32 __addr)  {*((u32 *)(addr_ptr(__addr))) = val__;};
    virtual void wrt_16(u16 val__, u32 __addr)  { *((u16*)(addr_ptr(__addr))) = val__;};
    virtual void wrt_8(u8 val__, u32 __addr)  {(*addr_ptr(__addr))=val__;};
    virtual u32 operator[] (u32 __addr) const  {return *(u32*)(addr_ptr(__addr));};
    virtual u32& operator[] (u32 __addr)  {return *(u32*)(addr_ptr(__addr)); };
};

class RV32i_mem : public RISC_mem {
private:
    char *dat_x;
protected:
    [[nodiscard]] char *addr_ptr(u32 __addr) const override {return  (dat_x + __addr);}
public:
    explicit RV32i_mem(size_t _sz);
    ~RV32i_mem();
};
*/
#endif