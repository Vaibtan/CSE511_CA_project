#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#define RV32i_LUI 0x37
#define RV32i_AUIPC 0x17
#define RV32i_JAL 0x6f
#define RV32i_JALR 0x67

#define RV32i_LOAD 0x03
    #define LB 0x0
    #define LH 0x1
    #define LW 0x2
    #define LBU 0x4
    #define LHU 0x5
 

#define RV32i_SB_TYPE 0x63
    #define BEQ 0x0
    #define BNE 0x1
    #define BLT 0x4
    #define BGE 0x5
    #define BLTU 0x6
    #define BGEU 0x7

#define RV32i_S_TYPE 0x23
    #define SB 0x0
    #define SH 0x1
    #define SW 0x2

#define RV32i_I_TYPE 0x13
    #define ADDI 0x0
    #define SLLI 0x1
    #define SLTI 0x2
    #define SLTIU 0x3
    #define XORI 0x4
    #define SRI 0x5
        #define SRLI 0x00
        #define SRAI 0x20
    #define ORI 0x6
    #define ANDI 0x7

#define RV32i_R_TYPE 0x33
    #define ADDSUB 0x0
        #define ADD 0x00
        #define SUB 0x20
    #define SLL 0x1
    #define SLT 0x2
    #define SLTU 0x3
    #define XOR 0x4
    #define SR 0x5
        #define SRL 0x00
        #define SRA 0x20
    #define OR 0x6
    #define AND 0x7

#endif

// typedef struct {
//     u32 opcode : 7;
//     u32 rd : 5;
//     u32 imm : 20;
// } U_type;

// typedef struct {
//     u32 opcode : 7;
//     u32 rd : 5;
//     u32 imm : 20;
// } UJ_type;

// typedef struct {
//     u32 opcode : 7;
//     u32 rd : 5;
//     u32 func3 : 3;
//     u32 rs1 : 5;
//     u32 rs2 : 5;
//     u32 func7 : 7;
// } R_type;

// typedef struct {
//     u32 opcode : 7;
//     u32 rd : 5;
//     u32 func3 : 3;
//     u32 rs1 : 5;
//     u32 imm : 12;
// } I_type;

// typedef struct {
//     u32 opcode : 7;
//     u32 imm_0_4 : 5;
//     u32 func3 : 3;
//     u32 rs1 : 5;
//     u32 rs2 : 5;
//     u32 imm_5_11 : 7;
// } S_type;

// typedef struct {
//     u32 opcode : 7;
//     u32 imm_4_1_11 : 5;
//     u32 func3 : 3;
//     u32 rs1 : 5;
//     u32 rs2 : 5;
//     u32 imm_12_5_10 : 7;
// } SB_type;


// typedef union {
//     U_type __u;
//     UJ_type __uj;
//     R_type __r;
//     I_type __i;
//     S_type __s;
//     SB_type __sb;
//     u32;
//     i32;
// } instr__;

/**
typedef enum {LOAD = 0x03, ALU_IMM = 0x13, STORE=0x23, ALU=0x33, BRANCH=0x63,
              LUI = 0x37, AUIPC = 0x17, JAL = 0x6F, JALR = 0x67} __opcode;


*/




