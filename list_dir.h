/* list_dir.h
 *
 * Copyright (C) 2009 Ricardo Massaro
 *
 * Licensed under the terms of the GNU GPL, version 2
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#ifndef LIST_DIR_FILE
#define LIST_DIR_FILE

typedef struct DIR_ITEM {
  char *name;
  int is_dir;
  unsigned long long size;
} DIR_ITEM;

int list_dir(const char *dir_name, const char *ext, char **list, int max_items);
int list_dir_attr(const char *dir_name, const char *ext, DIR_ITEM *list, int max_items);

#endif /* LIST_DIR_FILE */
