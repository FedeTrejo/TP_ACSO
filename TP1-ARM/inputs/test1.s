.text
    // Prueba de operaciones aritméticas y lógicas
    mov X1, 0x1000      // Inicializa X1 con 0x1000
    lsl X1, X1, 16      // Desplaza X1 a la izquierda 16 bits
    mov X10, 0x1234     // Carga un valor en X10
    adds X11, X10, X1   // Suma X10 y X1, almacenando el resultado en X11
    subs X12, X10, X1   // Resta X1 de X10, almacenando el resultado en X12
    ands X13, X10, X1   // AND lógico entre X10 y X1
    eor X14, X10, X1    // XOR entre X10 y X1
    orr X15, X10, X1    // OR entre X10 y X1

    // Prueba de carga y almacenamiento con direcciones extremas
    stur X10, [X1, 0x0]   // Almacena X10 en memoria en X1+0
    sturb W10, [X1, 0x6]   // Almacena byte de W10 en X1+6
    ldur X13, [X1, 0x0]    // Carga en X13 desde X1+0
    ldur X14, [X1, 0x4]    // Carga en X14 desde X1+4
    ldurb W15, [X1, 0x6]   // Carga byte en W15 desde X1+6

    // Prueba de registros negativos y overflow
    mov X2, 0             // Carga -1 en X2
    subs X2, X2, 1        // Genera -1 en X2
    adds X3, X2, X2       // Suma -1 + -1 (desbordamiento posible)
    subs X4, X2, X1       // Resta X1 de -1 (caso borde negativo)
    mul X5, X2, X10       // Multiplicación con un valor negativo

    // Prueba de saltos condicionales
    mov X6, 5            // Valor de comparación
    cmp X6, 0           // Comparación con cero
    beq etiqueta_eq     // Salto si son iguales
    bne etiqueta_ne     // Salto si son diferentes
    bgt etiqueta_gt     // Salto si es mayor
    blt etiqueta_lt     // Salto si es menor
    bge etiqueta_ge     // Salto si es mayor o igual
    ble etiqueta_le     // Salto si es menor o igual

etiqueta_eq:
    mov X20, 1          // Señaliza éxito de BEQ
etiqueta_ne:
    mov X21, 1          // Señaliza éxito de BNE
etiqueta_gt:
    mov X22, 1          // Señaliza éxito de BGT
etiqueta_lt:
    mov X23, 1          // Señaliza éxito de BLT
etiqueta_ge:
    mov X24, 1          // Señaliza éxito de BGE
etiqueta_le:
    mov X25, 1          // Señaliza éxito de BLE

    // Prueba de carga inmediata y CBZ/CBNZ
    mov X7, 0          // Carga 0 en X7
    cbz X7, cbz_label  // Debe saltar
    cbnz X6, cbnz_label // Debe saltar porque X6 es 5
cbz_label:
    mov X26, 1        // Señaliza éxito de CBZ
cbnz_label:
    mov X27, 1        // Señaliza éxito de CBNZ

    // Halt para detener la ejecución
    HLT 0