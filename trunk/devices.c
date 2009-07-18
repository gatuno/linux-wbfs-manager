/* devices.c
 *
 * Copyright (C) 2009 Ricardo Massaro
 *
 * Licensed under the terms of the GNU GPL, version 2
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <dirent.h>
#include <glob.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "config.h"
#include "devices.h"

typedef struct MOUNT_ITEM {
  char *device;
  char *mount_point;
} MOUNT_ITEM;

/**
 * Remove '..' and '.' entries from path.
 */
static void normalize_path(char *path)
{
  char *buf, *tmp, *save;
  char *parts[256];
  char *used_parts[256];
  int num_parts, num_used_parts, i;

  save = NULL;  /* keep GCC happy */
  tmp = buf = strdup(path);

  /* split path in the '/'s */
  i = 0;
  if (path[0] == '/') {
    parts[0] = "";
    i = 1;
  }
  while (i < sizeof(parts)/sizeof(parts[0])) {
    parts[i] = strtok_r(tmp, "/", &save);
    if (parts[i] == NULL)
      break;
    tmp = NULL;
    i++;
  }
  num_parts = i;

  /* rebuild path with necessary parts */
  num_used_parts = 0;
  for (i = 0; i < num_parts; i++) {
    if (strcmp(parts[i], ".") == 0)
      continue;
    if (strcmp(parts[i], "..") == 0) {
      if (num_used_parts > 0)
	num_used_parts--;
      continue;
    }
    used_parts[num_used_parts++] = parts[i];
  }

  for (i = 0; i < num_used_parts; i++) {
    int len;

    len = strlen(used_parts[i]);
    memcpy(path, used_parts[i], len);
    path += len;
    if (i+1 < num_used_parts)
      *path++ = '/';
    *path = '\0';
  }

  free(tmp);
}

static void resolve_symlink(char *ret, const char *symlink, const char *link_dest, int max_len)
{
  const char *last_slash;
  int len;

  /* if link dest is absolute or symlink has no directories, return the link dest */
  if (link_dest[0] == '/' || (last_slash = strrchr(symlink, '/')) == NULL) {
    strncpy(ret, link_dest, max_len);
    ret[max_len-1] = '\0';
    return;
  }

  last_slash++;
  len = last_slash - symlink;

  /* there's no space on ret, just return now whith what we have */
  if (len >= max_len) {
    strncpy(ret, symlink, max_len);
    ret[max_len-1] = '\0';
    return;
  }
  
  strncpy(ret, symlink, len);
  strncpy(ret + len, link_dest, max_len - len);
  ret[max_len-1] = '\0';
  normalize_path(ret);
}

static void free_mounts(MOUNT_ITEM *mounts, int num_mounts)
{
  int i;

  for (i = 0; i < num_mounts; i++) {
    free(mounts[i].device);
    free(mounts[i].mount_point);
  }
}

static int read_mounts(MOUNT_ITEM *mounts, int max_items)
{
  FILE *f;
  char line[1024];
  int num_items = 0;

  f = fopen("/proc/mounts", "r");
  if (f == NULL)
    return -1;
  while (num_items < max_items) {
    MOUNT_ITEM *item;
    char link_dest[PATH_MAX];
    ssize_t link_size;
    char *p_device, *p_mount_point;
    char *p_save;

    line[0] = '\0';
    if (fgets(line, sizeof(line), f) == NULL)
      break;

    /* get device and mount point */
    p_device = strtok_r(line, " \t", &p_save);
    if (p_device == NULL)
      continue;
    p_mount_point = strtok_r(NULL, " \t", &p_save);
    if (p_mount_point == NULL)
      continue;

    /* check if device is a symbolic link */
    link_size = readlink(p_device, link_dest, sizeof(link_dest)-1);
    if (link_size >= 0) {
      link_dest[link_size] = '\0';
      if (link_dest[0] != '/') {
	char tmp[PATH_MAX];
	resolve_symlink(tmp, p_device, link_dest, sizeof(tmp));
	strcpy(link_dest, tmp);
      }
      p_device = link_dest;
    }
    item = mounts + num_items++;
    item->device = strdup(p_device);
    item->mount_point = strdup(p_mount_point);
  }
  fclose(f);
  return num_items;
}

