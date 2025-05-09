#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "file.h"
#include "inode.h"
#include "diskimg.h"
#include <string.h>  

#define BLOCKS_PER_INDIRECT ((int)(DISKIMG_SECTOR_SIZE / sizeof(uint16_t)))

int file_getblock(struct unixfilesystem *fs, int inumber, int blockNum, void *buf) {
    struct inode in;
    if (inode_iget(fs, inumber, &in) < 0) return -1;

    int data_block = -1;

    if ((in.i_mode & ILARG) == 0) {
        // Bloques directos (hasta 8)
        if (blockNum >= 0 && blockNum < 8) {
            data_block = in.i_addr[blockNum];
        }
    } else if (blockNum < 7 * BLOCKS_PER_INDIRECT) {
        // Acceso indirecto simple (i_addr[0]..i_addr[6])
        int indir_index = blockNum / BLOCKS_PER_INDIRECT;
        int entry_index = blockNum % BLOCKS_PER_INDIRECT;

        int indir_sector = in.i_addr[indir_index];
        if (indir_sector == 0) return 0;

        uint16_t indir_block[BLOCKS_PER_INDIRECT];
        if (diskimg_readsector(fs->dfd, indir_sector, indir_block) < 0) return -1;

        data_block = indir_block[entry_index];
    } else {
        // Acceso doble indirecto (i_addr[7])
        int relative = blockNum - 7 * BLOCKS_PER_INDIRECT;
        int outer_index = relative / BLOCKS_PER_INDIRECT;
        int inner_index = relative % BLOCKS_PER_INDIRECT;

        int double_indir_sector = in.i_addr[7];
        if (double_indir_sector == 0) return 0;

        uint16_t outer_block[BLOCKS_PER_INDIRECT];
        if (diskimg_readsector(fs->dfd, double_indir_sector, outer_block) < 0) return -1;

        if (outer_index >= BLOCKS_PER_INDIRECT) return 0;
        int inner_sector = outer_block[outer_index];
        if (inner_sector == 0) return 0;

        uint16_t inner_block[BLOCKS_PER_INDIRECT];
        if (diskimg_readsector(fs->dfd, inner_sector, inner_block) < 0) return -1;

        data_block = inner_block[inner_index];
    }

    if (data_block == 0) return 0;

    char tmpbuf[DISKIMG_SECTOR_SIZE];
    if (diskimg_readsector(fs->dfd, data_block, tmpbuf) < 0) return -1;

    int filesize = inode_getsize(&in);
    int block_offset = blockNum * DISKIMG_SECTOR_SIZE;
    int bytesRemaining = filesize - block_offset;

    if (bytesRemaining <= 0) return 0;

    int nbytes = (bytesRemaining < DISKIMG_SECTOR_SIZE) ? bytesRemaining : DISKIMG_SECTOR_SIZE;
    memcpy(buf, tmpbuf, nbytes);
    return nbytes;

}
