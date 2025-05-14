#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "inode.h"
#include "diskimg.h"
#include "unixfilesystem.h"

#define INODES_PER_SECTOR   (DISKIMG_SECTOR_SIZE / sizeof(struct inode))
#define BLOCKS_PER_INDIRECT ((int)(DISKIMG_SECTOR_SIZE / sizeof(uint16_t)))

/**
 * inode_iget:
 *   - fs:  sistema de archivos abierto
 *   - inumber: número de inode (>= 1)
 *   - inp: puntero a struct inode donde almacenar el resultado
 *
 * Lee el sector correspondiente al inode y copia la entrada deseada.
 * Devuelve 0 si tiene éxito o -1 en caso de error.
 */
int inode_iget(struct unixfilesystem *fs,
              int inumber,
              struct inode *inp)
{
    /* Validaciones básicas */
    if (fs == NULL || inp == NULL || inumber < 1) {
        return -1;
    }

    /* Calcular sector que contiene el inode */
    int sector = INODE_START_SECTOR + (inumber - 1) / INODES_PER_SECTOR;
    struct inode inodes[INODES_PER_SECTOR];

    /* Leer todos los inodes de ese sector */
    if (diskimg_readsector(fs->dfd, sector, inodes) < 0) {
        return -1;
    }

    /* Calcular índice dentro del sector y copiar al destino */
    int index = (inumber - 1) % INODES_PER_SECTOR;
    *inp = inodes[index];

    return 0;
}

/**
 * inode_indexlookup:
 *   - fs:      sistema de archivos abierto
 *   - inp:     puntero al inode ya cargado
 *   - blockNum: índice lógico del bloque dentro del archivo (>= 0)
 *
 * Convierte un bloque lógico en un bloque físico de disco, soportando:
 *   1) Bloques directos (0..7)
 *   2) Indirección simple (hasta 7 * BLOCKS_PER_INDIRECT)
 *   3) Doble indirecto (> 7 * BLOCKS_PER_INDIRECT)
 *
 * Retorna:
 *   - número de bloque físico (> 0)
 *   - -1 en caso de error o rango inválido
 */
int inode_indexlookup(struct unixfilesystem *fs,
                      struct inode *inp,
                      int blockNum)
{
    /* Validaciones básicas */
    if (fs == NULL || inp == NULL || blockNum < 0) {
        return -1;
    }

    /* 1) Acceso directo para archivos pequeños */
    if ((inp->i_mode & ILARG) == 0) {
        if (blockNum >= 8) {
            return -1;
        }
        int physical_block = inp->i_addr[blockNum];
        return (physical_block == 0) ? -1 : physical_block;
    }

    /* 2) Indirección simple para archivos grandes */
    if (blockNum < 7 * BLOCKS_PER_INDIRECT) {
        int indir_index  = blockNum / BLOCKS_PER_INDIRECT;
        int entry_offset = blockNum % BLOCKS_PER_INDIRECT;

        int indir_sector = inp->i_addr[indir_index];
        if (indir_sector == 0) {
            return -1;
        }

        uint16_t indir_block[BLOCKS_PER_INDIRECT];
        if (diskimg_readsector(fs->dfd, indir_sector, indir_block) < 0) {
            return -1;
        }

        int data_block = indir_block[entry_offset];
        return (data_block == 0) ? -1 : data_block;
    }

    /* 3) Doble indirecto para bloques más allá */
    int relative      = blockNum - 7 * BLOCKS_PER_INDIRECT;
    int outer_index   = relative / BLOCKS_PER_INDIRECT;
    int inner_index   = relative % BLOCKS_PER_INDIRECT;

    /* Validar índices */
    if (outer_index < 0 || outer_index >= BLOCKS_PER_INDIRECT ||
        inner_index < 0 || inner_index >= BLOCKS_PER_INDIRECT) {
        return -1;
    }

    /* Leer bloque de punteros externos */
    int double_indir_sector = inp->i_addr[7];
    if (double_indir_sector == 0) {
        return -1;
    }

    uint16_t outer_block[BLOCKS_PER_INDIRECT];
    if (diskimg_readsector(fs->dfd, double_indir_sector, outer_block) < 0) {
        return -1;
    }

    /* Obtener sector intermedio */
    int indir_sector = outer_block[outer_index];
    if (indir_sector == 0) {
        return -1;
    }

    /* Leer bloque de punteros internos */
    uint16_t inner_block[BLOCKS_PER_INDIRECT];
    if (diskimg_readsector(fs->dfd, indir_sector, inner_block) < 0) {
        return -1;
    }

    /* Devolver bloque de datos */
    int data_block = inner_block[inner_index];
    return (data_block == 0) ? -1 : data_block;
}

int inode_getsize(struct inode *inp)
{
    return ((inp->i_size0 << 16) | inp->i_size1);
}
