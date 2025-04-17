; /** defines bool y puntero **/
%define NULL 0
%define TRUE 1
%define FALSE 0

section .data

section .text

global string_proc_list_create_asm
global string_proc_node_create_asm
global string_proc_list_add_node_asm
global string_proc_list_concat_asm

; FUNCIONES auxiliares que pueden llegar a necesitar:
extern malloc
extern free
extern str_concat


string_proc_list_create_asm:
    ; Prólogo de la función
    PUSH RBP
    MOV RBP, RSP
    
    ; Llamar a malloc para asignar memoria para la estructura string_proc_list
    MOV RDI, 16          ; Tamaño de la estructura = 16 bytes (2 punteros de 8 bytes cada uno)
    CALL malloc
    
    ; Verificar si malloc devolvió NULL
    TEST RAX, RAX
    JZ .RETORNAR_NULL    ; Si malloc devolvió NULL, retornar NULL
    
    ; Inicializar los campos first y last a NULL
    MOV QWORD [RAX], NULL    ; list->first = NULL
    MOV QWORD [RAX+8], NULL  ; list->last = NULL
    
    ; El valor de retorno ya está en RAX
    JMP .FIN
    
.RETORNAR_NULL:
    ; No es necesario hacer nada, RAX ya contiene NULL
    
.FIN:
    ; Epílogo de la función
    LEAVE
    RET

string_proc_node_create_asm:
    PUSH RBP
    MOV RBP, RSP

    ; Verificar si hash es NULL
    TEST RSI, RSI
    JZ .RETORNAR_NULL     ; Si hash == NULL, retornar NULL

    ; Guardar los parámetros
    PUSH RDI              ; Guardar type
    PUSH RSI              ; Guardar hash

    ; Llamar a malloc
    MOV RDI, 32
    CALL malloc

    ; Verificar si malloc devolvió NULL
    TEST RAX, RAX
    JZ .RECUPERAR_Y_NULL  ; Si malloc falló, restaurar pila y retornar NULL

    ; Recuperar parámetros
    POP RSI               ; hash
    POP RDI               ; type

    ; Inicializar campos
    MOV QWORD [RAX], NULL       ; next
    MOV QWORD [RAX+8], NULL     ; previous
    MOV BYTE  [RAX+16], DIL     ; type
    MOV QWORD [RAX+24], RSI     ; hash

    JMP .FIN

.RECUPERAR_Y_NULL:
    POP RSI
    POP RDI

.RETORNAR_NULL:
    XOR RAX, RAX

.FIN:
    LEAVE
    RET

string_proc_list_add_node_asm:
    ; Prólogo de la función
    PUSH RBP
    MOV RBP, RSP
    
    ; Guardar los parámetros
    ; RDI = list (string_proc_list*)
    ; RSI = type (uint8_t)
    ; RDX = hash (char*)
    
    ; Verificar si la lista es NULL
    TEST RDI, RDI
    JZ .FIN                ; Si list == NULL, terminar
    
    ; Guardar list en la pila
    PUSH RDI
    
    ; Llamar a string_proc_node_create
    MOV RDI, RSI           ; Primer parámetro: type
    MOV RSI, RDX           ; Segundo parámetro: hash
    CALL string_proc_node_create_asm
    
    ; Verificar si string_proc_node_create devolvió NULL
    TEST RAX, RAX
    JZ .RESTAURAR_Y_SALIR  ; Si new_node == NULL, limpiar y terminar
    
    ; Recuperar list
    POP RDI
    
    ; RAX = new_node
    ; RDI = list
    
    ; Verificar si list->first == NULL
    MOV RCX, QWORD [RDI]   ; RCX = list->first
    TEST RCX, RCX
    JNZ .LISTA_NO_VACIA
    
    ; Si la lista está vacía:
    ; list->first = new_node
    ; list->last = new_node
    MOV QWORD [RDI], RAX     ; list->first = new_node
    MOV QWORD [RDI+8], RAX   ; list->last = new_node
    JMP .FIN
    
.LISTA_NO_VACIA:
    ; Si la lista no está vacía:
    ; list->last->next = new_node
    ; new_node->previous = list->last
    ; list->last = new_node
    
    MOV RCX, QWORD [RDI+8]   ; RCX = list->last
    
    ; list->last->next = new_node (el primer campo del nodo es "next")
    MOV QWORD [RCX], RAX     
    
    ; new_node->previous = list->last (el segundo campo del nodo es "previous", offset 8)
    MOV QWORD [RAX+8], RCX   
    
    ; list->last = new_node
    MOV QWORD [RDI+8], RAX   
    JMP .FIN
    
