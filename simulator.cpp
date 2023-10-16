#include "cpu.hpp"
// include logging header

void start(RISCV_cpu *cpu){    
    CPU_reset(cpu);
    cpu->__pipe->cycle=0;
    while(cpu->pc!=INST_END){ // INST_END Needs to programed /Defined somewhere
        cpu_fetch(cpu);
        cpu_decode(cpu);
        cpu_execute(cpu);
        cpu_memory(cpu);
        cpu_writeback(cpu);
        cpu_pc_update(cpu);
        // statechange(cpu->__pipe);
        statechange_withoutBYPASSING(cpu->__pipe);
        // call savelog(cpu, pipe) 
        cpu->__pipe->cycle+=1;
    }
}

int main(){
    RISCV_cpu* mycpu = CPU_init(); 

    // Instruction loading needs to be done to memory 
    // mechanism of instruction end for while loop needs to be defined
    // logging needs to be configured in the start function
    
    start(mycpu);
    return 0;
}