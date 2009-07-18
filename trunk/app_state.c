/* app_state.h
 *
 * Copyright (C) 2009 Ricardo Massaro
 *
 * Licensed under the terms of the GNU GPL, version 2
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "app_state.h"
#include "devices.h"

APP_STATE app_state;

void app_init(void)
{
  app_state.num_devs = 0;
  app_state.ignore_mounted_devices = 1;
  app_state.list_partitions = 1;
  app_state.wbfs = NULL;
  app_state.cur_dev = -1;
  app_state.def_dev = -1;
}

void app_reload_device_list(void)
{
  int i;
  unsigned int flags;

  for (i = 0; i < app_state.num_devs; i++)
    free(app_state.dev[i]);

  flags = 0;
  if (app_state.ignore_mounted_devices)
    flags |= LISTDEV_SKIP_MOUNTED;
  if (app_state.list_partitions)
    flags |= LISTDEV_LIST_PARTITIONS;
  app_state.num_devs = list_available_devices(app_state.dev, APP_MAX_DEVICES, &app_state.def_dev, flags);
}
