#include "cpu.hpp"
// include logging header
#include "LogFile.cpp"

void start(RISCV_cpu *cpu,u32 INST_END){
    FILE* fp = fopen("logfile.log","a");
    if(fp==NULL){
        fprintf(stderr,"Error in opening logfile\n");
        exit(1);
    }
    fprintf(fp, "****************************************************************************\n");
    fprintf(fp, "*****************************RISCV SIMULATOR*******************************\n");
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    fprintf(fp, "Date and Time: %s\n", asctime(tm) );
    fprintf(fp, "\n");
    while(cpu->pc<INST_END){
        cpu_fetch(cpu);
        cpu_decode(cpu);
        cpu_execute(cpu);
        cpu_memory(cpu);
        cpu_writeback(cpu);
        cpu_pc_update(cpu);
        // call savelog(cpu, pipe)
        logValues(fp,cpu, cpu->__pipe);
        // statechange(cpu->__pipe);
        statechange_withoutBYPASSING(cpu->__pipe);
        cpu->__pipe->cycle+=1;
    }
    fclose(fp);
}
u32 load_Instrucctions_in_memory(RISCV_cpu* cpu,char* path){
    FILE* fp = fopen(path,"rb");
    if(fp==NULL){
        fprintf(stderr,"Error in opening binary file\n");
        exit(1);
    }
    u32  INST_END= 0;
    u32 INST = 0;
    while(fread(&INST,sizeof(u32),1,fp)){
        cpu_st(cpu, INST_END, 32, INST);// to be checked
        INST_END+=4;
    }
    fclose(fp);
    return INST_END;
}

int main(int argc, char* argv[]){

    RISCV_cpu* mycpu = CPU_init(); 
    if(argc!=2){
        fprintf(stderr,"No input to simulator: <path to bin file missing> command line arguments no-> %d\n",argc);
        exit(1);
    }
    u32 INST_END = load_Instrucctions_in_memory(mycpu,argv[1]);
    CPU_reset(mycpu);
    mycpu->__pipe->cycle=0;
    start(mycpu,INST_END);
    return 0;
}