static const char *check_device_mounted(const char *device, MOUNT_ITEM *mounts, int num_mounts)
{
  int i;

  for (i = 0; i < num_mounts; i++) {
    if (mounts[i].device[0] == '/') {
      if (memcmp(device, mounts[i].device, strlen(device)) == 0)
	return mounts[i].mount_point;
    } else {
      if (strcmp(device, mounts[i].device) == 0)
	return mounts[i].mount_point;
    }
  }
  return NULL;
}

static int check_partition_looks_wbfs(const char *device)
{
  int fd;
  char id[4];

  fd = open(device, O_RDONLY);
  if (fd < 0)
    return 0;
  if (read(fd, id, 4) != 4) {
    close(fd);
    return 0;
  }
  close(fd);
  return (memcmp(id, "WBFS", 4) == 0);
}

int is_device_mounted(const char *device, char *mount_point, int max_len)
{
  int num_mounts;
  MOUNT_ITEM mounts[256];
  const char *mp;

  num_mounts = read_mounts(mounts, sizeof(mounts)/sizeof(mounts[0]));
  mp = check_device_mounted(device, mounts, num_mounts);
  if (mp != NULL && mount_point != NULL) {
    strncpy(mount_point, mp, max_len);
    mount_point[max_len-1] = '\0';
  }
  free_mounts(mounts, num_mounts);

  return (mp != NULL) ? 1 : 0;
}

int list_available_devices(char **list, int max_items, int *preferred, unsigned int flags)
{
  MOUNT_ITEM mounts[256];
  glob_t gl;
  FILE *f;
  int num_items, num_mounts;
  int i;

  /* read mounted devices (if necessary) */
  if (flags & LISTDEV_SKIP_MOUNTED)
    num_mounts = read_mounts(mounts, sizeof(mounts)/sizeof(mounts[0]));
  else
    num_mounts = 0;

  /* read all available partitions */
  num_items = 0;
  if (flags & LISTDEV_LIST_PARTITIONS)
    f = fopen("/proc/partitions", "r");
  else
    f = NULL;
  if (f != NULL) {
    /* read /proc/partitions */
    char line[1024], *p, *save;
    char dev[256];
    int line_num = 0;

    while (42) {
      int n;

      line[0] = '\0';
      if (fgets(line, sizeof(line), f) == NULL)
	break;
      if (line_num++ == 0)
	continue;
      p = strtok_r(line, " \t\r\n", &save);
      n = 0;
      while (p != NULL && n++ < 3) {
	p = strtok_r(NULL, " \t\r\n", &save);
	if (p != NULL && n == 2 && (p[0] == '0' || p[0] == '1') && p[1] == '\0')
	  p = NULL;
      }

      if (p != NULL) {
	snprintf(dev, sizeof(dev), "/dev/%s", p);
	if (check_device_mounted(dev, mounts, num_mounts) == NULL)
	  list[num_items++] = strdup(dev);
      }	
    }
    fclose(f);
  } else {
    /* list /dev/[hs]d* */
    gl.gl_offs = 0;
    glob("/dev/hd*", 0, NULL, &gl);
    glob("/dev/sd*", GLOB_APPEND, NULL, &gl);
    for (i = 0; i < gl.gl_pathc && num_items < max_items; i++) {
      if (check_device_mounted(gl.gl_pathv[i], mounts, num_mounts) == NULL)
	list[num_items++] = strdup(gl.gl_pathv[i]);
    }
  }

  free_mounts(mounts, num_mounts);

  /* check if there's a preferred */
  if (preferred != NULL) {
    *preferred = -1;
    for (i = 0; i < num_items; i++)
      if (check_partition_looks_wbfs(list[i])) {
	*preferred = i;
	break;
      }
  }

  return num_items;
}
