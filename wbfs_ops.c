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
    show_error("Error writing ISO", "Can't seek in disc file (%llu)", off);
    return 1;
  }
  if (fwrite(iobuf, count*0x8000, 1, fp) != 1) {
    show_error("Error writing ISO", "Can't write disc file.");
    return 1;
  }
  return 0;
}

static int read_wii_file(void *_fp, u32 offset, u32 count, void *iobuf)
{
  FILE*fp =_fp;
  u64 off = offset;
  off<<=2;

  if (cancel_wbfs_op)
    return 1;

  if (fseeko(fp, off, SEEK_SET)) {
    show_error("Error reading ISO", "Can't seek in disc file.");
    return 1;
  }
  if (fread(iobuf, count, 1, fp) != 1) {
    show_error("Error reading ISO", "Can't read disc file.");
    return 1;
  }
  return 0;
}

static void progress_update(int cur, int max)
{
  printf("DUMMY UPDATE: %u/%u\n", (unsigned int) cur, (unsigned int) max);
}

int op_extract_iso(char *code, char *filename, void (*update)(int, int))
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

  wbfs_file_reserve_space(f, (disc->p->n_wii_sec_per_disc/2) * 0x8000ULL);
  wbfs_extract_disc(disc, write_wii_sector_file, (void *) f, update);

  fclose(f);
  wbfs_close_disc(disc);
  return 0;
}

long long info_get_free_space(void)
{
  unsigned int block_count;

  block_count = wbfs_count_usedblocks(app_state.wbfs);
  return (long long) app_state.wbfs->wbfs_sec_sz * block_count;
}

long long info_get_used_space(void)
{
  unsigned int block_count;

  block_count = wbfs_count_usedblocks(app_state.wbfs);
  return (long long) app_state.wbfs->wbfs_sec_sz * (app_state.wbfs->n_wbfs_sec - block_count);
}

long long info_get_total_space(void)
{
  return (long long) app_state.wbfs->n_wbfs_sec * app_state.wbfs->wbfs_sec_sz;
}

long long info_get_iso_size(char *filename, void (*update)(int, int))
{
  FILE *f;
  unsigned int used_blocks;

  f = fopen(filename, "r");
  if (f == NULL)
    return -1LL;
  used_blocks = wbfs_count_added_disc_blocks(app_state.wbfs,
					     read_wii_file,
					     (void *) f,
					     update,
					     ONLY_GAME_PARTITION,
					     0);
  fclose(f);

  return (unsigned long long) app_state.wbfs->wbfs_sec_sz * used_blocks;
}

int op_add_iso(char *filename, void (*update)(int, int))
{
  FILE *f;
  wbfs_disc_t *disc;
  char code[7];
  int ret;

  cancel_wbfs_op = 0;
  if (! update)
    update = progress_update;

  /* open ISO */
  f = fopen(filename, "r");
  if (f == NULL) {
    show_error("Error Adding ISO", "Can't open ISO file '%s'", filename);
    return 1;
  }
  if (fread(code, 1, 6, f) != 6) {
    fclose(f);
    show_error("Error Adding ISO", "Can't read disc ID from file '%s'.", filename);
    return 1;
  }
  code[6] = '\0';

  /* check if disc is already there */
  disc = wbfs_open_disc(app_state.wbfs, (u8 *) code);
  if (disc != NULL) {
    wbfs_close_disc(disc);
    fclose(f);
    show_error("Error Adding ISO", "The disc is already in the WBFS partition.");
    return 1;
  }

  /* add disc */
  ret = wbfs_add_disc(app_state.wbfs, read_wii_file, (void *) f, update, ONLY_GAME_PARTITION, 0, NULL);
  
  fclose(f);
  return ret;
}

int op_init_partition(char *device)
{
  if (app_state.wbfs) {
    wbfs_close(app_state.wbfs);
    app_state.wbfs = NULL;
  }

  app_state.wbfs = wbfs_try_open_partition(device, 1);
  if (app_state.wbfs != NULL)
    return 0;
  return 1;
}

int op_remove_disc(char *code)
{
  if (wbfs_rm_disc(app_state.wbfs, (u8 *) code)) {
    show_error("Error Removing Disc", "Can't find disc id '%s'", code);
    return 1;
  }
  return 0;
}

int op_rename_disc(char *code, char *new_name)
{
  if (wbfs_ren_disc(app_state.wbfs, (u8 *) code, (u8 *) new_name)) {
    show_error("Error Renaming Disc", "Can't find disc id '%s'", code);
    return 1;
  }
  return 0;
}
