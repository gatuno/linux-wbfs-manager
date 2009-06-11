/* app_state.h
 *
 * Copyright 2009 Ricardo Massaro
 *
 * Licensed under the terms of the GNU GPL, version 2
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#ifndef APP_STATE_H_FILE
#define APP_STATE_H_FILE

#include "libwbfs.h"

#define APP_MAX_DEVICES 256

typedef struct APP_STATE {
  /* configuration */
  int ignore_mounted_devices;

  /* data */
  int num_devs;
  char *dev[APP_MAX_DEVICES];
  int cur_dev;
  int def_dev;

  wbfs_t *wbfs;
} APP_STATE;

extern APP_STATE app_state;

void app_init(void);
void app_reload_device_list(void);

#endif /* APP_STATE_H_FILE */
