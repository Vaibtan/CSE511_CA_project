#include "cpu.hpp"

//PC reset and initialisation
void CPU_reset(RISCV_cpu *cpu) {
    cpu->x[0] = 0x00;                                   // register x0 hardwired to 0
    cpu->x[2] = RISCV_MEM_BASE + RISCV_MEM_SIZE;        // Set stack pointer
    cpu->pc = RISCV_MEM_BASE;                           // Set program counter to the base address
    cpu->x[1] = 0;                                      // return address
    for(int i=3;i<REG_LEN;i++){
        cpu->x[i]=0;
    }
    pipeline_reset(cpu->__pipe);
    cpu->__alu->flag_reset();
}

// cpu initialization
RISCV_cpu* CPU_init() {
    RISCV_cpu *cpu; 
    if (cpu = (RISCV_cpu*) malloc(sizeof(RISCV_cpu))) {
        fprintf(stderr, "[-] ERROR-> CPU_init: malloc failed\n");
        exit(1);
    }
    cpu->__bus = mem_bus_init(mem_init());
    if (cpu->__bus==NULL) {
        fprintf(stderr, "[-] ERROR-> mem_bus_init: malloc failed\n");
        exit(1);
    }
    cpu->__pipe = pipe_init();
    if (cpu->__pipe==NULL) {
        fprintf(stderr, "[-] ERROR-> pipe_init : malloc failed\n");
        exit(1);
    }
    cpu->__alu = alu_init();
    if (cpu->__alu==NULL) {
        fprintf(stderr, "[-] ERROR-> alu_init : malloc failed\n");
        exit(1);
    }
    CPU_reset(cpu);
    return cpu;
}

//FETCH_STAGE
void cpu_fetch(RISCV_cpu *cpu) {
    if(cpu->__pipe->fetch->done)return;
    cpu ->__pipe->fetch->inst = mem_bus_ld(cpu->__bus, cpu->pc, 32);
    // cpu->__pipe->bypass->pcf=cpu->pc;
    cpu->__pipe->fetch->done=true;
}

//Pc adder
u32 pc_adder(RISCV_cpu*cpu,u32 a){
    return cpu->pc+a;
}

//PC write function
void fetch_pc_update(RISCV_cpu*cpu,u32 pc_new){
    cpu->pc=pc_new;
}

//PC update function
void cpu_pc_update(RISCV_cpu*cpu){
    if(cpu->__pipe->isbranch){
        if(cpu->__pipe->isjalr){
            fetch_pc_update(cpu,cpu->__pipe->newpc_offset);
        }
        else{
            fetch_pc_update(cpu,pc_adder(cpu,cpu->__pipe->newpc_offset));
        }
    }
    else
    fetch_pc_update(cpu,pc_adder(cpu,4));
    // cpu->__pipe->bypass->new_pc=cpu->pc;
}

//Helpr functions for decoding
ALWAYS_INLINE u32 rd(u32 instr__) {
    return (instr__ >> 7) & 0x1f;    // rd :: 11..7
}
ALWAYS_INLINE u32 rs1(u32 instr__) {
    return (instr__ >> 15) & 0x1f;   // rs1 :: 19..15
}
ALWAYS_INLINE u32 rs2(u32 instr__) {
    return (instr__ >> 20) & 0x1f;   // rs2 :: 24..20
}

ALWAYS_INLINE i32 imm_I_TYPE(u32 instr__) {
    // imm[11:0] :: inst[31:20]
    return ((i32) (instr__ & 0xfff00000)) >> 20; 
}
ALWAYS_INLINE i32 imm_S_TYPE(u32 instr__) {
    // imm[11:5] :: inst[31:25], imm[4:0] :: inst[11:7]
    return ((i32)(instr__ & 0xfe000000) >> 20)
        | ((instr__ >> 7) & 0x1f); 
}
ALWAYS_INLINE i32 imm_SB_TYPE(u32 instr__) {
    // imm[12|10:5|4:1|11] :: inst[31|30:25|11:8|7]
    return ((i32)(instr__ & 0x80000000) >> 19)
        | ((instr__ & 0x80) << 4) // imm[11]
        | ((instr__ >> 20) & 0x7e0) // imm[10:5]
        | ((instr__ >> 7) & 0x1e); // imm[4:1]
}

ALWAYS_INLINE i32 imm_U_TYPE(u32 instr__) {
    // imm[31:12] :: inst[31:12]
    return (i32)((instr__ >>12 & 0xfffff000));
}
ALWAYS_INLINE i32 imm_UJ_TYPE(u32 instr__) {
    // imm[20|10:1|11|19:12] :: inst[31|30:21|20|19:12]
    return ((i32)(instr__ & 0x80000000) >> 11)
        | (instr__ & 0xff000) // imm[19:12]
        | ((instr__ >> 9) & 0x800) // imm[11]
        | ((instr__ >> 20) & 0x7fe); // imm[10:1]
}

