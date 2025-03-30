.text

start:
    MOV X0, 0x40         // Carga el valor 0x40 en X0
    LSL X0, X0, #16      // Desplaza X0 16 bits a la izquierda (equivalente a multiplicar por 65536)
    ADD X0, X0, 20       // Suma 20 a X0, sin usar ADDS
    BR X0                // Salta a la dirección almacenada en X0

    // Esta instruccion no se ejecuta porque el flujo de control se transfiere a la dirección contenida en X0
    ADD X2, X2, 32       // Suma 32 a X2
    // Esta instruccion si se ejecuta
    ADD X1, X1, 52       // Suma 52 a X1

HLT 0                   // Fin de la ejecución
