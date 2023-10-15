#include "cpu.hpp"
#include "instruction.hpp"
#include "memory.hpp"

void CPU_init(RISCV_cpu *cpu) {
    cpu->x[0] = 0x00;                                   // register x0 hardwired to 0
    cpu->x[2] = RISCV_MEM_BASE + RISCV_MEM_SIZE;        // Set stack pointer
    cpu->pc = RISCV_MEM_BASE;                           // Set program counter to the base address
}

u32 cpu_fetch(RISCV_cpu *cpu, RISC_mem *mem) {
    uint32_t inst = (u64)mem_ld(mem, cpu->pc, 32);
    return inst;
}

u64 cpu_ld(RISCV_cpu* cpu, RISC_mem *mem, u64 addr, uint64_t size) {
    return mem_ld(mem, addr, size);
}

void cpu_st(RISCV_cpu* cpu, RISC_mem *mem, u64 addr, u64 size, u64 value) {
    mem_st(mem, addr, size, value);
}

//INSTRUCTION_DECODING
ALWAYS_INLINE u64 rd(u32 instr__) {
    return (instr__ >> 7) & 0x1f;    // rd :: 11..7
}
ALWAYS_INLINE u64 rs1(u32 instr__) {
    return (instr__ >> 15) & 0x1f;   // rs1 :: 19..15
}
ALWAYS_INLINE u64 rs2(u32 instr__) {
    return (instr__ >> 20) & 0x1f;   // rs2 :: 24..20
}

ALWAYS_INLINE u64 imm_I_TYPE(u32 instr__) {
    // imm[11:0] :: inst[31:20]
    return ((i64)(i32) (instr__ & 0xfff00000)) >> 20; 
}
ALWAYS_INLINE u64 imm_S_TYPE(u32 instr__) {
    // imm[11:5] :: inst[31:25], imm[4:0] :: inst[11:7]
    return ((i64)(i32)(instr__ & 0xfe000000) >> 20)
        | ((instr__ >> 7) & 0x1f); 
}
ALWAYS_INLINE u64 imm_SB_TYPE(u32 instr__) {
    // imm[12|10:5|4:1|11] :: inst[31|30:25|11:8|7]
    return ((i64)(i32)(instr__ & 0x80000000) >> 19)
        | ((instr__ & 0x80) << 4) // imm[11]
        | ((instr__ >> 20) & 0x7e0) // imm[10:5]
        | ((instr__ >> 7) & 0x1e); // imm[4:1]
}

ALWAYS_INLINE u64 imm_U_TYPE(u32 instr__) {
    // imm[31:12] :: inst[31:12]
    return (i64)(i32)(instr__ & 0xfffff999);
}
ALWAYS_INLINE u64 imm_UJ_TYPE(u32 instr__) {
    // imm[20|10:1|11|19:12] :: inst[31|30:21|20|19:12]
    return (u64)((i64)(i32)(instr__ & 0x80000000) >> 11)
        | (instr__ & 0xff000) // imm[19:12]
        | ((instr__ >> 9) & 0x800) // imm[11]
        | ((instr__ >> 20) & 0x7fe); // imm[10:1]
}

ALWAYS_INLINE u32 shamt(u32 instr__) {
    // shamt(shift amount) [for immediate shift instructions]
    // shamt[4:5] :: imm[5:0]
    return (u32) (imm_I_TYPE(instr__) & 0x1f); //  0x1f / 0x3f ?
}


//EXECUTION_STAGE

void LUI_exe(RISCV_cpu* cpu, u32 inst) {
    // LUI places upper 20 bits of U-immediate value to rd
    cpu->x[rd(inst)] = (u64)(i64)(i32)(inst & 0xfffff000);
    //DBG(lui)
}

void AUIPC_exe(RISCV_cpu* cpu, u32 inst) {
    // AUIPC forms a 32-bit offset from the 20 upper bits 
    // of the U-immediate
    u64 imm = imm_U_TYPE(inst);
    cpu->x[rd(inst)] = ((i64) cpu->pc + (i64) imm) - 4;
    //DBG(auipc)
}

