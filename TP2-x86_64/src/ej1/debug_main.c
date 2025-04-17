#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Asegurate de tener estas structs y funciones si no las tenés en un header

typedef struct string_proc_node {
    uint8_t type;
    char* hash;
    struct string_proc_node* next;
    struct string_proc_node* previous;
} string_proc_node;

typedef struct {
    string_proc_node* first;
    string_proc_node* last;
} string_proc_list;

extern string_proc_list* string_proc_list_create(void);
extern void string_proc_list_add_node(string_proc_list* list, uint8_t type, char* hash);
extern char* string_proc_list_concat(string_proc_list* list, uint8_t type, char* hash);

int main() {
    puts("DEBUG: Entrando al main");  // o printf("DEBUG\n");
    string_proc_list* list = string_proc_list_create();

    string_proc_list_add_node(list, 1, "AAA");
    string_proc_list_add_node(list, 2, "BBB");
    string_proc_list_add_node(list, 1, "CCC");

    char* result = string_proc_list_concat(list, 1, "ZZZ");

    printf("Resultado esperado: AAACCCZZZ\n");
    printf("Resultado obtenido: %s\n", result);

    free(result);

    return 0;
}
