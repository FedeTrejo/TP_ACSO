#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include "file.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

int directory_findname(struct unixfilesystem *fs, const char *name,
  int dirinumber, struct direntv6 *dirEnt) {

  struct inode in;
  if (inode_iget(fs, dirinumber, &in) < 0) {
    return -1;
  }

  // Verificar que es un directorio
  if ((in.i_mode & IFMT) != IFDIR) {
    return -1;
  }

  int size = inode_getsize(&in);
  int numBlocks = (size + DISKIMG_SECTOR_SIZE - 1) / DISKIMG_SECTOR_SIZE;

  for (int bno = 0; bno < numBlocks; bno++) {
    char buf[DISKIMG_SECTOR_SIZE];
    int bytes = file_getblock(fs, dirinumber, bno, buf);
    if (bytes < 0) return -1;

    int nentries = bytes / sizeof(struct direntv6);
    struct direntv6 *entries = (struct direntv6 *) buf;

    for (int i = 0; i < nentries; i++) {
      if (strncmp(name, entries[i].d_name, 14) == 0) {
        *dirEnt = entries[i];
        return 0;
      }
    }
  }

  return -1; 
}