void JAL_exe(RISCV_cpu* cpu, u32 inst) {
    u64 imm = imm_UJ_TYPE(inst);
    cpu->x[rd(inst)] = cpu->pc;
    /*print_op("JAL-> rd:%ld, pc:%lx\n", rd(inst), cpu->pc);*/
    cpu->pc = cpu->pc + (i64) imm - 4;
    //DBG(jal)
// addr. misalignment consideration??
}


void JALR_exe(RISCV_cpu* cpu, u32 inst) {
    uint64_t imm = imm_I_TYPE(inst);
    uint64_t tmp = cpu->pc;
    cpu->pc = (cpu->x[rs1(inst)] + (int64_t) imm) & 0xfffffffe;
    cpu->x[rd(inst)] = tmp;
    /*print_op("NEXT -> %#lx, imm:%#lx\n", cpu->pc, imm);*/
    //DBG(jalr)
// addr. misalignment consideration??
}
void BEQ_exe(RISCV_cpu* cpu, u32 inst) {
    u64 imm = imm_SB_TYPE(inst);
    if ((i64) cpu->x[rs1(inst)] == (i64) cpu->x[rs2(inst)])
        cpu->pc = cpu->pc + (i64) imm - 4;
    //DBG(beq)
}
void BNE_exe(RISCV_cpu* cpu, u32 inst) {
    u64 imm = imm_SB_TYPE(inst);
    if ((int64_t) cpu->x[rs1(inst)] != (int64_t) cpu->x[rs2(inst)])
        cpu->pc = (cpu->pc + (int64_t) imm - 4);
    //DBG(bne)
}
void BLT_exe(RISCV_cpu* cpu, u32 inst) {
    /*print_op("Operation: BLT\n");*/
    u64 imm = imm_SB_TYPE(inst);
    if ((i64) cpu->x[rs1(inst)] < (i64) cpu->x[rs2(inst)])
        cpu->pc = cpu->pc + (i64) imm - 4;
    //DBG(blt)
}
void BGE_exe(RISCV_cpu* cpu, u32 inst) {
    u64 imm = imm_SB_TYPE(inst);
    if ((i64) cpu->x[rs1(inst)] >= (i64) cpu->x[rs2(inst)])
        cpu->pc = cpu->pc + (i64) imm - 4;
    //DBG(bge)
}
void BLTU_exe(RISCV_cpu* cpu, u32 inst) {
    uint64_t imm = imm_SB_TYPE(inst);
    if (cpu->x[rs1(inst)] < cpu->x[rs2(inst)])
        cpu->pc = cpu->pc + (i64) imm - 4;
    //DBG(bltu)
}
void exec_BGEU(RISCV_cpu* cpu, u32 inst) {
    uint64_t imm = imm_SB_TYPE(inst);
    if (cpu->x[rs1(inst)] >= cpu->x[rs2(inst)])
        cpu->pc = (i64) cpu->pc + (i64) imm - 4;
    //DBG(bgeu)
}
void LB_exe(RISCV_cpu* cpu, u32 inst) {
    // load 1 byte to rd from address in rs1
    u64 imm = imm_I_TYPE(inst);
    u64 addr = cpu->x[rs1(inst)] + (i64) imm;
    cpu->x[rd(inst)] = (i64)(i8) cpu_ld(cpu, addr, 8);
    //DBG(lb)
}
void LH_exe(RISCV_cpu* cpu, u32 inst) {
    // load 2 byte to rd from address in rs1
    u64 imm = imm_I_TYPE(inst);
    u64 addr = cpu->x[rs1(inst)] + (i64) imm;
    cpu->x[rd(inst)] = (i64)(i16) cpu_ld(cpu, addr, 16);
    //DBG(lh)
}
void LW_exe(RISCV_cpu* cpu, u32 inst) {
    // load 4 byte to rd from address in rs1
    u64 imm = imm_I_TYPE(inst);
    u64 addr = cpu->x[rs1(inst)] + (i64) imm;
    cpu->x[rd(inst)] = (i64)(i32) cpu_ld(cpu, addr, 32);
    //DBG(lw)
}
void exec_LD(CPU* cpu, uint32_t inst) {
    // load 8 byte to rd from address in rs1
    uint64_t imm = imm_I(inst);
    uint64_t addr = cpu->regs[rs1(inst)] + (int64_t) imm;
    cpu->regs[rd(inst)] = (int64_t) cpu_load(cpu, addr, 64);
    print_op("ld\n");
}
void exec_LBU(CPU* cpu, uint32_t inst) {
    // load unsigned 1 byte to rd from address in rs1
    uint64_t imm = imm_I(inst);
    uint64_t addr = cpu->regs[rs1(inst)] + (int64_t) imm;
    cpu->regs[rd(inst)] = cpu_load(cpu, addr, 8);
    print_op("lbu\n");
}
void exec_LHU(CPU* cpu, uint32_t inst) {
    // load unsigned 2 byte to rd from address in rs1
    uint64_t imm = imm_I(inst);
    uint64_t addr = cpu->regs[rs1(inst)] + (int64_t) imm;
    cpu->regs[rd(inst)] = cpu_load(cpu, addr, 16);
    print_op("lhu\n");
}
void exec_LWU(CPU* cpu, uint32_t inst) {
    // load unsigned 2 byte to rd from address in rs1
    uint64_t imm = imm_I(inst);
    uint64_t addr = cpu->regs[rs1(inst)] + (int64_t) imm;
    cpu->regs[rd(inst)] = cpu_load(cpu, addr, 32);
    print_op("lwu\n");
}
void exec_SB(CPU* cpu, uint32_t inst) {
    uint64_t imm = imm_S(inst);
    uint64_t addr = cpu->regs[rs1(inst)] + (int64_t) imm;
    cpu_store(cpu, addr, 8, cpu->regs[rs2(inst)]);
    print_op("sb\n");
}
void exec_SH(CPU* cpu, uint32_t inst) {
    uint64_t imm = imm_S(inst);
    uint64_t addr = cpu->regs[rs1(inst)] + (int64_t) imm;
    cpu_store(cpu, addr, 16, cpu->regs[rs2(inst)]);
    print_op("sh\n");
}
void exec_SW(CPU* cpu, uint32_t inst) {
    uint64_t imm = imm_S(inst);
    uint64_t addr = cpu->regs[rs1(inst)] + (int64_t) imm;
    cpu_store(cpu, addr, 32, cpu->regs[rs2(inst)]);
    print_op("sw\n");
}
void exec_SD(CPU* cpu, uint32_t inst) {
    uint64_t imm = imm_S(inst);
    uint64_t addr = cpu->regs[rs1(inst)] + (int64_t) imm;
    cpu_store(cpu, addr, 64, cpu->regs[rs2(inst)]);
    print_op("sd\n");
}

