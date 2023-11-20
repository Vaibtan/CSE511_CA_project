#include "LogFile.hpp"

void memstate(FILE* file, RISCV_cpu* cpu, pipeline* pipe) {
    mem_unit* mem_stage = pipe->memory;

    // memory stage is done
    if (mem_stage->done) {
        // Open a file to store memory access pattern
        FILE* memory_access_file = fopen("memory_access_pattern.txt", "w");
        if (memory_access_file != NULL) {
            fprintf(memory_access_file, "Memory Location,Data\n");

            // Print memory information for the entire memory range
            for (int i = 0; i < RISCV_MEM_SIZE; i++) {
                fprintf(memory_access_file, "0x%x,%u\n", i, cpu->__bus->data_bus->data_mem->rv32i_data_mem[i]);
            }

            // Print memory information for the specific address and data in the memory stage
            fprintf(memory_access_file, "\nMemory Stage Information:\n");
            fprintf(memory_access_file, "Address: 0x%x\n", mem_stage->addr);
            fprintf(memory_access_file, "Data   : 0x%x\n", mem_stage->value);

            fclose(memory_access_file);
        }
    }
}

void save_counters_to_file(FILE* file, RISCV_cpu* cpu) {
    fprintf(file, "Register Instructions: %u\n", cpu->register_instr_counter);
    fprintf(file, "Memory Instructions: %u\n", cpu->memory_instr_counter);
}

void save_counters(RISCV_cpu* cpu) {
    FILE *counter_file = fopen("counters.txt", "w");
    save_counters_to_file(counter_file, cpu);
    fclose(counter_file);
}


void RegValues(FILE* file, RISCV_cpu* cpu){
    fprintf(file, "PC: 0x%x\n", cpu->pc);
    fprintf(file, "The current value of registers:\n");
    for(int i=0;i<REG_LEN;i++){
        fprintf(file, "Register %d: 0x%x\n", i, cpu->x[i]);
    }
}

void pipelineValues(FILE* file, RISCV_cpu *cpu, pipeline* pipe) {
    
    fprintf(file, "\n");
    fprintf(file, "Pipeline State:\n");

    fprintf(file, "IF: Instr 0x%x\n", pipe->fetch->inst);
    fprintf(file, "ID: Instr 0x%x\n", pipe->decode->inst);
    fprintf(file, "EX: Instr 0x%x\n", pipe->execute->inst);
    fprintf(file, "MEM: Instr 0x%x\n", pipe->memory->inst);
    fprintf(file, "WB: Instr 0x%x\n", pipe->writeback->inst);
    
    fprintf(file, "\n");
    fprintf(file, "****************************************************************************\n");
}

void isStall(FILE* file, RISCV_cpu *cpu, pipeline* pipe){

    fprintf(file, "\nStall Status:\n");

    if (pipe->de_stall) {
        fprintf(file, "Decode stage is stalled\n");
    }

    if (pipe->ex_stall) {
        fprintf(file, "Execute stage is stalled\n");
    }
    fprintf(file, "\n");
}


void logValues(FILE* file, RISCV_cpu *cpu, pipeline* pipe) {

    fprintf(file, "Cycle %d:\n", pipe->cycle);
    fprintf(file, "\n");

    fprintf(file, "CPU Context:\n");

    RegValues(file, cpu);

    pipelineValues(file, cpu, pipe);

    isStall(file, cpu, pipe);

    save_counters(cpu);
}
