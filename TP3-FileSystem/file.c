#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file.h"
#include "inode.h"
#include "diskimg.h"

/**
 * file_getblock:
 *   - fs: sistema de archivos abierto
 *   - inumber: número de inode (>= 1)
 *   - blockNum: bloque lógico dentro del archivo (>= 0)
 *   - buf: puntero al buffer donde se escribirá (tamaño ≥ DISKIMG_SECTOR_SIZE)
 *
 * Devuelve:
 *   - ≥1 número de bytes leídos (≤ DISKIMG_SECTOR_SIZE)
 *   - 0  si blockNum está fuera del tamaño de archivo
 *   - -1 en caso de error (puntero nulo, inode no asignado, lectura fallida, etc.)
 */
int file_getblock(struct unixfilesystem *fs,
                  int inumber,
                  int blockNum,
                  void *buf)
{
    /* 1) Validaciones básicas */
    if (fs == NULL || buf == NULL || inumber < 1 || blockNum < 0) {
        return -1;
    }

    /* 2) Traer el inode */
    struct inode in;
    if (inode_iget(fs, inumber, &in) < 0) {
        return -1;
    }

    /* 3) Verificar que el inode esté asignado */
    if ((in.i_mode & IALLOC) == 0) {
        return -1;
    }

    /* 4) Traducir bloque lógico a bloque físico */
    int phys_block = inode_indexlookup(fs, &in, blockNum);
    if (phys_block < 0) {
        /* Bloque fuera de rango o no existe */
        return -1;
    }
    if (phys_block == 0) {
        /* Hueco sin asignar: se devuelve 0 bytes */
        return 0;
    }

    /* 5) Leer el sector completo en buffer temporal */
    unsigned char tmp[DISKIMG_SECTOR_SIZE];
    if (diskimg_readsector(fs->dfd, phys_block, tmp) < 0) {
        return -1;
    }

    /* 6) Calcular cuántos bytes del bloque son reales según el tamaño del archivo */
    int filesize = inode_getsize(&in);
    int offset   = blockNum * DISKIMG_SECTOR_SIZE;

    if (offset >= filesize) {
        /* Más allá del final del archivo */
        return 0;
    }

    int bytes_left = filesize - offset;
    int to_copy    = (bytes_left < DISKIMG_SECTOR_SIZE
                         ? bytes_left
                         : DISKIMG_SECTOR_SIZE);

    /* 7) Copiar al buffer de usuario */
    memcpy(buf, tmp, to_copy);

    return to_copy;
}