void exec_ADDI(CPU* cpu, uint32_t inst) {
    uint64_t imm = imm_I(inst);
    cpu->regs[rd(inst)] = cpu->regs[rs1(inst)] + (int64_t) imm;
    print_op("addi\n");
}

void exec_SLLI(CPU* cpu, uint32_t inst) {
    cpu->regs[rd(inst)] = cpu->regs[rs1(inst)] << shamt(inst);
    print_op("slli\n");
}

void exec_SLTI(CPU* cpu, uint32_t inst) {
    uint64_t imm = imm_I(inst);
    cpu->regs[rd(inst)] = (cpu->regs[rs1(inst)] < (int64_t) imm)?1:0;
    print_op("slti\n");
}

void exec_SLTIU(CPU* cpu, uint32_t inst) {
    uint64_t imm = imm_I(inst);
    cpu->regs[rd(inst)] = (cpu->regs[rs1(inst)] < imm)?1:0;
    print_op("sltiu\n");
}

void exec_XORI(CPU* cpu, uint32_t inst) {
    uint64_t imm = imm_I(inst);
    cpu->regs[rd(inst)] = cpu->regs[rs1(inst)] ^ imm;
    print_op("xori\n");
}

void exec_SRLI(CPU* cpu, uint32_t inst) {
    uint64_t imm = imm_I(inst);
    cpu->regs[rd(inst)] = cpu->regs[rs1(inst)] >> imm;
    print_op("srli\n");
}

void exec_SRAI(CPU* cpu, uint32_t inst) {
    uint64_t imm = imm_I(inst);
    cpu->regs[rd(inst)] = (int32_t)cpu->regs[rs1(inst)] >> imm;
    print_op("srai\n");
}

