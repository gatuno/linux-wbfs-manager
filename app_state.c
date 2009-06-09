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
#include <glob.h>

#include "app_state.h"

APP_STATE app_state;

void app_init(void)
{
  app_state.num_devs = 0;
  app_state.wbfs = NULL;
}

void app_reload_device_list(void)
{
  int i;
  glob_t gl;

  for (i = 0; i < app_state.num_devs; i++)
    free(app_state.dev[i]);
  app_state.num_devs = 0;

  gl.gl_offs = 0;
  glob("/dev/hd*", 0, NULL, &gl);
  glob("/dev/sd*", GLOB_APPEND, NULL, &gl);
  for (i = 0; i < gl.gl_pathc && i < APP_MAX_DEVICES; i++)
    app_state.dev[i] = strdup(gl.gl_pathv[i]);
  app_state.num_devs = i;
}
