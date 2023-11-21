#include "LogFile.hpp"
#include <stdio.h>
#include <iostream>

void printMemory(RISCV_cpu* cpu, FILE* file) {
    if (cpu->_pipe->memory->isload || cpu->_pipe->memory->isstore) {
        fprintf(file, "Memory that was updated\n");
        fprintf(file, "Addr: %u\n", cpu->__pipe->memory->addr);
        fprintf(file, "Value: %d\n", (int)(cpu->_bus->d_cache_bus->data_cache->_bus->data_bus->data_mem->rv32i_data_mem[cpu->_pipe->memory->addr]));
    }
}

void dataMemory(RISCV_cpu* cpu, FILE* file){
    // data memory just stores the information
        if (cpu->__pipe->memory->isstore) {
            fprintf(file, "Using Data Memory pattern \n");
            fprintf(file, "Addr: %u\n", cpu->__pipe->memory->addr);
            fprintf(file, "Value: %d\n", (int)(cpu->_bus->d_cache_bus->data_cache->_bus->data_bus->data_mem->rv32i_data_mem[cpu->_pipe->memory->addr]));
        }
}

void instrMemory(RISCV_cpu* cpu, FILE* file){
    // data memory just stores the information
        if (!cpu->__pipe->memory->isstore) {
            fprintf(file, "Using Instruction Memory\n");
            fprintf(file, "Addr: %u\n", cpu->__pipe->memory->addr);
            fprintf(file, "Value: %d\n", (int)(cpu->_bus->i_cache_bus->instr_cache->_bus->instr_bus->instr_mem->rv32i_instr_mem[cpu->_pipe->memory->addr]));
        }
}


void save_counters_to_file(FILE* file, RISCV_cpu* cpu) {
    fprintf(file, "Register Instructions: %u\n", cpu->register_instr_counter);
    fprintf(file, "Memory Instructions: %u\n", cpu->memory_instr_counter);
}

void save_counters(RISCV_cpu* cpu) {
    FILE *counter_file = fopen("../logs/counters.txt", "w");
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

void pipelineValues(FILE* file, RISCV_cpu cpu, pipeline pipe) {
    
    fprintf(file, "\n");
    fprintf(file, "Pipeline State:\n");

    fprintf(file, "IF: Instr 0x%x\n", pipe->fetch->inst);
    fprintf(file, "ID: Instr 0x%x\n", pipe->decode->inst);
    fprintf(file, "EX: Instr 0x%x\n", pipe->execute->inst);
    fprintf(file, "MEM: Instr 0x%x\n", pipe->memory->inst);
    fprintf(file, "WB: Instr 0x%x\n", pipe->writeback->inst);
    
    fprintf(file, "\n");
    fprintf(file, "**************************\n");
}

int isStall(FILE* file, RISCV_cpu cpu, pipeline pipe) {
    int stallCounter = 0;

    fprintf(file, "\nStall Status:");

    if (pipe->de_stall) {
        fprintf(file, "Decode stage is stalled\n");
        stallCounter++;
    }

    if (pipe->ex_stall) {
        fprintf(file, "Execute stage is stalled\n");
        stallCounter++;

    } else if (!(pipe->de_stall || pipe->ex_stall)) {
        fprintf(file, "0");
    }
    fprintf(file, "\n");
    return stallCounter;
}

void save_total_stalls_to_file(FILE* file, int totalStallCounter) {
    fprintf(file, "Total number of stalls: %d\n", totalStallCounter);
}

void logValues(FILE* file, RISCV_cpu cpu, pipeline pipe) {
    fprintf(file, "\n");
    fprintf(file, "\nCycle %d:\n", pipe->cycle);
    fprintf(file, "\n");

    fprintf(file, "CPU Context:\n");
    RegValues(file, cpu);

    pipelineValues(file, cpu, pipe);

    int totalStalls = isStall(file, cpu, pipe);
    
    printMemory(cpu, file);
    save_counters(cpu);
    save_total_stalls_to_file(file, totalStalls);
    
    instrMemory(cpu, file);
    dataMemory(cpu, file);
}




/**
#include "LogFile.hpp"
#include <stdio.h>
#include <iostream>

void printMemory(RISCV_cpu* cpu) {
    // detect 
    if (cpu->__pipe->memory->isload == true || cpu->__pipe->memory->isstore == true){
        std::cout << "Addr" << cpu->__pipe->memory->addr << std::endl;
        std::cout << "Value" << (int)cpu->__bus->d_cache_bus->data_cache->_bus->data_bus->data_mem->rv32i_data_mem[cpu->__pipe->memory->addr] << std::endl;
    }
}


void save_counters_to_file(FILE* file, RISCV_cpu* cpu) {
    fprintf(file, "Register Instructions: %u\n", cpu->register_instr_counter);
    fprintf(file, "Memory Instructions: %u\n", cpu->memory_instr_counter);
}

void save_counters(RISCV_cpu* cpu) {
    FILE *counter_file = fopen("../logs/counters.txt", "w");
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

int isStall(FILE* file, RISCV_cpu *cpu, pipeline* pipe) {
    int stallCounter = 0;

    fprintf(file, "\nStall Status:");

    if (pipe->de_stall) {
        fprintf(file, "Decode stage is stalled\n");
        stallCounter++;
    }

    if (pipe->ex_stall) {
        fprintf(file, "Execute stage is stalled\n");
        stallCounter++;
    } else if (!(pipe->de_stall || pipe->ex_stall)) {
        fprintf(file, "0");
    }
    fprintf(file, "\n");
    return stallCounter;
}

void save_total_stalls_to_file(FILE* file, int totalStalls) {
    fprintf(file, "Total number of stalls: %d\n", totalStalls);
}

void logValues(FILE* file, RISCV_cpu *cpu, pipeline* pipe) {
    fprintf(file, "Cycle %d:\n", pipe->cycle);
    fprintf(file, "\n");

    fprintf(file, "CPU Context:\n");
    RegValues(file, cpu);

    pipelineValues(file, cpu, pipe);

    int totalStalls = isStall(file, cpu, pipe);

    save_counters(cpu);
    save_total_stalls_to_file(file, totalStalls);
    printMemory(cpu);
}
*/
