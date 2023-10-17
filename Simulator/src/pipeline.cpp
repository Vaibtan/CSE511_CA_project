#include "pipeline.hpp"

pipeline* pipe_init(){
    pipeline *pipe = (pipeline *)malloc(sizeof(pipeline));
    pipe->cycle = 0;
    pipe->decode = (d_unit *)malloc(sizeof(d_unit));
    pipe->fetch = (f_unit *)malloc(sizeof(f_unit));
    pipe->execute = (exec_unit *)malloc(sizeof(exec_unit));
    pipe->memory = (mem_unit *)malloc(sizeof(mem_unit));
    pipe->writeback = (wb_unit *)malloc(sizeof(wb_unit));
    // pipe->bypass = (bypassreg *)malloc(sizeof(bypassreg));
    pipeline_reset(pipe);
    return pipe;
}

// void bypass_reset(bypassreg *bypass){
//     bypass->res_exec = 0;
//     bypass->res_mem = 0;
//     bypass->rde = 34;
//     bypass->rdm = 34;
//     bypass->pcd = 0;
//     bypass->pcf = 0;
//     bypass->new_pc = 0;
// }

void decode_reset(d_unit *decode){
    decode->inst = 0;
    decode->rs1 = 34;
    decode->rs2 = 34;
    decode->rd = 34;
    decode->imm = 0;
    decode->rs1_val = 0;
    decode->rs2_val = 0;
    decode->done=false;
    decode->isimm=true;
    decode->isload=false;
    decode->isstore=false;
    decode->iswrite=false;
}

void fetch_reset(f_unit *fetch){
    fetch->inst = 0;
    fetch->done=false;
}

void execute_reset(exec_unit *execute){
    execute->inst = 0;
    execute->op1 = 0;
    execute->op2 = 0;
    execute->result = 0;
    execute->rd = 34;
    execute->rs = 34;
    execute->size = 0;
    execute->usign = false;
    execute->done=false;
    execute->isload=false;
    execute->isstore=false;
    execute->iswrite=false;
}

void memory_reset(mem_unit *memory){
    memory->addr = 0;
    memory->size = 0;
    memory->value = 0;
    memory->rd = 34;
    memory->usign = false;
    memory->done=false;
    memory->isload=false;
    memory->isstore=false;
    memory->iswrite=false;
}

void writeback_reset(wb_unit *writeback){
    writeback->rd = 0;
    writeback->imm = 0;
    writeback->done=false;
    writeback->iswrite=false;
}

void cycle_reset(pipeline *pipe){
    pipe->cycle = 0;
}

void pipeline_reset(pipeline *pipe){
    cycle_reset(pipe);
    decode_reset(pipe->decode);
    fetch_reset(pipe->fetch);
    execute_reset(pipe->execute);
    memory_reset(pipe->memory);
    writeback_reset(pipe->writeback);
    // bypass_reset(pipe->bypass);
    pipe->isbranch=false;
    pipe->newpc_offset=4;
    pipe->isjalr=false;
    pipe->branch_by_decode=false;
    pipe->de_stall=false;
    pipe->ex_stall=false;
}

void changef_to_d(pipeline *pipe){
    pipe->decode->isload=false;
    pipe->decode->isstore=false;
    pipe->decode->iswrite=false;
    pipe->decode->inst = pipe->fetch->inst;
    pipe->decode->rs1 = 34;
    pipe->decode->rs2 = 34;
    pipe->decode->isimm=true;
    pipe->decode->rd = 34;
    pipe->decode->imm = 0;
    pipe->decode->rs1_val = 0;
    pipe->decode->rs2_val = 0;
    pipe->decode->done=false;
    // pipe->bypass->pcd = pipe->bypass->pcf;
}

void changed_to_ex(pipeline *pipe){
    pipe->execute->isstore = pipe->decode->isstore;
    pipe->execute->isload = pipe->decode->isload;
    pipe->execute->iswrite = pipe->decode->iswrite;
    pipe->execute->inst = pipe->decode->inst;
    // pipe->execute->op1=bypass_read(pipe,pipe->decode,true);
    pipe->execute->op1 = pipe->decode->rs1_val;
    pipe->execute->rs = pipe->decode->rs2;
    pipe->execute->result = 0;
    if(pipe->decode->rs2!=34){
        pipe->execute->op2 = pipe->decode->rs2_val;
        // pipe->execute->op2=bypass_read(pipe,pipe->decode,false);
        if(pipe->decode->isimm){
            pipe->execute->rd = pipe->decode->imm;
        }
        else{
            pipe->execute->rd = pipe->decode->rd;
        }
    }
    else{
        pipe->execute->op2 = pipe->decode->imm;
        pipe->execute->rd = pipe->decode->rd;
    }
    pipe->execute->size = 0;
    pipe->execute->usign = false;
    pipe->execute->done=false;
}

void changeex_to_m(pipeline *pipe){
    pipe->memory->rd =pipe->execute->rd;
    pipe->memory->usign = pipe->execute->usign;
    pipe->memory->size = pipe->execute->size;
    pipe->memory->isload = pipe->execute->isload;
    pipe->memory->isstore = pipe->execute->isstore;
    pipe->memory->iswrite = pipe->execute->iswrite;
    if(pipe->memory->isstore){
        pipe->memory->addr = pipe->execute->result;
        pipe->memory->value = pipe->execute->op2;
    }
    else if(pipe->memory->isload){
        pipe->memory->addr = pipe->execute->result;
    }
    else{
        pipe->memory->value = pipe->execute->result;
    }
    pipe->memory->done = false;
}

void changem_to_wb(pipeline *pipe){
    pipe->writeback->rd = pipe->memory->rd;
    pipe->writeback->imm = pipe->memory->value;
    pipe->writeback->iswrite = pipe->memory->iswrite;
    pipe->writeback->done = false;
}

