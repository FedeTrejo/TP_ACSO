.text
.align 2
.global _start
_start:

// Inicialización de registros
    movz x1, 10          // X1 = 10
    movz x2, 5           // X2 = 5
    movz x3, 0           // X3 = 0
    movz x4, 0xFF        // X4 = 255
    movz x13, 25         // X13 = 25
    movz x14, 20         // X14 = 20
    
// Sección 1: Prueba de ADD (Immediate y Extended Register)
    add x5, x1, 3        // X5 = X1 + 3 = 13
    add x6, x1, x2       // X6 = X1 + X2 = 15
    add x7, x1, 3, lsl 12 // X7 = X1 + (3 << 12) = 10 + 12288 = 12298
    
// Sección 2: Prueba de ADDS (Immediate y Extended Register)
    adds x8, x1, 3       // X8 = X1 + 3 = 13, actualiza flags
    adds x9, x1, x2      // X9 = X1 + X2 = 15, actualiza flags
    adds x10, x1, 3, lsl 12 // X10 = X1 + (3 << 12) = 12298, actualiza flags
    
// Sección 3: Prueba de SUBS (Immediate y Extended Register)
    subs x11, x1, 3      // X11 = X1 - 3 = 7, actualiza flags
    subs x12, x1, x2     // X12 = X1 - X2 = 5, actualiza flags
    subs x15, x1, 3, lsl 12 // X15 = X1 - (3 << 12) = 10 - 12288 = -12278, actualiza flags
    
// Sección 4: Prueba de CMP (Immediate y Extended Register)
    cmp x13, 4           // Compara X13(25) con 4, actualiza flags
    cmp x13, x14         // Compara X13(25) con X14(20), actualiza flags
    
// Sección 5: Prueba de operaciones lógicas
    // ANDS
    movz x16, 0xF        // X16 = 15 (1111 en binario)
    movz x17, 0x3        // X17 = 3 (0011 en binario)
    ands x18, x16, x17   // X18 = X16 & X17 = 15 & 3 = 3, actualiza flags
    
    // EOR
    eor x19, x16, x17    // X19 = X16 ^ X17 = 15 ^ 3 = 12 (1100 en binario)
    
    // ORR
    orr x20, x16, x17    // X20 = X16 | X17 = 15 | 3 = 15 (1111 en binario)
    
// Sección 6: Prueba de Shift
    // LSL
    lsl x21, x4, 4       // X21 = X4 << 4 = 255 << 4 = 4080
    
    // LSR
    lsr x22, x4, 4       // X22 = X4 >> 4 = 255 >> 4 = 15
    
// Sección 7: Prueba de MUL
    mul x23, x1, x2      // X23 = X1 * X2 = 10 * 5 = 50
    
// Sección 8: Configuración para pruebas de memoria
    // Creamos una dirección de memoria válida para pruebas
    // 0x10000000 es típicamente una dirección válida en el simulador
    movz x24, 0x1000     // Parte baja de la dirección
    lsl x24, x24, 16      // Desplaza 16 bits a la izquierda: 0x0000000010000000

