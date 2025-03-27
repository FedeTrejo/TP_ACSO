#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include "shell.h"

void decode_instruction();
void decode_adds_extended(uint32_t instruction);
void decode_adds_immediate(uint32_t instruction);
void decode_subs_extended(uint32_t instruction);
void decode_subs_immediate(uint32_t instruction);
void decode_halt(uint32_t instruction);
void decode_ands(uint32_t instruction);
void decode_eor(uint32_t instruction);
void decode_orr(uint32_t instruction);
void decode_branch(uint32_t instruction);
void decode_branch_to_register(uint32_t instruction);
void decode_bcond(uint32_t instruction);
void decode_beq(int32_t imm19);
void decode_bne(int32_t imm19);
void decode_bgt(int32_t imm19);
void decode_blt(int32_t imm19);
void decode_bge(int32_t imm19);
void decode_ble(int32_t imm19);
void decode_ls(uint32_t instruction);
void decode_lsl(uint32_t instruction);
void decode_lsr(uint32_t instruction);
void decode_stur(uint32_t instruction);
void decode_sturb(uint32_t instruction);
void decode_sturh(uint32_t instruction);
void decode_ldur(uint32_t instruction);
void decode_ldurb(uint32_t instruction);
void decode_ldurh(uint32_t instruction);
void decode_movz(uint32_t instruction);
void decode_add_extended(uint32_t instruction);
void decode_add_immediate(uint32_t instruction);
void decode_mul(uint32_t instruction);
void decode_cbz(uint32_t instruction);
void decode_cbnz(uint32_t instruction);


void process_instruction()
{
    /* execute one instruction here. You should use CURRENT_STATE and modify
     * values in NEXT_STATE. You can call mem_read_32() and mem_write_32() to
     * access memory. 
     * */
    printf("Processing instruction\n");
    decode_instruction();
}

typedef struct instruction_information
{
    /* data */
    uint32_t opcode;
    void* function;
} inst_info; 

