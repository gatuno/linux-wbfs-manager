/* devices.h
 *
 * Copyright (C) 2009 Ricardo Massaro
 *
 * Licensed under the terms of the GNU GPL, version 2
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#ifndef DEVICES_H_FILE
#define DEVICES_H_FILE

enum {
  LISTDEV_SKIP_MOUNTED = 0x01,
  LISTDEV_LIST_PARTITIONS = 0x02,
};

int is_device_mounted(const char *device, char *mount_point, int max_len);
int list_available_devices(char **list, int max_items, int *preferred, unsigned int flags);

#endif /* DEVICES_H_FILE */
