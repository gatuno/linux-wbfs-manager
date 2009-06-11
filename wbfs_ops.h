/* wbfs_ops.h
 *
 * Copyright (C) 2009 Ricardo Massaro
 *
 * Licensed under the terms of the GNU GPL, version 2
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#ifndef WBFS_OPS_H_FILE
#define WBFS_OPS_H_FILE

extern int cancel_wbfs_op;

void dump_wbfs_info(void);
int op_init_partition(char *device);
int op_extract_iso(char *code, char *filename, void (*progress_update)(int, int));
int op_add_iso(char *filename, void (*update)(int, int));
int op_remove_disc(char *code);
int op_rename_disc(char *code, char *new_name);

#endif /* WBFS_OPS_H_FILE */
