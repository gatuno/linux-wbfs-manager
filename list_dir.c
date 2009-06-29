/* list_dir.c
 *
 * Copyright (C) 2009 Ricardo Massaro
 *
 * Licensed under the terms of the GNU GPL, version 2
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "config.h"
#include "list_dir.h"

static int compare_file_names(const void *p1, const void *p2)
{
  char *s1 = *(char **) p1;
  char *s2 = *(char **) p2;
  
  return strcmp(s1, s2);
}

static int compare_dir_items(const void *p1, const void *p2)
{
  DIR_ITEM *i1 = (DIR_ITEM *) p1;
  DIR_ITEM *i2 = (DIR_ITEM *) p2;

  if ((i1->is_dir == 1 && i2->is_dir == 1)
      || (i1->is_dir != 1 && i2->is_dir != 1))
    return strcmp(i1->name, i2->name);

  return (i1->is_dir == 1) ? -1 : 1;
}

int list_dir(const char *dir_name, const char *ext, char **list, int max_items)
{
  DIR *dir;
  struct dirent *ent;
  int n;

  dir = opendir(dir_name);
  if (! dir)
    return 1;
  n = 0;
  while (n+1 < max_items && (ent = readdir(dir)) != NULL) {
    int name_len = strlen(ent->d_name);
    if (ext != NULL) {
      int ext_len = strlen(ext);
      
      if (name_len < ext_len || strcmp(ent->d_name + name_len - ext_len, ext) != 0)
        continue;
    }
    list[n] = malloc(name_len + 1);
    strcpy(list[n], ent->d_name);
    n++;
  }
  list[n] = NULL;
  closedir(dir);

  qsort(list, n, sizeof(char *), compare_file_names);
  return 0;
}

int list_dir_attr(const char *dir_name, const char *ext, unsigned int flags, DIR_ITEM *list, int max_items)
{
  DIR *dir;
  struct dirent *ent;
  int n;

  dir = opendir(dir_name);
  if (! dir)
    return 1;
  n = 0;
  while (n+1 < max_items && (ent = readdir(dir)) != NULL) {
    struct stat st;
    int name_len;
    char path[PATH_MAX];
    int is_dir;
    unsigned long long size;

    /* ignore hidden */
    if ((flags & LISTDIR_SHOW_HIDDEN) == 0) {
      if (ent->d_name[0] == '.' && ent->d_name[1] != '\0' && ent->d_name[1] != '.')
        continue;
    }

    /* get item info */
    snprintf(path, sizeof(path), "%s/%s", dir_name, ent->d_name);
    if (stat(path, &st) != 0) {
      is_dir = -1;
      size = 0;
    } else {
      is_dir = S_ISDIR(st.st_mode) ? 1 : 0;
      size = st.st_size;
    }

    /* check extension (if not directory) */
    name_len = strlen(ent->d_name);
    if (ext != NULL && ! is_dir) {
      int ext_len = strlen(ext);
      
      if (name_len < ext_len || strcmp(ent->d_name + name_len - ext_len, ext) != 0)
        continue;
    }

    /* add item */
    list[n].name = malloc(name_len + 1);
    strcpy(list[n].name, ent->d_name);
    list[n].is_dir = is_dir;
    list[n].size = size;
    n++;
  }
  list[n].name = NULL;
  list[n].size = 0xffffffffffffffllu;
  list[n].is_dir = -2;
  closedir(dir);

  qsort(list, n, sizeof(DIR_ITEM), compare_dir_items);
  return 0;
}
