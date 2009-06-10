/* wbfs_ops.c
 *
 * Copyright (C) 2009 Ricardo Massaro
 * Copyright (C) 2009 Kwiirk
 *
 * Licensed under the terms of the GNU GPL, version 2
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#include <stdlib.h>
#include <stdio.h>

#include "wbfs_ops.h"
#include "app_state.h"
#include "message.h"

#include "libwbfs.h"
#include "libwbfs_os.h"

int cancel_wbfs_op;

void dump_wbfs_info(void)
{
  wbfs_t *wbfs = app_state.wbfs;

  printf("----------------------------\n");
  printf("hd_sec_sz     = %d\n", wbfs->hd_sec_sz);
  printf("hd_sec_sz_s   = %d\n", wbfs->hd_sec_sz_s);
  printf("n_hd_sec      = %d\n", wbfs->n_hd_sec);
  printf("TOTAL         = %lld\n", (u64) wbfs->n_hd_sec * wbfs->hd_sec_sz);

  printf("----------------------------\n");
  printf("wii_sec_sz    = %d\n", wbfs->wii_sec_sz);
  printf("wii_sec_sz_s  = %d\n", wbfs->wii_sec_sz_s);
  printf("n_wii_sec     = %d\n", wbfs->n_wii_sec);
  printf("TOTAL         = %lld\n", (u64) wbfs->n_wii_sec * wbfs->wii_sec_sz);

  printf("----------------------------\n");
  printf("wbfs_sec_sz   = %d\n", wbfs->wbfs_sec_sz);
  printf("wbfs_sec_sz_s = %d\n", wbfs->wbfs_sec_sz_s);
  printf("n_wbfs_sec    = %d\n", wbfs->n_wbfs_sec);
  printf("TOTAL         = %lld\n", (u64) wbfs->n_wbfs_sec * wbfs->wbfs_sec_sz);

  printf("----------------------------\n");
  printf("disc_info_sz  = %d\n", wbfs->disc_info_sz);
  printf("max_disc      = %d\n", wbfs->max_disc);
  
}

static int write_wii_sector_file(void *_fp, u32 lba, u32 count, void *iobuf)
{
  FILE *fp = _fp;
  u64 off = lba;

  if (cancel_wbfs_op)
    return 1;

  off *= 0x8000;
  if (fseeko(fp, off, SEEK_SET)) {
    show_error("Error reading ISO", "Can't seek in disc file (%llu)", off);
    return 1;
  }
  if (fwrite(iobuf, count*0x8000, 1, fp) != 1) {
    show_error("Error reading ISO", "Can't write disc file.");
    return 1;
  }
  return 0;
}

static void progress_update(int cur, int max)
{
  printf("DUMMY UPDATE: %u/%u\n", (unsigned int) cur, (unsigned int) max);
}

int extract_iso(char *code, char *filename, void (*update)(int, int))
{
  FILE *f;
  wbfs_disc_t *disc;

  cancel_wbfs_op = 0;
  if (! update)
    update = progress_update;

  /* open disc */
  disc = wbfs_open_disc(app_state.wbfs, (u8 *) code);
  if (disc == NULL) {
    show_error("Error Extracting ISO", "Can't find disc id '%s'", code);
    return 1;
  }

  /* open ISO */
  f = fopen(filename, "w");
  if (f == NULL) {
    show_error("Error Extracting ISO", "Can't open ISO file '%s'", filename);
    wbfs_close_disc(disc);
    return 1;
  }

  /* TODO: fill ISO file with (disc->p->n_wii_sec_per_disc/2)*0x8000ULL bytes*/
  wbfs_extract_disc(disc, write_wii_sector_file, (void *) f, update);

  fclose(f);
  wbfs_close_disc(disc);
  return 0;
}