// Sección 9: Prueba de operaciones de memoria
    // STUR
    stur x1, [x24, #0x10]    // M[0x10000010] = X1 (10)
    
    // STURB - Corregida para usar registro W
    sturb w4, [x24, #0x20]   // M[0x10000020] = X4(7:0) (solo los primeros 8 bits de 255 = 0xFF)
    
    // STURH
    sturh w4, [x24, #0x30]   // M[0x10000030] = X4(15:0) (solo los primeros 16 bits de 255 = 0x00FF)
    
    // LDUR
    ldur x25, [x24, #0x10]   // X25 = M[0x10000010] = 10
    
    // LDURB
    ldurb w26, [x24, #0x20]  // X26 = 56'b0 + M[0x10000020](7:0) = 0xFF (255)
    
    // LDURH
    ldurh w27, [x24, #0x30]  // X27 = 48'b0 + M[0x10000030](15:0) = 0x00FF (255)
    
// Sección 11: Prueba de CBZ y CBNZ
    // CBZ
    movz x3, 0           // X3 = 0 (nos aseguramos que sea 0)
    cbz x3, cbz_target   // Salta a cbz_target porque X3 = 0
    movz x3, 0xBAD       // Esta instrucción no debería ejecutarse
    
cbz_target:
    movz x3, 0x1         // X3 = 1
    
    // CBNZ
    cbnz x3, cbnz_target // Salta a cbnz_target porque X3 = 1
    movz x3, 0xBAD       // Esta instrucción no debería ejecutarse
    
cbnz_target:
    movz x3, 0x2         // X3 = 2
    
// Sección 12: Prueba de saltos condicionales
    // Configuración para pruebas condicionales
    movz x1, 10          // X1 = 10
    movz x2, 5           // X2 = 5
    
    // BEQ - No debería saltar porque X1 != X2
    cmp x1, x2
    beq beq_target
    movz x5, 0x1         // X5 = 1
    b beq_continue
beq_target:
    movz x5, 0xF         // No debería ejecutarse
beq_continue:
    
    // BNE - Debería saltar porque X1 != X2
    cmp x1, x2
    bne bne_target
    movz x6, 0xF         // No debería ejecutarse
    b bne_continue
bne_target:
    movz x6, 0x1         // X6 = 1
bne_continue:
    
    // BGT - Debería saltar porque X1 > X2
    cmp x1, x2
    bgt bgt_target
    movz x7, 0xF         // No debería ejecutarse
    b bgt_continue
bgt_target:
    movz x7, 0x1         // X7 = 1
bgt_continue:
    
    // BLT - No debería saltar porque X1 !< X2
    cmp x1, x2
    blt blt_target
    movz x8, 0x1         // X8 = 1
    b blt_continue
blt_target:
    movz x8, 0xF         // No debería ejecutarse
blt_continue:
    
    // BGE - Debería saltar porque X1 >= X2
    cmp x1, x2
    bge bge_target
    movz x9, 0xF         // No debería ejecutarse
    b bge_continue
bge_target:
    movz x9, 0x1         // X9 = 1
bge_continue:
    
    // BLE - No debería saltar porque X1 !<= X2
    cmp x1, x2
    ble ble_target
    movz x10, 0x1        // X10 = 1
    b ble_continue
ble_target:
    movz x10, 0xF        // No debería ejecutarse
ble_continue:
    
    // Ahora probamos con X2 > X1
    movz x1, 5           // X1 = 5
    movz x2, 10          // X2 = 10
    
    // BEQ - No debería saltar porque X1 != X2
    cmp x1, x2
    beq beq_target2
    movz x11, 0x1        // X11 = 1
    b beq_continue2
beq_target2:
    movz x11, 0xF        // No debería ejecutarse
beq_continue2:
    
    // BLT - Debería saltar porque X1 < X2
    cmp x1, x2
    blt blt_target2
    movz x12, 0xF        // No debería ejecutarse
    b blt_continue2
blt_target2:
    movz x12, 0x1        // X12 = 1
blt_continue2:
    
    // BLE - Debería saltar porque X1 <= X2
    cmp x1, x2
    ble ble_target2
    movz x13, 0xF        // No debería ejecutarse
    b ble_continue2
ble_target2:
    movz x13, 0x1        // X13 = 1
ble_continue2:
    
    // Prueba con igualdad
    movz x1, 10          // X1 = 10
    movz x2, 10          // X2 = 10
    
    // BEQ - Debería saltar porque X1 == X2
    cmp x1, x2
    beq beq_target3
    movz x14, 0xF        // No debería ejecutarse
    b beq_continue3
beq_target3:
    movz x14, 0x1        // X14 = 1
beq_continue3:
    
    // BGE - Debería saltar porque X1 >= X2
    cmp x1, x2
    bge bge_target3
    movz x15, 0xF        // No debería ejecutarse
    b bge_continue3
bge_target3:
    movz x15, 0x1        // X15 = 1
bge_continue3:
    
    // BLE - Debería saltar porque X1 <= X2
    cmp x1, x2
    ble ble_target3
    movz x16, 0xF        // No debería ejecutarse
    b ble_continue3
ble_target3:
    movz x16, 0x1        // X16 = 1
ble_continue3:

// Finalización del programa
    hlt 0                // Detiene la simulación
    