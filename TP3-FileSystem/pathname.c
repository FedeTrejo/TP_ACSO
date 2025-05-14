#include "pathname.h"
#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define MAX_PATH_LEN    1024
#define MAX_NAME_LEN    14  

/**
 * pathname_lookup:
 *   - fs:      sistema de archivos abierto
 *   - pathname: ruta absoluta POSIX que empieza en '/'
 *
 * Recorre cada componente separado por '/' desde el root directory,
 * invocando directory_findname para cada nivel.
 * Devuelve el número de inode final o -1 en caso de error.
 */
int pathname_lookup(struct unixfilesystem *fs, const char *pathname)
{
    /* 1) Validaciones básicas */
    if (fs == NULL || pathname == NULL) {
        return -1;
    }

    /* 2) Debe ser ruta absoluta (comienza en '/') */
    if (pathname[0] != '/') {
        return -1;
    }

    /* 3) Caso especial: raíz */
    if (strcmp(pathname, "/") == 0) {
        return ROOT_INUMBER;
    }

    /* 4) Copiar el pathname porque strtok lo modifica */
    char pathcopy[MAX_PATH_LEN];
    strncpy(pathcopy, pathname, sizeof(pathcopy));
    pathcopy[sizeof(pathcopy) - 1] = '\0';

    /* 5) Iniciar búsqueda desde la raíz */
    int current_inumber = ROOT_INUMBER;
    char *token = strtok(pathcopy, "/");

    /* 6) Iterar por cada componente */
    while (token != NULL) {
        /* Longitud válida según ext2 v6 (14 chars) */
        if (strlen(token) == 0 || strlen(token) > MAX_NAME_LEN) {
            return -1;
        }

        /* 7) Buscar el nombre en el directorio actual */
        struct direntv6 entry;
        if (directory_findname(fs, token, current_inumber, &entry) < 0) {
            return -1;
        }

        /* 8) Avanzar al siguiente nivel */
        current_inumber = entry.d_inumber;
        token = strtok(NULL, "/");
    }

    /* 9) Devolver el inode resultante */
    return current_inumber;
}
