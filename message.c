/* message.c
 *
 * Copyright (C) 2009 Ricardo Massaro
 *
 * Licensed under the terms of the GNU GPL, version 2
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <gtk/gtk.h>
#include <glade/glade.h>

#include "message.h"
#include "wbfs_gtk.h"

int show_warning_yes_no(const char *title, const char *s, ...)
{
  va_list args;
  char msg[256];
  GtkResponseType resp;

  va_start(args, s);
  vsnprintf(msg, sizeof(msg), s, args);
  va_end(args);

  resp = show_dialog_message(title, msg, GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO);
  if (resp == GTK_RESPONSE_YES)
    return 1;
  return 0;
}

int show_confirmation(const char *title, const char *s, ...)
{
  va_list args;
  char msg[256];
  GtkResponseType resp;

  va_start(args, s);
  vsnprintf(msg, sizeof(msg), s, args);
  va_end(args);

  resp = show_dialog_message(title, msg, GTK_MESSAGE_INFO, GTK_BUTTONS_OK_CANCEL);
  if (resp == GTK_RESPONSE_OK)
    return 1;
  return 0;
}

void show_message(const char *title, const char *s, ...)
{
  va_list args;
  char msg[256];

  va_start(args, s);
  vsnprintf(msg, sizeof(msg), s, args);
  va_end(args);

  show_dialog_message(title, msg, GTK_MESSAGE_INFO, GTK_BUTTONS_OK);
}

void show_error(const char *title, const char *s, ...)
{
  va_list args;
  char msg[256];

  va_start(args, s);
  vsnprintf(msg, sizeof(msg), s, args);
  va_end(args);

  show_dialog_message(title, msg, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK);
}
