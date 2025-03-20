#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include "shell.h"

void decode_adds_extended(uint32_t instruction);
void decode_adds_immediate(uint32_t instruction);
void decode_subs_extended(uint32_t instruction);
void decode_subs_immediate(uint32_t instruction);
void decode_halt(uint32_t instruction);
void decode_cmp_immediate(uint32_t instruction);
void decode_cmp_extended(uint32_t instruction);
void decode_ands(uint32_t instruction);
void decode_eor(uint32_t instruction);
void decode_orr(uint32_t instruction);

void process_instruction()
{
    /* execute one instruction here. You should use CURRENT_STATE and modify
     * values in NEXT_STATE. You can call mem_read_32() and mem_write_32() to
     * access memory. 
     * */

    void decode_instruction();
    void execute_instruction();
}


typedef struct instruction_information
{
    /* data */
    uint32_t opcode;
    void* function;
} inst_info; 

// FALTAN MAS INSTRUCCIONES

inst_info INSTRUCTION_SET[] = {
    {0b10101011000, &decode_adds_extended},
    {0b10110001, &decode_adds_immediate},
    {0b11101011000, &decode_subs_extended},
    {0b11110001, &decode_subs_immediate},
    {0b11010100010, &decode_halt},
    {0b11110001, &decode_cmp_immediate},
    {0b11101011001, &decode_cmp_extended}, // ULTIMO DIGITO 0 ==> IGUAL A SUBS
    {0b11101010000, &decode_ands},
    {0b11001010000, &decode_eor},
    {0b10101010000, &decode_orr},
};

#define INSTRUCTION_SET_SIZE (sizeof(INSTRUCTION_SET) / sizeof(INSTRUCTION_SET[0]))