inst_info INSTRUCTION_SET[] = {
    {0b10101011000, &decode_adds_extended},
    {0b10110001, &decode_adds_immediate},
    {0b11101011000, &decode_subs_extended},
    {0b11110001, &decode_subs_immediate},
    {0b11010100010, &decode_halt},
    {0b11101010000, &decode_ands},
    {0b11001010000, &decode_eor},
    {0b10101010000, &decode_orr},
    {0b000101, &decode_branch},
    {0b11010110000, &decode_branch_to_register},
    {0b01010100, &decode_bcond},
    {0b1101001101, &decode_ls},
    {0b11111000000, &decode_stur},
    {0b00111000000, &decode_sturb},
    {0b01111000000, &decode_sturh},
    {0b11111000010, &decode_ldur},
    {0b01111000010, &decode_ldurh},
    {0b00111000010, &decode_ldurb},
    {0b11010010100, &decode_movz},
    {0b10001011001, &decode_add_extended},
    {0b10010001, &decode_add_immediate},
    {0b10011011000, &decode_mul},
    {0b10110100, &decode_cbz},
    {0b10110101, &decode_cbnz}
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
    printf("Decoding subs extended\n");
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

    // Check if the register is not xzr
    if (rd != 0b11111) {
        // Update the register
        NEXT_STATE.REGS[rd] = result;
    }
    
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
    printf("Decoding subs immediate\n");
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

            // Check if the register is not xzr
            if (rd != 0b11111) {
                // Update the register
                NEXT_STATE.REGS[rd] = result;
            }

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

            // Check if the register is not xzr
            if (rd != 0b11111) {
                // Update the register
                NEXT_STATE.REGS[rd] = result;
            }

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
    printf("Decoding eor\n");
    // Get the register numbers
    uint32_t rd = (instruction >> 0) & 0b11111;
    uint32_t rn = (instruction >> 5) & 0b11111;
    uint32_t rm = (instruction >> 16) & 0b11111;

    // Get imm6
    uint32_t imm6 = (instruction >> 10) & 0b111111;

    // Eor the values
    uint64_t result = CURRENT_STATE.REGS[rn] ^ CURRENT_STATE.REGS[rm];

    // // Update the flags
    // NEXT_STATE.FLAG_N = (result >> 63) & 0b1;
    // NEXT_STATE.FLAG_Z = (result == 0) ? 1 : 0;

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

    // // Update the flags
    // NEXT_STATE.FLAG_N = (result >> 63) & 0b1;
    // NEXT_STATE.FLAG_Z = (result == 0) ? 1 : 0;

    // Update the register
    NEXT_STATE.REGS[rd] = result;

    // Update the PC
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void decode_branch(uint32_t instruction) {
    // Extract the 26-bit immediate from the instruction
    int32_t imm26 = instruction & 0b11111111111111111111111111; // Get bits [25:0]
    
    // Sign-extend from 26 bits to 32 bits
    if (imm26 & 0b1000000000000000000000000) { // Check if bit 25 is set (negative number)
        imm26 |= 0b11111111111111111111111111000000; // Extend with 1's
    }
    
    // Multiply by 4 (as mentioned in the second image)
    // "Its offset from the address of this instruction is encoded as 'imm26' times 4"
    int32_t offset = imm26 * 4;
    
    // Update PC - the target is PC-relative
    NEXT_STATE.PC = CURRENT_STATE.PC + offset;
}

void decode_branch_to_register(uint32_t instruction) {
    // Get the register number
    uint32_t rn = (instruction >> 5) & 0b11111;

    // Get the register value
    uint64_t target = CURRENT_STATE.REGS[rn];

    // Update PC - the target is PC-relative
    NEXT_STATE.PC = target;
}

void decode_bcond(uint32_t instruction){
    // Get the condition
    uint32_t cond = (instruction >> 0) & 0b1111;

    // Extract the 19-bit immediate from the instruction 
    int32_t imm19 = (instruction >> 5) & 0b1111111111111111111;

     
    switch (cond)
    {
    case 0b0000: // BEQ
        decode_beq(imm19);
        break;
    case 0b0001: // BNE
        decode_bne(imm19);
        break;
    case 0b1100: // BGT
        decode_bgt(imm19);
        break;
    case 0b1011: // BLT
        decode_blt(imm19);
        break;
    case 0b1010: // BGE
        decode_bge(imm19);
        break;
    case 0b1101: // BLE
        decode_ble(imm19);
        break;
    default:
        break;
    }
}

void decode_beq(int32_t imm19) {
    if (CURRENT_STATE.FLAG_Z == 1) {
        // Sign-extend from 19 bits to 32 bits
        if (imm19 & 0b1000000000000000000) { // Check if bit 18 is set (negative number)
            imm19 |= 0b11111111111111111111100000000000; // Extend with 1's
        }
        
        // Multiply by 4 (as mentioned in the second image)
        // "Its offset from the address of this instruction is encoded as 'imm19' times 4"
        int32_t offset = imm19 * 4;
        
        // Update PC - the target is PC-relative
        NEXT_STATE.PC = CURRENT_STATE.PC + offset;
    }
    else {
        NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    }
}

void decode_bne(int32_t imm19) {
    if (CURRENT_STATE.FLAG_Z == 0) {
        // Sign-extend from 19 bits to 32 bits
        if (imm19 & 0b1000000000000000000) { // Check if bit 18 is set (negative number)
            imm19 |= 0b11111111111111111111100000000000; // Extend with 1's
        }
        
        // Multiply by 4 (as mentioned in the second image)
        // "Its offset from the address of this instruction is encoded as 'imm19' times 4"
        int32_t offset = imm19 * 4;
        
        // Update PC - the target is PC-relative
        NEXT_STATE.PC = CURRENT_STATE.PC + offset;
    }
    else {
        NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    }
}

void decode_bgt(int32_t imm19) {
    if (CURRENT_STATE.FLAG_N == 0 && CURRENT_STATE.FLAG_Z == 0) {
        // Sign-extend from 19 bits to 32 bits
        if (imm19 & 0b1000000000000000000) { // Check if bit 18 is set (negative number)
            imm19 |= 0b11111111111111111111100000000000; // Extend with 1's
        }
        
        // Multiply by 4 (as mentioned in the second image)
        // "Its offset from the address of this instruction is encoded as 'imm19' times 4"
        int32_t offset = imm19 * 4;
        
        // Update PC - the target is PC-relative
        NEXT_STATE.PC = CURRENT_STATE.PC + offset;
    }
    else {
        NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    }
}

void decode_blt(int32_t imm19) {
    if (CURRENT_STATE.FLAG_N == 1) {
        // Sign-extend from 19 bits to 32 bits
        if (imm19 & 0b1000000000000000000) { // Check if bit 18 is set (negative number)
            imm19 |= 0b11111111111111111111100000000000; // Extend with 1's
        }
        
        // Multiply by 4 (as mentioned in the second image)
        // "Its offset from the address of this instruction is encoded as 'imm19' times 4"
        int32_t offset = imm19 * 4;
        
        // Update PC - the target is PC-relative
        NEXT_STATE.PC = CURRENT_STATE.PC + offset;
    }
    else {
        NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    }
}

void decode_bge(int32_t imm19) {
    if (CURRENT_STATE.FLAG_N == 0) {
        // Sign-extend from 19 bits to 32 bits
        if (imm19 & 0b1000000000000000000) { // Check if bit 18 is set (negative number)
            imm19 |= 0b11111111111111111111100000000000; // Extend with 1's
        }
        
        // Multiply by 4 (as mentioned in the second image)
        // "Its offset from the address of this instruction is encoded as 'imm19' times 4"
        int32_t offset = imm19 * 4;
        
        // Update PC - the target is PC-relative
        NEXT_STATE.PC = CURRENT_STATE.PC + offset;
    }
    else {
        NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    }
}

void decode_ble(int32_t imm19) {
    if (CURRENT_STATE.FLAG_N == 1 || CURRENT_STATE.FLAG_Z == 1) {
        // Sign-extend from 19 bits to 32 bits
        if (imm19 & 0b1000000000000000000) { // Check if bit 18 is set (negative number)
            imm19 |= 0b11111111111111111111100000000000; // Extend with 1's
        }
        
        // Multiply by 4 (as mentioned in the second image)
        // "Its offset from the address of this instruction is encoded as 'imm19' times 4"
        int32_t offset = imm19 * 4;
        
        // Update PC - the target is PC-relative
        NEXT_STATE.PC = CURRENT_STATE.PC + offset;
    }
    else {
        NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    }
}

void decode_ls(uint32_t instruction){
    // Get the imms [15:10]
    uint32_t imms = (instruction >> 10) & 0b111111;
    switch (imms)
    {
    case 0b111111:
        decode_lsr(instruction);
        break;
    default:
        decode_lsl(instruction);
        break;
    }
}

void decode_lsl(uint32_t instruction){
    // Obtener los registros destino y origen
    uint32_t rd = (instruction >> 0) & 0b11111;
    uint32_t rn = (instruction >> 5) & 0b11111;

    // Extraer los campos immr
    uint32_t immr = (instruction >> 16) & 0b111111;
    
    // Calcular el shift amount correcto
    uint32_t shift_amount = (64 - immr) % 64;

    // Realizar el desplazamiento lógico a la izquierda
    uint64_t result = CURRENT_STATE.REGS[rn] << shift_amount;

    // // Actualizar los flags
    // NEXT_STATE.FLAG_N = (result >> 63) & 0b1;
    // NEXT_STATE.FLAG_Z = (result == 0) ? 1 : 0;

    // Guardar el resultado en el registro destino
    NEXT_STATE.REGS[rd] = result;

    // Avanzar el PC
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void decode_lsr(uint32_t instruction){
    // Get the register numbers
    uint32_t rd = (instruction >> 0) & 0b11111;
    uint32_t rn = (instruction >> 5) & 0b11111;

    // Extraer los campos immr
    uint32_t immr = (instruction >> 16) & 0b111111;

    // Calcular el shift amount correcto
    uint32_t shift_amount = immr;

    // Shift the value
    uint64_t result = CURRENT_STATE.REGS[rn] >> shift_amount;

    // // Update the flags
    // NEXT_STATE.FLAG_N = (result >> 63) & 0b1;
    // NEXT_STATE.FLAG_Z = (result == 0) ? 1 : 0;

    // Update the register
    NEXT_STATE.REGS[rd] = result;

    // Update the PC
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void decode_stur(uint32_t instruction){
    // Get the register numbers
    uint32_t rt = (instruction >> 0) & 0b11111;
    uint32_t rn = (instruction >> 5) & 0b11111;

    // Get the immediate value
    uint32_t imm9 = (instruction >> 12) & 0b111111111;

    // Get the register value
    uint64_t rt_value = CURRENT_STATE.REGS[rt];

    // Get the address
    uint64_t address = CURRENT_STATE.REGS[rn] + imm9;

    // Escribir los 32 bits inferiores
    mem_write_32(address, (uint32_t)(rt_value & 0xFFFFFFFF));
    
    // Escribir los 32 bits superiores
    mem_write_32(address + 4, (uint32_t)(rt_value >> 32));

    // Update the PC
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void decode_sturb(uint32_t instruction){
    // Obtener los registros
    uint32_t rt = (instruction >> 0) & 0b11111;
    uint32_t rn = (instruction >> 5) & 0b11111;

    // Obtener el inmediato
    int32_t imm9 = (int32_t)((instruction >> 12) & 0b111111111); // Signo extendido
    if (imm9 & 0x100) imm9 |= 0xFFFFFF00; // Extender signo si el bit más alto está en 1

    // Obtener el valor del registro Rt (solo los 8 bits inferiores)
    uint8_t byte_value = (uint8_t)(CURRENT_STATE.REGS[rt] & 0xFF);

    // Calcular la dirección
    uint64_t address = CURRENT_STATE.REGS[rn] + imm9;

    // Leer los 32 bits actuales de memoria
    uint32_t word = mem_read_32(address & ~0b11); // Alinear dirección a 4 bytes

    // Calcular la posición del byte dentro de la palabra
    int byte_offset = address & 0b11;

    // Reemplazar solo el byte correspondiente
    word &= ~(0xFF << (byte_offset * 8));  // Limpiar el byte objetivo
    word |= (byte_value << (byte_offset * 8)); // Escribir el nuevo byte

    // Escribir de vuelta los 32 bits modificados
    mem_write_32(address & ~0b11, word);

    // Actualizar PC
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void decode_sturh(uint32_t instruction){
    // Obtener los registros
    uint32_t rt = (instruction >> 0) & 0b11111;
    uint32_t rn = (instruction >> 5) & 0b11111;

    // Obtener el inmediato
    int32_t imm9 = (int32_t)((instruction >> 12) & 0b111111111);
    if (imm9 & 0x100) imm9 |= 0xFFFFFF00; // Extender signo si el bit más alto está en 1

    // Obtener el valor del registro Rt (solo los 16 bits inferiores)
    uint16_t half_value = (uint16_t)(CURRENT_STATE.REGS[rt] & 0xFFFF);

    // Calcular la dirección
    uint64_t address = CURRENT_STATE.REGS[rn] + imm9;

    // Leer los 32 bits actuales de memoria
    uint32_t word = mem_read_32(address & ~0b11); // Alinear dirección a 4 bytes

    // Calcular la posición del halfword dentro de la palabra
    int half_offset = (address & 0b10) >> 1; // Puede ser 0 o 1

    // Reemplazar solo los 16 bits correspondientes
    word &= ~(0xFFFF << (half_offset * 16));  // Limpiar el halfword objetivo
    word |= (half_value << (half_offset * 16)); // Escribir el nuevo halfword

    // Escribir de vuelta los 32 bits modificados
    mem_write_32(address & ~0b11, word);

    // Actualizar PC
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}


void decode_ldur(uint32_t instruction){
    // Get the register numbers
    uint32_t rt = (instruction >> 0) & 0b11111;
    uint32_t rn = (instruction >> 5) & 0b11111;

    // Get the immediate value
    uint32_t imm9 = (instruction >> 12) & 0b111111111;

    // Get the address
    uint64_t address = CURRENT_STATE.REGS[rn] + imm9;
    
    // Para registros X (64 bits), necesitamos leer dos palabras de 32 bits
    uint32_t value_low = mem_read_32(address);
    uint32_t value_high = mem_read_32(address + 4);

    // Combinar los 64 bits
    uint64_t full_value = ((uint64_t)value_high << 32) | value_low;

    // Actualizar el registro (64 bits)
    NEXT_STATE.REGS[rt] = full_value;

    // Update the PC
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void decode_ldurb(uint32_t instruction){
    // Get the register numbers
    uint32_t rt = (instruction >> 0) & 0b11111;
    uint32_t rn = (instruction >> 5) & 0b11111;

    // Get the immediate value
    uint32_t imm9 = (instruction >> 12) & 0b111111111;

    // Get the address
    uint64_t address = CURRENT_STATE.REGS[rn] + imm9;

    // Read the value from memory
    uint32_t value = mem_read_32(address);

    // Reduce the value to the first 8 bits
    value = value & 0b11111111;

    // Update the register
    NEXT_STATE.REGS[rt] = value;

    // Update the PC
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void decode_ldurh(uint32_t instruction){
    // Get the register numbers
    uint32_t rt = (instruction >> 0) & 0b11111;
    uint32_t rn = (instruction >> 5) & 0b11111;

    // Get the immediate value
    uint32_t imm9 = (instruction >> 12) & 0b111111111;

    // Get the address
    uint64_t address = CURRENT_STATE.REGS[rn] + imm9;

    // Read the value from memory
    uint32_t value = mem_read_32(address);

    // Reduce the value to the first 16 bits
    value = value & 0b1111111111111111;

    // Update the register
    NEXT_STATE.REGS[rt] = value;

    // Update the PC
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void decode_movz(uint32_t instruction){
    // Get the register number
    uint32_t rd = (instruction >> 0) & 0b11111;

    // Get the immediate value
    uint32_t imm16 = (instruction >> 5) & 0b1111111111111111;

    // Update the register
    NEXT_STATE.REGS[rd] = imm16;

    // Update the PC
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void decode_add_extended(uint32_t instruction){
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

    // Update the register
    NEXT_STATE.REGS[rd] = result;

    // Update the PC
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void decode_add_immediate(uint32_t instruction){
    // Get the register numbers
    uint32_t rd = (instruction >> 0) & 0b11111;
    uint32_t rn = (instruction >> 5) & 0b11111;
    
    // Get the immediate value
    uint32_t imm12 = (instruction >> 10) & 0b111111111111;

    // Check for shift
    uint32_t shift = (instruction >> 22) & 0b11;
    switch (shift) {
        case 0b00: {
            // Handle case shift 00
            
            // Add the values
            uint64_t result = CURRENT_STATE.REGS[rn] + imm12;

            // Update the register
            NEXT_STATE.REGS[rd] = result;

            // Update the PC
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }
        case 0b01: {
            // Handle case 01
            
            // Shift the immediate value
            uint64_t shifted_imm12 = imm12 << 12;

            // Add the values
            uint64_t result = CURRENT_STATE.REGS[rn] + shifted_imm12;

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

void decode_mul(uint32_t instruction){
    // Get the register numbers
    uint32_t rd = (instruction >> 0) & 0b11111;
    uint32_t rn = (instruction >> 5) & 0b11111;
    uint32_t rm = (instruction >> 16) & 0b11111;

    // Multiply the values
    uint64_t result = CURRENT_STATE.REGS[rn] * CURRENT_STATE.REGS[rm];

    // Update the register
    NEXT_STATE.REGS[rd] = result;

    // Update the PC
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void decode_cbz(uint32_t instruction){
    // Get the register number
    uint32_t rt = (instruction >> 5) & 0b11111;

    // Get the immediate value
    uint32_t imm19 = (instruction >> 5) & 0b1111111111111111111;

    // Check if the register is zero
    if (CURRENT_STATE.REGS[rt] == 0) {
        // Sign-extend from 19 bits to 32 bits
        if (imm19 & 0b1000000000000000000) { // Check if bit 18 is set (negative number)
            imm19 |= 0b11111111111111111111100000000000; // Extend with 1's
        }
        
        // Multiply by 4 (as mentioned in the second image)
        // "Its offset from the address of this instruction is encoded as 'imm19' times 4"
        int32_t offset = imm19 * 4;
        
        // Update PC - the target is PC-relative
        NEXT_STATE.PC = CURRENT_STATE.PC + offset;
    }
    else {
        NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    }
}

void decode_cbnz(uint32_t instruction){
    // Get the register number
    uint32_t rt = (instruction >> 5) & 0b11111;

    // Get the immediate value
    uint32_t imm19 = (instruction >> 5) & 0b1111111111111111111;

    // Check if the register is zero
    if (CURRENT_STATE.REGS[rt] != 0) {
        // Sign-extend from 19 bits to 32 bits
        if (imm19 & 0b1000000000000000000) { // Check if bit 18 is set (negative number)
            imm19 |= 0b11111111111111111111100000000000; // Extend with 1's
        }
        
        // Multiply by 4 (as mentioned in the second image)
        // "Its offset from the address of this instruction is encoded as 'imm19' times 4"
        int32_t offset = imm19 * 4;
        
        // Update PC - the target is PC-relative
        NEXT_STATE.PC = CURRENT_STATE.PC + offset;
    }
    else {
        NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    }
}

void decode_instruction(){

    printf("Decoding instruction\n");
    uint32_t instruction = mem_read_32(CURRENT_STATE.PC);
    printf("Instruction: 0x%X\n", instruction);

    // Posibles opcodes con diferentes tamaños según el formato de instrucción
    uint32_t opcode_11 = (instruction >> 21) & 0x7FF;  // 11 bits (R, I, D, IW)
    uint32_t opcode_10  = (instruction >> 22) &  0x3FF; // 10 bits 
    uint32_t opcode_8  = (instruction >> 24) & 0xFF;   // 8 bits (CB)
    uint32_t opcode_6  = (instruction >> 26) & 0x3F;   // 6 bits (B)

    printf("Opcode 11: 0x%X\n", opcode_11);
    printf("Opcode 10: 0x%X\n", opcode_10);
    printf("Opcode 8: 0x%X\n", opcode_8);
    printf("Opcode 6: 0x%X\n", opcode_6);
    // Buscar el opcode en el conjunto de instrucciones
    for (int i = 0; i < INSTRUCTION_SET_SIZE; i++) {
        if (INSTRUCTION_SET[i].opcode == opcode_11) { 
            printf("Match found\n");
            void (*decode_function)(uint32_t) = INSTRUCTION_SET[i].function;
            decode_function(instruction);
            return;
        }
        else if (INSTRUCTION_SET[i].opcode == opcode_8) {
            printf("Match found\n");
            void (*decode_function)(uint32_t) = INSTRUCTION_SET[i].function;
            decode_function(instruction);
            return;
        }
        else if (INSTRUCTION_SET[i].opcode == opcode_10) {
            printf("Match found\n");
            void (*decode_function)(uint32_t) = INSTRUCTION_SET[i].function;
            decode_function(instruction);
            return;
        }
        else if (INSTRUCTION_SET[i].opcode == opcode_6) {
            printf("Match found\n");
            void (*decode_function)(uint32_t) = INSTRUCTION_SET[i].function;
            decode_function(instruction);
            return;
        }
    }
    printf("Unknown instruction\n");
    // Stop the simulation, segmenation fault
    RUN_BIT = 0;
}

