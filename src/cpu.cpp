#include "cpu.hpp"
#include "instruction.hpp"
#include "memory.hpp"

//DBG_OPCODE_HANDLERS
#define ANSI_YELLOW  "\x1b[33m"
#define ANSI_BLUE    "\x1b[31m"
#define ANSI_RESET   "\x1b[0m"

void CPU_init(RISCV_cpu *cpu) {
    cpu->x[0] = 0x00;                                   // register x0 hardwired to 0
    cpu->x[2] = RISCV_MEM_BASE + RISCV_MEM_SIZE;        // Set stack pointer
    cpu->pc = RISCV_MEM_BASE;                           // Set program counter to the base address
}


u32 cpu_fetch(RISCV_cpu *cpu) {
    u32 inst = mem_bus_ld(&(cpu->__bus), cpu->pc, 32);
    return inst;
}

u64 cpu_ld(RISCV_cpu* cpu, u64 addr, uint64_t size) {
    return mem_bus_ld(&(cpu->__bus), addr, size);
}

void cpu_st(RISCV_cpu* cpu, u64 addr, u64 size, u64 value) {
    mem_bus_st(&(cpu->__bus), addr, size, value);
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
void BGEU_exe(RISCV_cpu* cpu, u32 inst) {
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
void LD_exe(RISCV_cpu* cpu, u32 inst) {
    // load 8 byte to rd from address in rs1
    u64 imm = imm_I_TYPE(inst);
    u64 addr = cpu->x[rs1(inst)] + (i64) imm;
    cpu->x[rd(inst)] = (i64) cpu_ld(cpu, addr, 64);
    //DBG(ld)
}
void LBU_exe(RISCV_cpu* cpu, u32 inst) {
    // load unsigned 1 byte to rd from address in rs1
    u64 imm = imm_I_TYPE(inst);
    u64 addr = cpu->x[rs1(inst)] + (i64) imm;
    cpu->x[rd(inst)] = cpu_ld(cpu, addr, 8);
    //DBG(lbu)
}
void LHU_exe(RISCV_cpu* cpu, u32 inst) {
    // load unsigned 2 byte to rd from address in rs1
    u64 imm = imm_I_TYPE(inst);
    u64 addr = cpu->x[rs1(inst)] + (i64) imm;
    cpu->x[rd(inst)] = cpu_ld(cpu, addr, 16);
    //DBG(lhu)
}

void SB_exe(RISCV_cpu* cpu, u32 inst) {
    uint64_t imm = imm_S_TYPE(inst);
    uint64_t addr = cpu->x[rs1(inst)] + (int64_t) imm;
    cpu_st(cpu, addr, 8, cpu->x[rs2(inst)]);
    //DBG(sb)
}
void SH_exe(RISCV_cpu* cpu, u32 inst) {
    u64 imm = imm_S_TYPE(inst);
    u64 addr = cpu->x[rs1(inst)] + (i64) imm;
    cpu_st(cpu, addr, 16, cpu->x[rs2(inst)]);
    //DBG(sh)
}
void SW_exe(RISCV_cpu* cpu, u32 inst) {
    u64 imm = imm_S_TYPE(inst);
    uint64_t addr = cpu->x[rs1(inst)] + (i64) imm;
    cpu_st(cpu, addr, 32, cpu->x[rs2(inst)]);
    //DBG(sw)
}

void ADDI_exe(RISCV_cpu* cpu, u32 inst) {
    u64 imm = imm_I_TYPE(inst);
    cpu->x[rd(inst)] = cpu->x[rs1(inst)] + (i64) imm;
    //DBG(addi)
}

void SLLI_exe(RISCV_cpu* cpu, u32 inst) {
    cpu->x[rd(inst)] = cpu->x[rs1(inst)] << shamt(inst);
    //DBG(slli)
}

void SLTI_exe(RISCV_cpu* cpu, u32 inst) {
    u64 imm = imm_I_TYPE(inst);
    cpu->x[rd(inst)] = (cpu->x[rs1(inst)] < (i64) imm)?1:0;
    //DBG(slti)
}

void SLTIU_exe(RISCV_cpu* cpu, u32 inst) {
    u64 imm = imm_I_TYPE(inst);
    cpu->x[rd(inst)] = (cpu->x[rs1(inst)] < imm)?1:0;
    //DBG(sltiu)
}

void XORI_exe(RISCV_cpu* cpu, u32 inst) {
    u64 imm = imm_I_TYPE(inst);
    cpu->x[rd(inst)] = cpu->x[rs1(inst)] ^ imm;
    //DBG(xori)
}

void SRLI_exe(RISCV_cpu* cpu, u32 inst) {
    uint64_t imm = imm_I_TYPE(inst);
    cpu->x[rd(inst)] = cpu->x[rs1(inst)] >> imm;
    //DBG(srli)
}

void SRAI_exe(RISCV_cpu* cpu, u32 inst) {
    uint64_t imm = imm_I_TYPE(inst);
    cpu->x[rd(inst)] = (i32)cpu->x[rs1(inst)] >> imm;
    //DBG(srai)
}

void ORI_exe(RISCV_cpu* cpu, u32 inst) {
    u64 imm = imm_I_TYPE(inst);
    cpu->x[rd(inst)] = cpu->x[rs1(inst)] | imm;
    //DBG(ori)
}

void ANDI_exe(RISCV_cpu* cpu, u32 inst) {
    u64 imm = imm_I_TYPE(inst);
    cpu->x[rd(inst)] = cpu->x[rs1(inst)] & imm;
    //DBG(andi)
}

void ADD_exe(RISCV_cpu* cpu, u32 inst) {
    cpu->x[rd(inst)] =
        (u64) ((i64)cpu->x[rs1(inst)] + (i64)cpu->x[rs2(inst)]);
    //DBG(add)
}

void SUB_exe(RISCV_cpu* cpu, u32 inst) {
    cpu->x[rd(inst)] =
        (u64) ((i64)cpu->x[rs1(inst)] - (i64)cpu->x[rs2(inst)]);
    //DBG(sub)
}

void SLL_exe(RISCV_cpu* cpu, u32 inst) {
    cpu->x[rd(inst)] = cpu->x[rs1(inst)] << (int64_t)cpu->x[rs2(inst)];
    //DBG(sll)
}

void SLT_exe(RISCV_cpu* cpu, u32 inst) {
    cpu->x[rd(inst)] = (cpu->x[rs1(inst)] < (i64) cpu->x[rs2(inst)])?1:0;
    //DBG(slt)
}

void SLTU_exe(RISCV_cpu* cpu, u32 inst) {
    cpu->x[rd(inst)] = (cpu->x[rs1(inst)] < cpu->x[rs2(inst)])?1:0;
    //DBG(sltu)
}

void XOR_exe(RISCV_cpu* cpu, u32 inst) {
    cpu->x[rd(inst)] = cpu->x[rs1(inst)] ^ cpu->x[rs2(inst)];
    //DBG(xor)
}

void SRL_exe(RISCV_cpu* cpu, u32 inst) {
    cpu->x[rd(inst)] = cpu->x[rs1(inst)] >> cpu->x[rs2(inst)];
    //DBG(srl)
}

void SRA_exe(RISCV_cpu* cpu, u32 inst) {
    cpu->x[rd(inst)] = (i32)cpu->x[rs1(inst)] >> 
        (i64) cpu->x[rs2(inst)];
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

int cpu_execute(RISCV_cpu *cpu, u32 inst) {
    int opcode = inst & 0x7f;           // opcode in bits 6..0
    int funct3 = (inst >> 12) & 0x7;    // funct3 in bits 14..12
    int funct7 = (inst >> 25) & 0x7f;   // funct7 in bits 31..25

    cpu->x[0] = 0;                   // x0 hardwired to 0 at each cycle

    /*printf("%s\n%#.8lx -> Inst: %#.8x <OpCode: %#.2x, funct3:%#x, funct7:%#x> %s",*/
            /*ANSI_YELLOW, cpu->pc-4, inst, opcode, funct3, funct7, ANSI_RESET); // DEBUG*/
    printf("%s\n%#.8lx -> %s", ANSI_YELLOW, cpu->pc-4, ANSI_RESET); // DEBUG
