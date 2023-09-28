#include<bits/stdc++.h>
using namespace std;

using u64 = uint64_t;
using u32 = uint32_t;
using i64 = int64_t;
using i32 = int32_t;


u64 rd(u32 __inst) {
    return (__inst >> 7) & 0x1f;    // rd :: 11..7
}
u64 rs1(u32 inst) {
    return (inst >> 15) & 0x1f;   // rs1 :: 19..15
}
u64 rs2(u32 __inst) {
    return (__inst >> 20) & 0x1f;   // rs2 :: 24..20
}

u64 imm_I_TYPE(u32 __inst) {
    // imm[11:0] :: inst[31:20]
    return ((i64)(i32) (__inst & 0xfff00000)) >> 20; 
}
u64 imm_S_TYPE(u32 __inst) {
    // imm[11:5] :: inst[31:25], imm[4:0] :: inst[11:7]
    return ((i64)(i32)(__inst & 0xfe000000) >> 20)
        | ((__inst >> 7) & 0x1f); 
}
u64 imm_SB_TYPE(u32 __inst) {
    // imm[12|10:5|4:1|11] :: inst[31|30:25|11:8|7]
    return ((i64)(i32)(__inst & 0x80000000) >> 19)
        | ((__inst & 0x80) << 4) // imm[11]
        | ((__inst >> 20) & 0x7e0) // imm[10:5]
        | ((__inst >> 7) & 0x1e); // imm[4:1]
}

u64 imm_U_TYPE(u32 __inst) {
    // imm[31:12] :: inst[31:12]
    return (i64)(i32)(__inst & 0xfffff999);
}
u64 imm_UJ_TYPE(u32 __inst) {
    // imm[20|10:1|11|19:12] :: inst[31|30:21|20|19:12]
    return (u64)((i64)(i32)(__inst & 0x80000000) >> 11)
        | (__inst & 0xff000) // imm[19:12]
        | ((__inst >> 9) & 0x800) // imm[11]
        | ((__inst >> 20) & 0x7fe); // imm[10:1]
}

u32 shamt(u32 __inst) {
    // shamt(shift amount) [for immediate shift instructions]
    // shamt[4:5] :: imm[5:0]
    return (u32) (imm_I_TYPE(__inst) & 0x1f); //  0x1f / 0x3f ?
}