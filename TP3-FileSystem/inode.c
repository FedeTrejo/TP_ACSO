#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "inode.h"
#include "diskimg.h"

#include "unixfilesystem.h"

#define INODES_PER_SECTOR (DISKIMG_SECTOR_SIZE / sizeof(struct inode))
#define BLOCKS_PER_INDIRECT (DISKIMG_SECTOR_SIZE / sizeof(uint16_t))

int inode_iget(struct unixfilesystem *fs, int inumber, struct inode *inp) {
    if (inumber < 1) {
        return -1;
    }

    int sector = INODE_START_SECTOR + (inumber - 1) / INODES_PER_SECTOR;
    struct inode inodes[INODES_PER_SECTOR];

    if (diskimg_readsector(fs->dfd, sector, inodes) == -1) {
        return -1;
    }

    int index = (inumber - 1) % INODES_PER_SECTOR;
    *inp = inodes[index];

    return 0;
}

int inode_indexlookup(struct unixfilesystem *fs, struct inode *inp, int blockNum) {
    // Archivos pequeños (sin ILARG): acceso directo
    if ((inp->i_mode & ILARG) == 0) {
        if (blockNum < 0 || blockNum >= 8) {
            return -1;
        }
        int physical_block = inp->i_addr[blockNum];
        return (physical_block == 0) ? -1 : physical_block;
    }

    // Archivos grandes: acceso indirecto
    if (blockNum < 256 * 7) {
        int indir_index = blockNum / 256;
        int indir_offset = blockNum % 256;

        int indir_sector = inp->i_addr[indir_index];
        if (indir_sector == 0) return -1;

        uint16_t indir_block[BLOCKS_PER_INDIRECT];
        if (diskimg_readsector(fs->dfd, indir_sector, indir_block) == -1) {
            return -1;
        }

        int data_block = indir_block[indir_offset];
        return (data_block == 0) ? -1 : data_block;
    }

    // Acceso doble indirecto (bloques 1792+)
    int relative = blockNum - 256 * 7;
    int outer_index = relative / 256;
    int inner_index = relative % 256;

    if (outer_index < 0 || outer_index >= (int)BLOCKS_PER_INDIRECT) {
        return -1;
    }
    
    if (inner_index < 0 || inner_index >= (int)BLOCKS_PER_INDIRECT) {
        return -1;
    }    

    int double_indir_sector = inp->i_addr[7];
    if (double_indir_sector == 0) return -1;

    uint16_t outer_block[BLOCKS_PER_INDIRECT];
    if (diskimg_readsector(fs->dfd, double_indir_sector, outer_block) == -1) {
        return -1;
    }

    int indir_sector = outer_block[outer_index];
    if (indir_sector == 0) return -1;

    uint16_t inner_block[BLOCKS_PER_INDIRECT];
    if (diskimg_readsector(fs->dfd, indir_sector, inner_block) == -1) {
        return -1;
    }

    int data_block = inner_block[inner_index];

    return (data_block == 0) ? -1 : data_block;

}

int inode_getsize(struct inode *inp) {
  return ((inp->i_size0 << 16) | inp->i_size1); 
}