void decode_adds_extended(uint32_t instruction){
    // Get the register numbers
    uint32_t rd = (instruction >> 0) & 0b11111;
    uint32_t rn = (instruction >> 5) & 0b11111;
    uint32_t rm = (instruction >> 16) & 0b11111;

    // Get imm3
    uint32_t imm3 = (instruction >> 10) & 0b111;

    // Get option
    uint32_t option = (instruction >> 13) & 0b111;

    // Add the values
    uint64_t result = CURRENT_STATE.REGS[rn] + CURRENT_STATE.REGS[rm];

    // Update the flags
    NEXT_STATE.FLAG_N = (result >> 63) & 0b1;
    NEXT_STATE.FLAG_Z = (result == 0) ? 1 : 0;

    // Update the register
    NEXT_STATE.REGS[rd] = result;

    // Update the PC
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void decode_subs_extended(uint32_t instruction){
    // Get the register numbers
    uint32_t rd = (instruction >> 0) & 0b11111;
    uint32_t rn = (instruction >> 5) & 0b11111;
    uint32_t rm = (instruction >> 16) & 0b11111;

    // Get imm3
    uint32_t imm3 = (instruction >> 10) & 0b111;

    // Get option
    uint32_t option = (instruction >> 13) & 0b111;

    // Subtract the values
    uint64_t result = CURRENT_STATE.REGS[rn] - CURRENT_STATE.REGS[rm];

    // Update the flags
    NEXT_STATE.FLAG_N = (result >> 63) & 0b1;
    NEXT_STATE.FLAG_Z = (result == 0) ? 1 : 0;

    // Update the register
    NEXT_STATE.REGS[rd] = result;

    // Update the PC
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void decode_adds_immediate(uint32_t instruction){
    // Get the register numbers
    uint32_t rd = (instruction >> 0) & 0b11111;
    uint32_t rn = (instruction >> 5) & 0b11111;
    
    // Get the immediate value
    uint32_t imm12 = (instruction >> 10) & 0b111111111111;

    // Check for shift
    uint32_t shift = (instruction >> 22) & 0b11;
    switch (shift) {
        case 0x0: {
            // Handle case shift 00
            
            // Add the values
            uint64_t result = CURRENT_STATE.REGS[rn] + imm12;

            // Update the flags
            NEXT_STATE.FLAG_N = (result >> 63) & 0b1;
            NEXT_STATE.FLAG_Z = (result == 0) ? 1 : 0;

            // Update the register
            NEXT_STATE.REGS[rd] = result;

            // Update the PC
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    
            break;
        }
        case 0x1: {
            // Handle case 01
            
            // Shift the immediate value
            uint64_t shifted_imm12 = imm12 << 12;

            // Add the values
            uint64_t result = CURRENT_STATE.REGS[rn] + shifted_imm12;

            // Update the flags
            NEXT_STATE.FLAG_N = (result >> 63) & 0b1;
            NEXT_STATE.FLAG_Z = (result == 0) ? 1 : 0;

            // Update the register
            NEXT_STATE.REGS[rd] = result;

            // Update the PC
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }

        default: {
            // Handle unexpected cases
            printf("Unexpected shift value\n");
            break;
        }
    }
}


void decode_subs_immediate(uint32_t instruction){
    // Get the register numbers
    uint32_t rd = (instruction >> 0) & 0b11111;
    uint32_t rn = (instruction >> 5) & 0b11111;
    
    // Get the immediate value
    uint32_t imm12 = (instruction >> 10) & 0b111111111111;

    // Check for shift
    uint32_t shift = (instruction >> 22) & 0b11;
    switch (shift) {
        case 0x0: {
            // Handle case shift 00
            
            // Substract the values
            uint64_t result = CURRENT_STATE.REGS[rn] - imm12;

            // Update the flags
            NEXT_STATE.FLAG_N = (result >> 63) & 0b1;
            NEXT_STATE.FLAG_Z = (result == 0) ? 1 : 0;

            // Update the register
            NEXT_STATE.REGS[rd] = result;

            // Update the PC
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    
            break;
        }
        case 0x1: {
            // Handle case 01
            
            // Shift the immediate value
            uint64_t shifted_imm12 = imm12 << 12;

            // Substract the values
            uint64_t result = CURRENT_STATE.REGS[rn] - shifted_imm12;

            // Update the flags
            NEXT_STATE.FLAG_N = (result >> 63) & 0b1;
            NEXT_STATE.FLAG_Z = (result == 0) ? 1 : 0;

            // Update the register
            NEXT_STATE.REGS[rd] = result;

            // Update the PC
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }

        default: {
            // Handle unexpected cases
            printf("Unexpected shift value\n");
            break;
        }
    }
}

void decode_halt(uint32_t instruction){
    // Set the run bit to zero
    RUN_BIT = 0;
}   

void decode_cmp_immediate(uint32_t instruction){
    // Get the register numbers
    uint32_t rn = (instruction >> 5) & 0b11111;
    
    // Get the immediate value
    uint32_t imm12 = (instruction >> 10) & 0b111111111111;

    // Check for shift
    uint32_t shift = (instruction >> 22) & 0b11;
    switch (shift) {
        case 0x0: {
            // Handle case shift 00
            
            // Substract the values
            uint64_t result = CURRENT_STATE.REGS[rn] - imm12;

            // Update the flags
            NEXT_STATE.FLAG_N = (result >> 63) & 0b1;
            NEXT_STATE.FLAG_Z = (result == 0) ? 1 : 0;

            // Update the PC
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    
            break;
        }

        case 0x1: {
            // Handle case 01
            
            // Shift the immediate value
            uint64_t shifted_imm12 = imm12 << 12;

            // Substract the values
            uint64_t result = CURRENT_STATE.REGS[rn] - shifted_imm12;

            // Update the flags
            NEXT_STATE.FLAG_N = (result >> 63) & 0b1;
            NEXT_STATE.FLAG_Z = (result == 0) ? 1 : 0;

            // Update the PC
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }

        default: {
            // Handle unexpected cases
            printf("Unexpected shift value\n");
            break;
        }
    }
}

void decode_cmp_extended(uint32_t instruction){
    // Get the register numbers
    uint32_t rn = (instruction >> 5) & 0b11111;
    uint32_t rm = (instruction >> 16) & 0b11111;

    // Get imm3
    uint32_t imm3 = (instruction >> 10) & 0b111;

    // Get option
    uint32_t option = (instruction >> 13) & 0b111;

    // Subtract the values
    uint64_t result = CURRENT_STATE.REGS[rn] - CURRENT_STATE.REGS[rm];

    // Update the flags
    NEXT_STATE.FLAG_N = (result >> 63) & 0b1;
    NEXT_STATE.FLAG_Z = (result == 0) ? 1 : 0;

    // Update the PC
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void decode_ands(uint32_t instruction){
    // Get the register numbers
    uint32_t rd = (instruction >> 0) & 0b11111;
    uint32_t rn = (instruction >> 5) & 0b11111;
    uint32_t rm = (instruction >> 16) & 0b11111;

    // Get imm6
    uint32_t imm6 = (instruction >> 10) & 0b111111;

    // And the values
    uint64_t result = CURRENT_STATE.REGS[rn] & CURRENT_STATE.REGS[rm];

    // Update the flags
    NEXT_STATE.FLAG_N = (result >> 63) & 0b1;
    NEXT_STATE.FLAG_Z = (result == 0) ? 1 : 0;

    // Update the register
    NEXT_STATE.REGS[rd] = result;

    // Update the PC
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void decode_eor(uint32_t instruction){

    // Get the register numbers
    uint32_t rd = (instruction >> 0) & 0b11111;
    uint32_t rn = (instruction >> 5) & 0b11111;
    uint32_t rm = (instruction >> 16) & 0b11111;

    // Get imm6
    uint32_t imm6 = (instruction >> 10) & 0b111111;

    // Eor the values
    uint64_t result = CURRENT_STATE.REGS[rn] ^ CURRENT_STATE.REGS[rm];

    // Update the flags
    NEXT_STATE.FLAG_N = (result >> 63) & 0b1;
    NEXT_STATE.FLAG_Z = (result == 0) ? 1 : 0;

    // Update the register
    NEXT_STATE.REGS[rd] = result;

    // Update the PC
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void decode_orr(uint32_t instruction){
    // Get the register numbers
    uint32_t rd = (instruction >> 0) & 0b11111;
    uint32_t rn = (instruction >> 5) & 0b11111;
    uint32_t rm = (instruction >> 16) & 0b11111;

    // Get imm6
    uint32_t imm6 = (instruction >> 10) & 0b111111;

    // Or the values
    uint64_t result = CURRENT_STATE.REGS[rn] | CURRENT_STATE.REGS[rm];

    // Update the flags
    NEXT_STATE.FLAG_N = (result >> 63) & 0b1;
    NEXT_STATE.FLAG_Z = (result == 0) ? 1 : 0;

    // Update the register
    NEXT_STATE.REGS[rd] = result;

    // Update the PC
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void decode_instruction()
{
    /* decode the instruction located at CURRENT_STATE.PC. You have to read
     * the instruction, decode it, and update the values in NEXT_STATE. */

    uint32_t instruction = mem_read_32(CURRENT_STATE.PC);

    // Get opcode (31-21)
    uint32_t opcode = (instruction >> 21) & 0x7FF;

    // Compare opcode with instruction set
    for (int i = 0; i < INSTRUCTION_SET_SIZE; i++) {
        if (INSTRUCTION_SET[i].opcode == opcode) {
            // Call the corresponding function
            void (*decode_function)(uint32_t) = INSTRUCTION_SET[i].function;
            decode_function(instruction);
            return;
        }
    }

    // If no match is found
    printf("Unknown instruction with opcode: 0x%X\n", opcode);
}

