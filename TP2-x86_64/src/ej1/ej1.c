#include "ej1.h"

string_proc_list* string_proc_list_create(void){
	string_proc_list* list = (string_proc_list*)malloc(sizeof(string_proc_list));
	if(list == NULL){
		return NULL;
	}
	list->first = NULL;
	list->last  = NULL;
	return list;
}

string_proc_node* string_proc_node_create(uint8_t type, char* hash){
	if (hash == NULL){
		return NULL;
	}
	string_proc_node* node = (string_proc_node*)malloc(sizeof(string_proc_node));
	if(node == NULL){
		return NULL;
	}
	node->next     = NULL;
	node->previous = NULL;
	node->hash     = hash;
	node->type     = type;
	return node;
}

void string_proc_list_add_node(string_proc_list* list, uint8_t type, char* hash){
	if (list == NULL) {
		return;
	}
	string_proc_node* new_node = string_proc_node_create(type, hash);
	if(new_node == NULL){
		return;
	}
	if(list->first == NULL){
		list->first = new_node;
		list->last  = new_node;
	}else{
		list->last->next = new_node;
		new_node->previous = list->last;
		list->last = new_node;
	}
}

char* string_proc_list_concat(string_proc_list* list, uint8_t type, char* hash){
	if (list == NULL) return NULL;

	char* new_hash = malloc(1);
	if (new_hash == NULL) return NULL;
	new_hash[0] = '\0';

	string_proc_node* cur = list->first;
	while (cur) {
		if (cur->type == type) {
		// ⧺ node->hash
			char* tmp = str_concat(new_hash, cur->hash);
			free(new_hash);
			new_hash = tmp;
			}
		cur = cur->next;
	}

	if (hash != NULL) {
	// prefix: hash_param ⧺ new_hash
	char* tmp = str_concat(hash, new_hash);
	free(new_hash);
	new_hash = tmp;
	}

	return new_hash;
}


/** AUX FUNCTIONS **/

void string_proc_list_destroy(string_proc_list* list){

	/* borro los nodos: */
	string_proc_node* current_node	= list->first;
	string_proc_node* next_node		= NULL;
	while(current_node != NULL){
		next_node = current_node->next;
		string_proc_node_destroy(current_node);
		current_node	= next_node;
	}
	/*borro la lista:*/
	list->first = NULL;
	list->last  = NULL;
	free(list);
}
void string_proc_node_destroy(string_proc_node* node){
	node->next      = NULL;
	node->previous	= NULL;
	node->hash		= NULL;
	node->type      = 0;			
	free(node);
}


char* str_concat(char* a, char* b) {
	int len1 = strlen(a);
    int len2 = strlen(b);
	int totalLength = len1 + len2;
    char *result = (char *)malloc(totalLength + 1); 
    strcpy(result, a);
    strcat(result, b);
    return result;  
}

void string_proc_list_print(string_proc_list* list, FILE* file){
        uint32_t length = 0;
        string_proc_node* current_node  = list->first;
        while(current_node != NULL){
                length++;
                current_node = current_node->next;
        }
        fprintf( file, "List length: %d\n", length );
		current_node    = list->first;
        while(current_node != NULL){
                fprintf(file, "\tnode hash: %s | type: %d\n", current_node->hash, current_node->type);
                current_node = current_node->next;
        }
}

