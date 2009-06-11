/* libwbfs_os.c
 *
 * Copyright 2009 Ricardo Massaro
 *
 * Licensed under the terms of the GNU GPL, version 2
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */


#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "libwbfs_os.h"
#include "message.h"

void wbfs_fatal(const char *s, ...)
{
  va_list args;
  char msg[256];

  va_start(args, s);
  vsnprintf(msg, sizeof(msg), s, args);
  va_end(args);

  show_error("FATAL ERROR", "Fatal error from libwbfs:\n%s\n\nApplication will now terminate.", msg);
  exit(1);
}

void wbfs_error(const char *s, ...)
{
  va_list args;
  char msg[256];

  va_start(args, s);
  vsnprintf(msg, sizeof(msg), s, args);
  va_end(args);

  show_error("Error", "%s", msg);
}

void wbfs_warning(const char *s, ...)
{
  va_list args;
  char msg[256];

  va_start(args, s);
  vsnprintf(msg, sizeof(msg), s, args);
  va_end(args);

  show_message("Warning", "%s", msg);
}