void exec_ORI(CPU* cpu, uint32_t inst) {
    uint64_t imm = imm_I(inst);
    cpu->regs[rd(inst)] = cpu->regs[rs1(inst)] | imm;
    print_op("ori\n");
}

void exec_ANDI(CPU* cpu, uint32_t inst) {
    uint64_t imm = imm_I(inst);
    cpu->regs[rd(inst)] = cpu->regs[rs1(inst)] & imm;
    print_op("andi\n");
}

void exec_ADD(CPU* cpu, uint32_t inst) {
    cpu->regs[rd(inst)] =
        (uint64_t) ((int64_t)cpu->regs[rs1(inst)] + (int64_t)cpu->regs[rs2(inst)]);
    print_op("add\n");
}

void exec_SUB(CPU* cpu, uint32_t inst) {
    cpu->regs[rd(inst)] =
        (uint64_t) ((int64_t)cpu->regs[rs1(inst)] - (int64_t)cpu->regs[rs2(inst)]);
    print_op("sub\n");
}

void exec_SLL(CPU* cpu, uint32_t inst) {
    cpu->regs[rd(inst)] = cpu->regs[rs1(inst)] << (int64_t)cpu->regs[rs2(inst)];
    print_op("sll\n");
}

void exec_SLT(CPU* cpu, uint32_t inst) {
    cpu->regs[rd(inst)] = (cpu->regs[rs1(inst)] < (int64_t) cpu->regs[rs2(inst)])?1:0;
    print_op("slt\n");
}

void exec_SLTU(CPU* cpu, uint32_t inst) {
    cpu->regs[rd(inst)] = (cpu->regs[rs1(inst)] < cpu->regs[rs2(inst)])?1:0;
    print_op("slti\n");
}

void exec_XOR(CPU* cpu, uint32_t inst) {
    cpu->regs[rd(inst)] = cpu->regs[rs1(inst)] ^ cpu->regs[rs2(inst)];
    print_op("xor\n");
}

void exec_SRL(CPU* cpu, uint32_t inst) {
    cpu->regs[rd(inst)] = cpu->regs[rs1(inst)] >> cpu->regs[rs2(inst)];
    //DBG(srl)
}

void SRA_exe(CPU* cpu, u32 inst) {
    cpu->x[rd(inst)] = (i32)cpu->x[rs1(inst)] >> 
        (i64) cpu->regs[rs2(inst)];
    //DBG(sra)
}

void OR_exe(RISCV_cpu* cpu, u32 inst) {
    cpu->x[rd(inst)] = cpu->x[rs1(inst)] | cpu->x[rs2(inst)];
    //DBG(or)
}

void AND_exe(RISCV_cpu* cpu, u32 inst) {
    cpu->x[rd(inst)] = cpu->x[rs1(inst)] & cpu->x[rs2(inst)];
    //DBG(and)
}

// AMO_W
void exec_LR_W(CPU* cpu, uint32_t inst) {}  
void exec_SC_W(CPU* cpu, uint32_t inst) {}  
void exec_AMOSWAP_W(CPU* cpu, uint32_t inst) {}  
void exec_AMOADD_W(CPU* cpu, uint32_t inst) {
    uint32_t tmp = cpu_load(cpu, cpu->regs[rs1(inst)], 32);
    uint32_t res = tmp + (uint32_t)cpu->regs[rs2(inst)];
    cpu->regs[rd(inst)] = tmp;
    cpu_store(cpu, cpu->regs[rs1(inst)], 32, res);
    print_op("amoadd.w\n");
} 
void exec_AMOXOR_W(CPU* cpu, uint32_t inst) {
    uint32_t tmp = cpu_load(cpu, cpu->regs[rs1(inst)], 32);
    uint32_t res = tmp ^ (uint32_t)cpu->regs[rs2(inst)];
    cpu->regs[rd(inst)] = tmp;
    cpu_store(cpu, cpu->regs[rs1(inst)], 32, res);
    print_op("amoxor.w\n");
} 
void exec_AMOAND_W(CPU* cpu, uint32_t inst) {
    uint32_t tmp = cpu_load(cpu, cpu->regs[rs1(inst)], 32);
    uint32_t res = tmp & (uint32_t)cpu->regs[rs2(inst)];
    cpu->regs[rd(inst)] = tmp;
    cpu_store(cpu, cpu->regs[rs1(inst)], 32, res);
    print_op("amoand.w\n");
} 
void exec_AMOOR_W(CPU* cpu, uint32_t inst) {
    uint32_t tmp = cpu_load(cpu, cpu->regs[rs1(inst)], 32);
    uint32_t res = tmp | (uint32_t)cpu->regs[rs2(inst)];
    cpu->regs[rd(inst)] = tmp;
    cpu_store(cpu, cpu->regs[rs1(inst)], 32, res);
    print_op("amoor.w\n");
} 
void exec_AMOMIN_W(CPU* cpu, uint32_t inst) {} 
void exec_AMOMAX_W(CPU* cpu, uint32_t inst) {} 
void exec_AMOMINU_W(CPU* cpu, uint32_t inst) {} 
void exec_AMOMAXU_W(CPU* cpu, uint32_t inst) {} 

