#include "pathname.h"
#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

int pathname_lookup(struct unixfilesystem *fs, const char *pathname) {
    if (pathname == NULL || pathname[0] != '/') {
      return -1; // Path no válido
    }
  
    // Copiamos el path porque strtok lo modifica
    char pathcopy[1024];
    strncpy(pathcopy, pathname, sizeof(pathcopy));
    pathcopy[sizeof(pathcopy) - 1] = '\0';
    
    // Comenzamos desde la raíz
    int current_inumber = ROOT_INUMBER;  
  
    char *token = strtok(pathcopy, "/");
    while (token != NULL) {
      struct direntv6 entry;
      if (directory_findname(fs, token, current_inumber, &entry) < 0) {
        return -1; // No se encontró el componente
      }
      current_inumber = entry.d_inumber; // Avanzamos al siguiente nivel
      token = strtok(NULL, "/");
    }
  
    return current_inumber;
  }