.RESTAURAR_Y_SALIR:
    POP RDI                  ; Restaurar la pila si new_node es NULL
    
.FIN:
    ; Epílogo de la función
    LEAVE
    RET

string_proc_list_concat_asm:
    ; PRÓLOGO DE LA FUNCIÓN
    PUSH RBP
    MOV RBP, RSP
    PUSH RBX                ; GUARDAR REGISTROS A USAR
    PUSH R12
    PUSH R13
    PUSH R14
    PUSH R15
    SUB RSP, 8              ; ALINEAR LA PILA A 16 BYTES
    
    ; GUARDAR LOS PARÁMETROS EN REGISTROS NO VOLÁTILES
    MOV R12, RDI            ; R12 = LIST
    MOV R13, RSI            ; R13 = TYPE
    MOV R14, RDX            ; R14 = HASH
    
    ; OBTENER EL PRIMER NODO DE LA LISTA
    MOV R15, [R12]          ; R15 = LIST->FIRST
    
    ; INICIALIZAR EL NUEVO HASH COMO STRING VACÍO
    MOV RDI, 1              ; TAMAÑO: 1 BYTE PARA '\0'
    CALL malloc
    TEST RAX, RAX           ; VERIFICAR SI malloc FALLÓ
    JZ .ERROR_SALIDA
    
    MOV BYTE [RAX], 0       ; NEW_HASH[0] = '\0'
    MOV RBX, RAX            ; RBX = NEW_HASH (LO GUARDAMOS PARA USARLO DESPUÉS)
    
.BUCLE_NODOS:
    TEST R15, R15           ; IF(CURRENT_NODE == NULL)
    JZ .CONCATENAR_HASH_FINAL   ; SI ES NULL, SALIMOS DEL BUCLE
    
    ; COMPROBAR TIPO DEL NODO
    MOVZX EAX, BYTE [R15 + 16]  ; CURRENT_NODE->TYPE (OFFSET 16 EN LA ESTRUCTURA)
    CMP AL, R13B            ; COMPARAR CON EL TIPO BUSCADO
    JNE .SIGUIENTE_NODO     ; SI NO COINCIDE, PASAMOS AL SIGUIENTE NODO
    
    ; EL TIPO COINCIDE, CONCATENAR HASH
    MOV RDI, RBX            ; PRIMER PARÁMETRO: NEW_HASH
    MOV RSI, [R15 + 24]     ; SEGUNDO PARÁMETRO: CURRENT_NODE->HASH (OFFSET 24)
    CALL str_concat         ; LLAMAR A str_concat
    
    MOV RDI, RBX            ; PREPARAR PARA LIBERAR EL VIEJO HASH
    MOV RBX, RAX            ; ACTUALIZAR PUNTERO AL NUEVO HASH
    CALL free               ; LIBERAR EL VIEJO HASH
    
.SIGUIENTE_NODO:
    MOV R15, [R15]          ; CURRENT_NODE = CURRENT_NODE->NEXT (OFFSET 0)
    JMP .BUCLE_NODOS        ; VOLVER A COMPROBAR

.CONCATENAR_HASH_FINAL:
    ; VERIFICAR SI HASH ES NULL (DIFERENTE DE LA VERSIÓN ORIGINAL)
    TEST R14, R14
    JZ .FINALIZAR           ; Si HASH es NULL, saltamos directamente a retornar

    ; CONCATENAR EL HASH FINAL PASADO POR PARÁMETRO
    MOV RDI, R14            ; PRIMER PARÁMETRO: HASH (PARÁMETRO)
    MOV RSI, RBX            ; SEGUNDO PARÁMETRO: NEW_HASH
    CALL str_concat         ; LLAMAR A str_concat
    
    MOV RDI, RBX            ; PREPARAR PARA LIBERAR EL VIEJO HASH
    MOV RBX, RAX            ; GUARDAR RESULTADO FINAL
    CALL free               ; LIBERAR EL VIEJO HASH

.FINALIZAR:
    ;CHEQUEAR NO SACO LOS REGISTROS
    MOV RAX, RBX            ; RETORNAR EL NUEVO HASH

.ERROR_SALIDA:
    ; EPÍLOGO DE LA FUNCIÓN
    ADD RSP, 8 
    POP R15
    POP R14
    POP R13
    POP R12
    POP RBX
    LEAVE
    RET