// reset the branch flags
void reset_branchflags(pipeline *pipe){
    pipe->isbranch=false;
    pipe->newpc_offset=4;
    pipe->isjalr=false;
    pipe->branch_by_decode=false;
}

//to try state change
void try_statechange(pipeline* pipe){
    changem_to_wb(pipe);
    memory_reset(pipe->memory);
    if(!pipe->ex_stall){
        changeex_to_m(pipe);
        execute_reset(pipe->execute);
        if(!pipe->de_stall){
            changed_to_ex(pipe);
            changef_to_d(pipe);
        }
    }
}

//to detect and kill the pipeline
void jump_withoutBYPASSING(pipeline*pipe){
    if(pipe->isbranch){
        if(pipe->branch_by_decode){
            fetch_reset(pipe->fetch);
        }
        else{
            fetch_reset(pipe->fetch);
            decode_reset(pipe->decode);
        }
    }
    reset_branchflags(pipe);
}

// void jump(pipeline*pipe){
//     if(pipe->isbranch){
//         if(pipe->branch_by_decode){
//             if(pipe->bypass->new_pc!=pipe->bypass->pcf)
//             fetch_reset(pipe->fetch);
//         }
//         else{
//             if(pipe->bypass->new_pc!=pipe->bypass->pcd){
//                 decode_reset(pipe->decode);
//                 if(pipe->bypass->new_pc!=pipe->bypass->pcf)
//                 fetch_reset(pipe->fetch);
//             }
//         }
//     }
//     reset_branchflags(pipe);
// }

// to stall the pipeline
void stall_withoutBYPASSING(pipeline*pipe){
    if(pipe->execute->isstore && pipe->memory->rd==pipe->execute->rs){
        pipe->ex_stall=true;
    }
    else{
        pipe->ex_stall=false;
        if(pipe->memory->rd==pipe->decode->rs1||pipe->memory->rd==pipe->decode->rs2){
            pipe->de_stall=true;
        }
        else
        pipe->de_stall=false;
    }
}

// void stall(pipeline*pipe){
//     if(pipe->execute->isload &&(pipe->execute->rd==pipe->decode->rs1||pipe->execute->rd==pipe->decode->rs2))
//         pipe->de_stall=true;
//     else
//     pipe->de_stall=false;
//     pipe->ex_stall=false;
// }

// state change functions
void statechange_withoutBYPASSING(pipeline* pipe){
    jump_withoutBYPASSING(pipe);
    stall_withoutBYPASSING(pipe);
    try_statechange(pipe);
}

// void statechange(pipeline* pipe){
//     jump(pipe);
//     stall(pipe);
//     try_statechange(pipe);
// }

//Bypass register's reading function
// u32 bypass_read(pipeline *pipe,struct d_unit *decode,bool isrs1){
//     if(isrs1){
//     if(pipe->bypass->rde==decode->rs1){
//         return pipe->bypass->res_exec;
//     }
//     else if(pipe->bypass->rdm==decode->rs1){
//         return pipe->bypass->res_mem;
//     }
//     return decode->rs1_val;
//     }
//     else{
//     if(pipe->bypass->rde==decode->rs2){
//         return pipe->bypass->res_exec;
//     }
//     else if(pipe->bypass->rdm==decode->rs2){
//         return pipe->bypass->res_mem;
//     }
//     return decode->rs2_val;
//     }
// }


// ignore (useless implementation) 
//  //   ideal pipeline state change function Implementation 
// void changestate(RISCV_cpu *cpu){
    // cpu->__pipe->writeback->rd = cpu->__pipe->memory->rd;
    // cpu->__pipe->writeback->imm = cpu->__pipe->memory->value;
    // cpu->__pipe->memory->rd =cpu->__pipe->execute->rd;
    // cpu->__pipe->memory->usign = cpu->__pipe->execute->usign;
    // cpu->__pipe->memory->size = cpu->__pipe->execute->size;
    // if(cpu->__pipe->isstore&0x4==0x4){
    //     cpu->__pipe->memory->addr = cpu->__pipe->execute->result;
    //     cpu->__pipe->memory->value = cpu->__pipe->execute->op2;
    // }
    // else if(cpu->__pipe->isload&0x4==0x4){
    //     cpu->__pipe->memory->addr = cpu->__pipe->execute->result;
    // }
    // else{
    //     cpu->__pipe->memory->value = cpu->__pipe->execute->result;
    // }
    // cpu->__pipe->execute->inst = cpu->__pipe->decode->inst;
    // cpu->__pipe->execute->op1 = cpu->__pipe->decode->rs1_val;
    // cpu->__pipe->execute->size = 0;
    // cpu->__pipe->execute->usign = false;
    // cpu->__pipe->execute->result = 0;
    // if(cpu->__pipe->decode->rs2!=34){
    //     cpu->__pipe->execute->op2 = cpu->__pipe->decode->rs2_val;
    //     if(cpu->__pipe->decode->isimm){
    //         cpu->__pipe->execute->rd = cpu->__pipe->decode->imm;
    //     }
    //     else{
    //         cpu->__pipe->execute->rd = cpu->__pipe->decode->rd;
    //     }
    // }
    // else{
    //     cpu->__pipe->execute->op2 = cpu->__pipe->decode->imm;
    //     cpu->__pipe->execute->rd = cpu->__pipe->decode->rd;
    // }
    // cpu->__pipe->decode->inst = cpu->__pipe->fetch->inst;
    // cpu->__pipe->decode->isimm = true;
    // cpu->__pipe->decode->rs2==34;
    // cpu->__pipe->decode->rs1==34;
// }