ALWAYS_INLINE i32 shamt(u32 instr__) {
    // shamt(shift amount) [for immediate shift instructions]
    // shamt[4:5] :: imm[5:0]
    return (i32) (u32) (imm_I_TYPE(instr__) & 0x1f); //  0x1f / 0x3f ?
}

ALWAYS_INLINE u32 jalr_adder(u32 a, u32 b){
    return (a+b)&0xfffffffe;
}

//Decodes read registers
u32 cpu_read_reg(RISCV_cpu*cpu,u32 rs){
    return cpu->x[rs]; 
}

//control unit for decode stage
void cpu_control_unit(RISCV_cpu *cpu,pipeline *pipe,u32 inst) {
    pipe->decode->inst = inst & 0x7f;           // opcode in bits 6..0
    int funct3 = (inst >> 12) & 0x7;    // funct3 in bits 14..12
    int funct7 = (inst >> 25) & 0x7f;   // funct7 in bits 31..25
    switch (pipe->decode->inst){
        case RV32i_LUI:
            pipe->decode->inst=1;
            pipe->decode->rd=rd(inst);
            pipe->decode->imm=(u32)(i32)(inst & 0xfffff000);
            pipe->decode->iswrite=true;
            // LUI_exe(cpu, inst);
            break;
        case RV32i_AUIPC:
            pipe->decode->inst=2; 
            pipe->decode->rd=rd(inst);
            pipe->decode->imm= imm_U_TYPE(inst);
            pipe->decode->iswrite=true;
            // AUIPC_exe(cpu, inst); 
            break;

        case RV32i_JAL:
            pipe->decode->inst=3; 
            pipe->isbranch=true;
            pipe->newpc_offset= imm_UJ_TYPE(inst);
            pipe->decode->rd=rd(inst);
            pipe->decode->iswrite=true;
            pipe->decode->imm=cpu->pc;
            pipe->branch_by_decode=true;
            // JAL_exe(cpu, inst);
            break;
        case RV32i_JALR:
            pipe->decode->inst=4; 
            pipe->decode->rd=rd(inst);
            pipe->decode->rs1=rs1(inst);
            pipe->newpc_offset=imm_I_TYPE(inst);
            pipe->newpc_offset=jalr_adder(pipe->newpc_offset,cpu->x[pipe->decode->rs1]);//Adding jalr offset
            pipe->decode->iswrite=true;
            pipe->isbranch=true;
            pipe->decode->imm=cpu->pc;
            pipe->isjalr=true;
            pipe->branch_by_decode=true;
            // JALR_exe(cpu, inst);
            break;

        case RV32i_SB_TYPE:
            switch (funct3) {
                case BEQ:
                    pipe->decode->inst=5; 
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->rs2=rs2(inst);
                    pipe->decode->imm= imm_SB_TYPE(inst);   
                    // BEQ_exe(cpu, inst); 
                    break;
                case BNE:
                    pipe->decode->inst=6; 
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->rs2=rs2(inst);
                    pipe->decode->imm= imm_SB_TYPE(inst);     
                    // BNE_exe(cpu, inst); 
                    break;
                case BLT:
                    pipe->decode->inst=7; 
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->rs2=rs2(inst);
                    pipe->decode->imm= imm_SB_TYPE(inst);   
                    // BLT_exe(cpu, inst); 
                    break;
                case BGE:
                    pipe->decode->inst=8;
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->rs2=rs2(inst);
                    pipe->decode->imm= imm_SB_TYPE(inst);    
                    // BGE_exe(cpu, inst); 
                    break;
                case BLTU:  
                    pipe->decode->inst=9;
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->rs2=rs2(inst);
                    pipe->decode->imm= imm_SB_TYPE(inst); 
                    // BLTU_exe(cpu, inst); 
                    break;
                case BGEU:
                    pipe->decode->inst=10;
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->rs2=rs2(inst);
                    pipe->decode->imm= imm_SB_TYPE(inst);  
                    // BGEU_exe(cpu, inst); 
                    break;
                default:
                    fprintf(stderr, 
                            "[-] ERROR-> opcode:0x%x, funct3:0x%x, funct3:0x%x\n"
                            , pipe->decode->inst, funct3,funct7);
                    exit(1);
            } 
            break;

        case RV32i_LOAD:
            pipe->decode->isload=true;
            pipe->decode->iswrite=true;
            switch (funct3) {
                case LB  :
                    pipe->decode->inst=11;
                    pipe->decode->rd=rd(inst);
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->imm= imm_I_TYPE(inst); 
                    // LB_exe(cpu, inst);
                    break;  
                case LH  :  
                    pipe->decode->inst=12;
                    pipe->decode->rd=rd(inst);
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->imm= imm_I_TYPE(inst); 
                    // LH_exe(cpu, inst);
                    break;  
                case LW  :  
                    pipe->decode->inst=13;
                    pipe->decode->rd=rd(inst);
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->imm= imm_I_TYPE(inst); 
                    // LW_exe(cpu, inst);
                    break;
                case LBU  :  
                    pipe->decode->inst=14;
                    pipe->decode->rd=rd(inst);
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->imm= imm_I_TYPE(inst); 
                    // LW_exe(cpu, inst);
                    break;    
                case LHU :  
                    pipe->decode->inst=15;
                    pipe->decode->rd=rd(inst);
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->imm= imm_I_TYPE(inst); 
                    // LHU_exe(cpu, inst); 
                    break; 
                default:
                    fprintf(stderr, 
                            "[-] ERROR-> opcode:0x%x, funct3:0x%x, funct3:0x%x\n"
                            , pipe->decode->inst, funct3,funct7);
                    exit(1);
            }
            break;

        case RV32i_S_TYPE:
            pipe->decode->isstore=true;
            switch (funct3) {
                case SB  :
                    pipe->decode->inst=16;
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->rs2=rs2(inst);
                    pipe->decode->imm= imm_S_TYPE(inst);
                    // SB_exe(cpu, inst);
                    break;  
                case SH  :
                    pipe->decode->inst=17;
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->rs2=rs2(inst);
                    pipe->decode->imm= imm_S_TYPE(inst);  
                    // SH_exe(cpu, inst);
                    break;  
                case SW  : 
                    pipe->decode->inst=18;
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->rs2=rs2(inst);
                    pipe->decode->imm= imm_S_TYPE(inst);
                    // SW_exe(cpu, inst);
                    break;   
                default:
                    fprintf(stderr, 
                            "[-] ERROR-> opcode:0x%x, funct3:0x%x, funct3:0x%x\n"
                            , pipe->decode->inst, funct3,funct7);
                    exit(1);
            } 
            break;

        case RV32i_I_TYPE:
            pipe->decode->iswrite=true;
            switch (funct3) {
                case ADDI: 
                    pipe->decode->inst=19;
                    pipe->decode->rd=rd(inst);
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->imm= imm_I_TYPE(inst);
                    // ADDI_exe(cpu, inst);
                    break;
                case SLLI:
                    pipe->decode->inst=25;
                    pipe->decode->rd=rd(inst);
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->imm= shamt(inst);
                    // SLLI_exe(cpu, inst);
                    break;
                case SLTI:
                    pipe->decode->inst=20;
                    pipe->decode->rd=rd(inst);
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->imm= imm_I_TYPE(inst);  
                    // SLTI_exe(cpu, inst);
                    break;
                case SLTIU:
                    pipe->decode->inst=21;
                    pipe->decode->rd=rd(inst);
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->imm= imm_I_TYPE(inst);
                    // SLTIU_exe(cpu, inst);
                    break;
                case XORI: 
                    pipe->decode->inst=22;
                    pipe->decode->rd=rd(inst);
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->imm= imm_I_TYPE(inst);
                    // XORI_exe(cpu, inst);
                    break;
                case SRI:   
                    switch (funct7) {
                        case SRLI:
                            pipe->decode->inst=26;
                            pipe->decode->rd=rd(inst);
                            pipe->decode->rs1=rs1(inst);
                            pipe->decode->imm= shamt(inst);
                            // SRLI_exe(cpu, inst);
                            break;
                        case SRAI: 
                            pipe->decode->inst=27;
                            pipe->decode->rd=rd(inst);
                            pipe->decode->rs1=rs1(inst);
                            pipe->decode->imm= shamt(inst);
                            // SRAI_exe(cpu, inst);
                            break;
                        default:
                            fprintf(stderr, 
                                    "[-] ERROR-> opcode:0x%x, funct3:0x%x, funct3:0x%x\n"
                                    , pipe->decode->inst,funct3,funct7);
                            exit(1);
                    } 
                    break;
                case ORI:
                    pipe->decode->inst=23;
                    pipe->decode->rd=rd(inst);
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->imm= imm_I_TYPE(inst);   
                    // ORI_exe(cpu, inst);
                    break;
                case ANDI:
                    pipe->decode->inst=24;
                    pipe->decode->rd=rd(inst);
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->imm= imm_I_TYPE(inst);  
                    // ANDI_exe(cpu, inst); 
                    break;
                default:
                    fprintf(stderr, 
                            "[-] ERROR-> opcode:0x%x, funct3:0x%x, funct3:0x%x\n"
                            , pipe->decode->inst, funct3,funct7);
                    exit(1);
            } 
            break;

        case RV32i_R_TYPE:
            pipe->decode->isimm=false;
            pipe->decode->iswrite=true;   
            switch (funct3) {
                case ADDSUB:
                    switch (funct7) {
                        case ADD: 
                            pipe->decode->inst=28;
                            pipe->decode->rd=rd(inst);
                            pipe->decode->rs1=rs1(inst);
                            pipe->decode->rs2=rs2(inst);
                            // ADD_exe(cpu, inst);
                            break;
                        case SUB:
                            pipe->decode->inst=29;
                            pipe->decode->rd=rd(inst);
                            pipe->decode->rs1=rs1(inst);
                            pipe->decode->rs2=rs2(inst);
                            // SUB_exe(cpu, inst);
                            break;
                        default:
                            fprintf(stderr, 
                                "[-] ERROR-> opcode:0x%x, funct3:0x%x, funct3:0x%x\n"
                                , pipe->decode->inst, funct3,funct7);
                            exit(1);
                    } 
                    break;
                case SLL:
                    pipe->decode->inst=30;
                    pipe->decode->rd=rd(inst);
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->rs2=rs2(inst);

                    // SLL_exe(cpu, inst);
                    break;
                case SLT:
                    pipe->decode->inst=31;
                    pipe->decode->rd=rd(inst);
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->rs2=rs2(inst);  
                    // SLT_exe(cpu, inst); 
                    break;
                case SLTU: 
                    pipe->decode->inst=32;
                    pipe->decode->rd=rd(inst);
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->rs2=rs2(inst);
                    // SLTU_exe(cpu, inst); 
                    break;
                case XOR:
                    pipe->decode->inst=33;
                    pipe->decode->rd=rd(inst);
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->rs2=rs2(inst);  
                    // XOR_exe(cpu, inst); 
                    break;
                case SR:   
                    switch (funct7) {
                        case SRL:
                            pipe->decode->inst=34;
                            pipe->decode->rd=rd(inst);
                            pipe->decode->rs1=rs1(inst);
                            pipe->decode->rs2=rs2(inst);  
                            // SRL_exe(cpu, inst); 
                            break;
                        case SRA: 
                            pipe->decode->inst=35;
                            pipe->decode->rd=rd(inst);
                            pipe->decode->rs1=rs1(inst);
                            pipe->decode->rs2=rs2(inst);
                            // SRA_exe(cpu, inst); 
                            break;
                        default:
                            fprintf(stderr, 
                                    "[-] ERROR-> opcode:0x%x, funct3:0x%x, funct3:0x%x\n"
                                    , pipe->decode->inst, funct3,funct7);
                            exit(1);
                    }
                    break;
                case OR:
                    pipe->decode->inst=36;
                    pipe->decode->rd=rd(inst);
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->rs2=rs2(inst);   
                    // OR_exe(cpu, inst); 
                    break;
                case AND:
                    pipe->decode->inst=37;
                    pipe->decode->rd=rd(inst);
                    pipe->decode->rs1=rs1(inst);
                    pipe->decode->rs2=rs2(inst);  
                    // AND_exe(cpu, inst);
                    break;
                default:
                    fprintf(stderr, 
                            "[-] ERROR-> opcode:0x%x, funct3:0x%x, funct3:0x%x\n"
                            , pipe->decode->inst, funct3,funct7);
                    exit(1);
            } 
            break;

        case 0x00:
            pipe->decode->inst=0;
            break;

        default:
            fprintf(stderr, 
                    "[-] ERROR-> opcode:0x%x, funct3:0x%x, funct3:0x%x\n"
                    , pipe->decode->inst, funct3, funct7);
            exit(1);
    }
}

