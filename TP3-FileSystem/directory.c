#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include "file.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define MAX_NAME_LEN     sizeof(((struct direntv6 *)0)->d_name)

/**
 * directory_findname:
 *   - fs:         sistema de archivos abierto
 *   - name:       nombre de entrada a buscar (no NULL, longitud ≤ MAX_NAME_LEN)
 *   - dirinumber: número de inode del directorio donde buscar (≥ 1)
 *   - dirEnt:     salida, puntero a direntv6 donde copiar la entrada encontrada
 *
 * Recorre todos los bloques del directorio y compara cada direntv6.
 * Devuelve  0  si encontró el nombre y llenó *dirEnt,
 *         -1  en caso de error (punteros nulos, inode no asignado, no es directorio, etc.),
 *          1  si no existe la entrada.
 */
int directory_findname(struct unixfilesystem *fs,
                       const char *name,
                       int dirinumber,
                       struct direntv6 *dirEnt)
{
    /* 1) Validaciones básicas */
    if (fs == NULL || name == NULL || dirEnt == NULL || dirinumber < 1) {
        return -1;
    }

    /* 2) Longitud del nombre válida */
    size_t namelen = strlen(name);
    if (namelen == 0 || namelen > MAX_NAME_LEN) {
        return -1;
    }

    /* 3) Obtener y validar inode del directorio */
    struct inode in;
    if (inode_iget(fs, dirinumber, &in) < 0) {
        return -1;
    }
    /* Debe estar asignado */
    if ((in.i_mode & IALLOC) == 0) {
        return -1;
    }
    /* Debe ser directorio */
    if ((in.i_mode & IFMT) != IFDIR) {
        return -1;
    }

    /* 4) Calcular cantidad de bloques que ocupa el directorio */
    int dirsize   = inode_getsize(&in);
    int numBlocks = (dirsize + DISKIMG_SECTOR_SIZE - 1)
                    / DISKIMG_SECTOR_SIZE;

    /* 5) Recorrer cada bloque con file_getblock */
    for (int bno = 0; bno < numBlocks; bno++) {
        char buf[DISKIMG_SECTOR_SIZE];
        int bytes = file_getblock(fs, dirinumber, bno, buf);
        if (bytes < 0) {
            return -1;     /* error de lectura */
        }
        /* Si bloque no asignado o fuera de rango, bytes==0, entonces seguimos */
        if (bytes == 0) {
            continue;
        }

        /* 6) Iterar sobre cada entrada válida en el bloque */
        int nentries = bytes / sizeof(struct direntv6);
        struct direntv6 *entries = (struct direntv6 *)buf;

        for (int i = 0; i < nentries; i++) {
            /* Saltar inodes vacíos */
            if (entries[i].d_inumber == 0) {
                continue;
            }
            /* Comparar nombre exacto (direntv6.loose-terminado en '\0') */
            if (namelen == strnlen(entries[i].d_name, MAX_NAME_LEN)
             && strncmp(name, entries[i].d_name, MAX_NAME_LEN) == 0)
            {
                /* 7) Copiar resultado y salir */
                *dirEnt = entries[i];
                return 0;
            }
        }
    }

    /* 8) No encontrado */
    return -1;
}