// AMO_D TODO
void exec_LR_D(CPU* cpu, uint32_t inst) {}  
void exec_SC_D(CPU* cpu, uint32_t inst) {}  
void exec_AMOSWAP_D(CPU* cpu, uint32_t inst) {}  
void exec_AMOADD_D(CPU* cpu, uint32_t inst) {
    uint32_t tmp = cpu_load(cpu, cpu->regs[rs1(inst)], 32);
    uint32_t res = tmp + (uint32_t)cpu->regs[rs2(inst)];
    cpu->regs[rd(inst)] = tmp;
    cpu_store(cpu, cpu->regs[rs1(inst)], 32, res);
    print_op("amoadd.w\n");
} 
void exec_AMOXOR_D(CPU* cpu, uint32_t inst) {
    uint32_t tmp = cpu_load(cpu, cpu->regs[rs1(inst)], 32);
    uint32_t res = tmp ^ (uint32_t)cpu->regs[rs2(inst)];
    cpu->regs[rd(inst)] = tmp;
    cpu_store(cpu, cpu->regs[rs1(inst)], 32, res);
    print_op("amoxor.w\n");
} 
void exec_AMOAND_D(CPU* cpu, uint32_t inst) {
    uint32_t tmp = cpu_load(cpu, cpu->regs[rs1(inst)], 32);
    uint32_t res = tmp & (uint32_t)cpu->regs[rs2(inst)];
    cpu->regs[rd(inst)] = tmp;
    cpu_store(cpu, cpu->regs[rs1(inst)], 32, res);
    print_op("amoand.w\n");
} 
void exec_AMOOR_D(CPU* cpu, uint32_t inst) {
    uint32_t tmp = cpu_load(cpu, cpu->regs[rs1(inst)], 32);
    uint32_t res = tmp | (uint32_t)cpu->regs[rs2(inst)];
    cpu->regs[rd(inst)] = tmp;
    cpu_store(cpu, cpu->regs[rs1(inst)], 32, res);
    print_op("amoor.w\n");
} 
void exec_AMOMIN_D(CPU* cpu, uint32_t inst) {} 
void exec_AMOMAX_D(CPU* cpu, uint32_t inst) {} 
void exec_AMOMINU_D(CPU* cpu, uint32_t inst) {} 
void exec_AMOMAXU_D(CPU* cpu, uint32_t inst) {} 

int cpu_execute(CPU *cpu, uint32_t inst) {
    int opcode = inst & 0x7f;           // opcode in bits 6..0
    int funct3 = (inst >> 12) & 0x7;    // funct3 in bits 14..12
    int funct7 = (inst >> 25) & 0x7f;   // funct7 in bits 31..25

    cpu->regs[0] = 0;                   // x0 hardwired to 0 at each cycle

    /*printf("%s\n%#.8lx -> Inst: %#.8x <OpCode: %#.2x, funct3:%#x, funct7:%#x> %s",*/
            /*ANSI_YELLOW, cpu->pc-4, inst, opcode, funct3, funct7, ANSI_RESET); // DEBUG*/
    printf("%s\n%#.8lx -> %s", ANSI_YELLOW, cpu->pc-4, ANSI_RESET); // DEBUG