//Decode_stage
void cpu_decode(RISCV_cpu *cpu){
    if(cpu->__pipe->decode->done)return;
    cpu_control_unit(cpu,cpu->__pipe,cpu->__pipe->decode->inst);

    // Reading from reg file:
    if(cpu->__pipe->decode->rs1!=34){
        cpu->__pipe->decode->rs1_val=cpu_read_reg(cpu,cpu->__pipe->decode->rs1);
    }
    if(cpu->__pipe->decode->rs2!=34){
        cpu->__pipe->decode->rs2_val=cpu_read_reg(cpu,cpu->__pipe->decode->rs2);
    }
    cpu->__pipe->decode->done=true;
}

//Execute_stage
void cpu_execute(RISCV_cpu *cpu){
    if(cpu->__pipe->execute->done)return;
    switch(cpu->__pipe->execute->inst){
        case 0:
            break;
        case 1:
            cpu->__pipe->execute->result=cpu->__pipe->execute->op2;
            break;
        case 2:
            cpu->__pipe->execute->result=cpu->__alu->add((i32)cpu->__alu->sl((i32)cpu->__pipe->execute->op2,12),(i32)cpu->pc);
            break;
        case 3:
            cpu->__pipe->execute->result=cpu->__alu->add(4,cpu->pc);
            break;
        case 4:
            cpu->__pipe->execute->result=cpu->__alu->add(4,cpu->pc);
            break;
        case 5:
            cpu->__alu->compu(cpu->__pipe->execute->op1,cpu->__pipe->execute->op2);
            if(cpu->__alu->getflag()&0x41==0x41){
                cpu->__pipe->isbranch=true;
                cpu->__pipe->branch_by_decode=false;
                cpu->__pipe->newpc_offset=cpu->__pipe->execute->rd;
            }
            break;
        case 6:
            cpu->__alu->compu(cpu->__pipe->execute->op1,cpu->__pipe->execute->op2);
            if(cpu->__alu->getflag()&0x9==0x9){
                cpu->__pipe->isbranch=true;
                cpu->__pipe->branch_by_decode=false;
                cpu->__pipe->newpc_offset=cpu->__pipe->execute->rd;
            }
            break;
        case 7:
            cpu->__alu->comp((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            if(cpu->__alu->getflag()&0x21==0x21){
                cpu->__pipe->isbranch=true;
                cpu->__pipe->branch_by_decode=false;
                cpu->__pipe->newpc_offset=cpu->__pipe->execute->rd;
            }
            break;
        case 8:
            cpu->__alu->comp((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            if(cpu->__alu->getflag()&0x51==0x51){
                cpu->__pipe->isbranch=true;
                cpu->__pipe->branch_by_decode=false;
                cpu->__pipe->newpc_offset=cpu->__pipe->execute->rd;
            }
            break;
        case 9:
            cpu->__alu->compu(cpu->__pipe->execute->op1,cpu->__pipe->execute->op2);
            if(cpu->__alu->getflag()&0x21==0x21){
                cpu->__pipe->isbranch=true;
                cpu->__pipe->branch_by_decode=false;
                cpu->__pipe->newpc_offset=cpu->__pipe->execute->rd;
            }
            break;
        case 10:
            cpu->__alu->compu(cpu->__pipe->execute->op1,cpu->__pipe->execute->op2);
            if(cpu->__alu->getflag()&0x51==0x51){
                cpu->__pipe->isbranch=true;
                cpu->__pipe->branch_by_decode=false;
                cpu->__pipe->newpc_offset=cpu->__pipe->execute->rd;
            }
            break;
        case 11:
            cpu->__pipe->execute->result=cpu->__alu->add((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            cpu->__pipe->execute->size=8;
            break;
        case 12:
            cpu->__pipe->execute->result=cpu->__alu->add((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            cpu->__pipe->execute->size=16;
            break;
        case 13:
            cpu->__pipe->execute->result=cpu->__alu->add((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            cpu->__pipe->execute->size=32;
            break;
        case 14:
            cpu->__pipe->execute->result=cpu->__alu->add((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            cpu->__pipe->execute->size=8;
            cpu->__pipe->execute->usign=true;
            break;
        case 15:
            cpu->__pipe->execute->result=cpu->__alu->add((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            cpu->__pipe->execute->size=16;
            cpu->__pipe->execute->usign=true;
            break;
        case 16:
            cpu->__pipe->execute->result=cpu->__alu->add((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->rd);
            break;
        case 17:
            cpu->__pipe->execute->result=cpu->__alu->add((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->rd);
            break;
        case 18:
            cpu->__pipe->execute->result=cpu->__alu->add((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->rd);
            break;
        case 19:
            cpu->__pipe->execute->result=cpu->__alu->add((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            break;
        case 20:
            cpu->__alu->comp((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            if(cpu->__alu->getflag()&0x21==0x21){
                cpu->__pipe->execute->result=1;
            }
            else {
                cpu->__pipe->execute->result=0;
            }
            break;
        case 21:
            cpu->__alu->compu((u32)cpu->__pipe->execute->op1,(u32)cpu->__pipe->execute->op2);
            if(cpu->__alu->getflag()&0x21==0x21){
                cpu->__pipe->execute->result=1;
            }
            else {
                cpu->__pipe->execute->result=0;
            }
            break;
        case 22:
            cpu->__pipe->execute->result=cpu->__alu->xori((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            break;
        case 23:
            cpu->__pipe->execute->result=cpu->__alu->ori((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            break;
        case 24:
            cpu->__pipe->execute->result=cpu->__alu->andi((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            break;
        case 25:
            cpu->__pipe->execute->result=cpu->__alu->sl((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            break;
        case 26:
            cpu->__pipe->execute->result=cpu->__alu->sr((u32)cpu->__pipe->execute->op1,(u32)cpu->__pipe->execute->op2);
            break;
        case 27:
            cpu->__pipe->execute->result=cpu->__alu->sra((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            break;
        case 28:
            cpu->__pipe->execute->result=cpu->__alu->add((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            break;
        case 29:
            cpu->__pipe->execute->result=cpu->__alu->sub((i32)cpu->__pipe->execute->op1,-(i32)cpu->__pipe->execute->op2);
            break;
        case 30:
            cpu->__pipe->execute->result=cpu->__alu->sl((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            break;
        case 31:
            cpu->__alu->comp((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            if(cpu->__alu->getflag()&0x21==0x21){
                cpu->__pipe->execute->result=1;
            }
            else {
                cpu->__pipe->execute->result=0;
            }
            break;
        case 32:
            cpu->__alu->compu((u32)cpu->__pipe->execute->op1,(u32)cpu->__pipe->execute->op2);
            if(cpu->__alu->getflag()&0x21==0x21){
                cpu->__pipe->execute->result=1;
            }
            else {
                cpu->__pipe->execute->result=0;
            }
            break;
        case 33:
            cpu->__pipe->execute->result=cpu->__alu->xori((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            break;
        case 34:
            cpu->__pipe->execute->result=cpu->__alu->sr((u32)cpu->__pipe->execute->op1,(u32)cpu->__pipe->execute->op2);
            break;
        case 35:
            cpu->__pipe->execute->result=cpu->__alu->sra((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            break;
        case 36:
            cpu->__pipe->execute->result=cpu->__alu->ori((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            break;
        case 37:
            cpu->__pipe->execute->result=cpu->__alu->andi((i32)cpu->__pipe->execute->op1,(i32)cpu->__pipe->execute->op2);
            break;
        default:
        fprintf(stderr, 
                "[-] ERROR-> inst%d\n"
                , cpu->__pipe->execute->inst);
        exit(1);
    }
    cpu->__alu->flag_reset();
    // cpu->__pipe->bypass->res_exec = cpu->__pipe->execute->result;
    // cpu->__pipe->bypass->rde = cpu->__pipe->execute->rd;
    cpu->__pipe->execute->done=true;
}

//Helper Function for load and store
u64 cpu_ld(RISCV_cpu* cpu, u64 addr, u64 size) {
    return mem_bus_ld((cpu->__bus), addr, size);
}
void cpu_st(RISCV_cpu* cpu, u64 addr, u64 size, u64 value) {
    mem_bus_st((cpu->__bus), addr, size, value);
}

//Memory_stage
void cpu_memory(RISCV_cpu *cpu){
    if(cpu->__pipe->memory->done)return;
    if(cpu->__pipe->memory->isload){
        if(cpu->__pipe->memory->usign)
        cpu->__pipe->memory->value=(u32)cpu_ld(cpu,cpu->__pipe->memory->addr,cpu->__pipe->memory->size);
        else{
            if(cpu->__pipe->memory->size==8)
            cpu->__pipe->memory->value=(i32)(i8)cpu_ld(cpu,cpu->__pipe->memory->addr,cpu->__pipe->memory->size);
            else if(cpu->__pipe->memory->size==16)
            cpu->__pipe->memory->value=(i32)(i16)cpu_ld(cpu,cpu->__pipe->memory->addr,cpu->__pipe->memory->size);
            else
            cpu->__pipe->memory->value=(i32)cpu_ld(cpu,cpu->__pipe->memory->addr,cpu->__pipe->memory->size);
        }
    }
    if(cpu->__pipe->memory->isstore){
        cpu_st(cpu,cpu->__pipe->memory->addr,cpu->__pipe->memory->size,cpu->__pipe->memory->value);
        
    }
    // cpu->__pipe->bypass->res_mem = cpu->__pipe->memory->value;
    // cpu->__pipe->bypass->rdm = cpu->__pipe->memory->rd;
    cpu->__pipe->memory->done=true;
}

//Writeback_stage
void cpu_writeback(RISCV_cpu*cpu){
    if(cpu->__pipe->writeback->done)return;
    if(cpu->__pipe->writeback->iswrite){
        cpu->x[cpu->__pipe->writeback->rd]=cpu->__pipe->writeback->imm;
    }
    cpu->__pipe->writeback->done=true;
    if(cpu->__pipe->ex_stall){
        cpu->__pipe->execute->op2=cpu_read_reg(cpu,cpu->__pipe->execute->rs);
    }
    else if(cpu->__pipe->de_stall){
        if(cpu->__pipe->decode->rs1!=34){
            cpu->__pipe->decode->rs1_val=cpu_read_reg(cpu,cpu->__pipe->decode->rs1);
        }
        if(cpu->__pipe->decode->rs2!=34){
            cpu->__pipe->decode->rs2_val=cpu_read_reg(cpu,cpu->__pipe->decode->rs2);
        }
    }
}


// //useless implementation
//  void LUI_exe(RISCV_cpu* cpu, u32 inst) {
//     // LUI places upper 20 bits of U-immediate value to rd
//     cpu->x[rd(inst)] = (u64)(i64)(i32)(inst & 0xfffff000);
//     //DBG(lui)
// }
// void AUIPC_exe(RISCV_cpu* cpu, u32 inst) {
//     // AUIPC forms a 32-bit offset from the 20 upper bits 
//     // of the U-immediate
//     u64 imm = imm_U_TYPE(inst);
//     cpu->x[rd(inst)] = ((i64) cpu->pc + (i64) imm) - 4;
//     //DBG(auipc)
// }
// void JAL_exe(RISCV_cpu* cpu, u32 inst) {
//     u64 imm = imm_UJ_TYPE(inst);
//     cpu->x[rd(inst)] = cpu->pc;
//     /*print_op("JAL-> rd:%ld, pc:%lx\n", rd(inst), cpu->pc);*/
//     cpu->pc = cpu->pc + (i64) imm - 4;
//     //DBG(jal)
// // addr. misalignment consideration??
// }
// void JALR_exe(RISCV_cpu* cpu, u32 inst) {
//     u64 imm = imm_I_TYPE(inst);
//     u64 tmp = cpu->pc;
//     cpu->pc = (cpu->x[rs1(inst)] + (i64) imm) & 0xfffffffe;
//     cpu->x[rd(inst)] = tmp;
//     /*print_op("NEXT -> %#lx, imm:%#lx\n", cpu->pc, imm);*/
//     //DBG(jalr)
// // addr. misalignment consideration??
// }
// void BEQ_exe(RISCV_cpu* cpu, u32 inst) {
//     u64 imm = imm_SB_TYPE(inst);
//     if ((i64) cpu->x[rs1(inst)] == (i64) cpu->x[rs2(inst)])
//         cpu->pc = cpu->pc + (i64) imm - 4;
//     //DBG(beq)
// }
// void BNE_exe(RISCV_cpu* cpu, u32 inst) {
//     u64 imm = imm_SB_TYPE(inst);
//     if ((i64) cpu->x[rs1(inst)] != (i64) cpu->x[rs2(inst)])
//         cpu->pc = (cpu->pc + (i64) imm - 4);
//     //DBG(bne)
// }
// void BLT_exe(RISCV_cpu* cpu, u32 inst) {
//     /*print_op("Operation: BLT\n");*/
//     u64 imm = imm_SB_TYPE(inst);
//     if ((i64) cpu->x[rs1(inst)] < (i64) cpu->x[rs2(inst)])
//         cpu->pc = cpu->pc + (i64) imm - 4;
//     //DBG(blt)
// }
// void BGE_exe(RISCV_cpu* cpu, u32 inst) {
//     u64 imm = imm_SB_TYPE(inst);
//     if ((i64) cpu->x[rs1(inst)] >= (i64) cpu->x[rs2(inst)])
//         cpu->pc = cpu->pc + (i64) imm - 4;
//     //DBG(bge)
// }
// void BLTU_exe(RISCV_cpu* cpu, u32 inst) {
//     u64 imm = imm_SB_TYPE(inst);
//     if (cpu->x[rs1(inst)] < cpu->x[rs2(inst)])
//         cpu->pc = cpu->pc + (i64) imm - 4;
//     //DBG(bltu)
// }
// void BGEU_exe(RISCV_cpu* cpu, u32 inst) {
//     u64 imm = imm_SB_TYPE(inst);
//     if (cpu->x[rs1(inst)] >= cpu->x[rs2(inst)])
//         cpu->pc = (i64) cpu->pc + (i64) imm - 4;
//     //DBG(bgeu)
// }
// void LB_exe(RISCV_cpu* cpu, u32 inst) {
//     // load 1 byte to rd from address in rs1
//     u64 imm = imm_I_TYPE(inst);
//     u64 addr = cpu->x[rs1(inst)] + (i64) imm;
//     cpu->x[rd(inst)] = (i64)(i8) cpu_ld(cpu, addr, 8);
//     //DBG(lb)
// }
// void LH_exe(RISCV_cpu* cpu, u32 inst) {
//     // load 2 byte to rd from address in rs1
//     u64 imm = imm_I_TYPE(inst);
//     u64 addr = cpu->x[rs1(inst)] + (i64) imm;
//     cpu->x[rd(inst)] = (i64)(i16) cpu_ld(cpu, addr, 16);
//     //DBG(lh)
// }
// void LW_exe(RISCV_cpu* cpu, u32 inst) {
//     // load 4 byte to rd from address in rs1
//     u64 imm = imm_I_TYPE(inst);
//     u64 addr = cpu->x[rs1(inst)] + (i64) imm;
//     cpu->x[rd(inst)] = (i64)(i32) cpu_ld(cpu, addr, 32);
//     //DBG(lw)
// }
// void LBU_exe(RISCV_cpu* cpu, u32 inst) {
//     // load unsigned 1 byte to rd from address in rs1
//     u64 imm = imm_I_TYPE(inst);
//     u64 addr = cpu->x[rs1(inst)] + (i64) imm;
//     cpu->x[rd(inst)] = cpu_ld(cpu, addr, 8);
//     //DBG(lbu)
// }
// void LHU_exe(RISCV_cpu* cpu, u32 inst) {
//     // load unsigned 2 byte to rd from address in rs1
//     u64 imm = imm_I_TYPE(inst);
//     u64 addr = cpu->x[rs1(inst)] + (i64) imm;
//     cpu->x[rd(inst)] = cpu_ld(cpu, addr, 16);
//     //DBG(lhu)
// }
// void SB_exe(RISCV_cpu* cpu, u32 inst) {
//     u64 imm = imm_S_TYPE(inst);
//     u64 addr = cpu->x[rs1(inst)] + (i64) imm;
//     cpu_st(cpu, addr, 8, cpu->x[rs2(inst)]);
//     //DBG(sb)
// }
// void SH_exe(RISCV_cpu* cpu, u32 inst) {
//     u64 imm = imm_S_TYPE(inst);
//     u64 addr = cpu->x[rs1(inst)] + (i64) imm;
//     cpu_st(cpu, addr, 16, cpu->x[rs2(inst)]);
//     //DBG(sh)
// }
// void SW_exe(RISCV_cpu* cpu, u32 inst) {
//     u64 imm = imm_S_TYPE(inst);
//     u64 addr = cpu->x[rs1(inst)] + (i64) imm;
//     cpu_st(cpu, addr, 32, cpu->x[rs2(inst)]);
//     //DBG(sw)
// }
// void ADDI_exe(RISCV_cpu* cpu, u32 inst) {
//     u64 imm = imm_I_TYPE(inst);
//     cpu->x[rd(inst)] = cpu->x[rs1(inst)] + (i64) imm;
//     //DBG(addi)
// }
// void SLLI_exe(RISCV_cpu* cpu, u32 inst) {
//     cpu->x[rd(inst)] = cpu->x[rs1(inst)] << shamt(inst);
//     //DBG(slli)
// }
// void SLTI_exe(RISCV_cpu* cpu, u32 inst) {
//     u64 imm = imm_I_TYPE(inst);
//     cpu->x[rd(inst)] = (cpu->x[rs1(inst)] < (i64) imm)?1:0;
//     //DBG(slti)
// }
// void SLTIU_exe(RISCV_cpu* cpu, u32 inst) {
//     u64 imm = imm_I_TYPE(inst);
//     cpu->x[rd(inst)] = (cpu->x[rs1(inst)] < imm)?1:0;
//     //DBG(sltiu)
// }
// void XORI_exe(RISCV_cpu* cpu, u32 inst) {
//     u64 imm = imm_I_TYPE(inst);
//     cpu->x[rd(inst)] = cpu->x[rs1(inst)] ^ imm;
//     //DBG(xori)
// }
// void SRLI_exe(RISCV_cpu* cpu, u32 inst) {
//     u64 imm = imm_I_TYPE(inst);
//     cpu->x[rd(inst)] = cpu->x[rs1(inst)] >> imm;
//     //DBG(srli)
// }
// void SRAI_exe(RISCV_cpu* cpu, u32 inst) {
//     u64 imm = imm_I_TYPE(inst);
//     cpu->x[rd(inst)] = (i32)cpu->x[rs1(inst)] >> imm;
//     //DBG(srai)
// }
// void ORI_exe(RISCV_cpu* cpu, u32 inst) {
//     u64 imm = imm_I_TYPE(inst);
//     cpu->x[rd(inst)] = cpu->x[rs1(inst)] | imm;
//     //DBG(ori)
// }
// void ANDI_exe(RISCV_cpu* cpu, u32 inst) {
//     u64 imm = imm_I_TYPE(inst);
//     cpu->x[rd(inst)] = cpu->x[rs1(inst)] & imm;
//     //DBG(andi)
// }
// void ADD_exe(RISCV_cpu* cpu, u32 inst) {
//     cpu->x[rd(inst)] =
//         (u64) ((i64)cpu->x[rs1(inst)] + (i64)cpu->x[rs2(inst)]);
//     //DBG(add)
// }
// void SUB_exe(RISCV_cpu* cpu, u32 inst) {
//     cpu->x[rd(inst)] =
//         (u64) ((i64)cpu->x[rs1(inst)] - (i64)cpu->x[rs2(inst)]);
//     //DBG(sub)
// }
// void SLL_exe(RISCV_cpu* cpu, u32 inst) {
//     cpu->x[rd(inst)] = cpu->x[rs1(inst)] << (i64)cpu->x[rs2(inst)];
//     //DBG(sll)
// }
// void SLT_exe(RISCV_cpu* cpu, u32 inst) {
//     cpu->x[rd(inst)] = (cpu->x[rs1(inst)] < (i64) cpu->x[rs2(inst)])?1:0;
//     //DBG(slt)
// }
// void SLTU_exe(RISCV_cpu* cpu, u32 inst) {
//     cpu->x[rd(inst)] = (cpu->x[rs1(inst)] < cpu->x[rs2(inst)])?1:0;
//     //DBG(sltu)
// }
// void XOR_exe(RISCV_cpu* cpu, u32 inst) {
//     cpu->x[rd(inst)] = cpu->x[rs1(inst)] ^ cpu->x[rs2(inst)];
//     //DBG(xor)
// }
// void SRL_exe(RISCV_cpu* cpu, u32 inst) {
//     cpu->x[rd(inst)] = cpu->x[rs1(inst)] >> cpu->x[rs2(inst)];
//     //DBG(srl)
// }
// void SRA_exe(RISCV_cpu* cpu, u32 inst) {
//     cpu->x[rd(inst)] = (i32)cpu->x[rs1(inst)] >> 
//         (i64) cpu->x[rs2(inst)];
//     //DBG(sra)
// }
// void OR_exe(RISCV_cpu* cpu, u32 inst) {
//     cpu->x[rd(inst)] = cpu->x[rs1(inst)] | cpu->x[rs2(inst)];
//     //DBG(or)
// }
// void AND_exe(RISCV_cpu* cpu, u32 inst) {
//     cpu->x[rd(inst)] = cpu->x[rs1(inst)] & cpu->x[rs2(inst)];
//     //DBG(and)
// }