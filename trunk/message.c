/* message.c
 *
 * Copyright (C) 2009 Ricardo Massaro
 *
 * Licensed under the terms of the GNU GPL, version 2
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <gtk/gtk.h>
#include <glade/glade.h>

#include "message.h"
#include "wbfs_gtk.h"

static int capturing_msgs = 0;
static char captured_msgs[1024];
static int captured_msgs_size;

static GtkResponseType show_dialog_message(const char *title, const char *msg, GtkMessageType type, GtkButtonsType buttons)
{
  GtkWidget *main_window;
  GtkWidget *dlg;
  GtkResponseType resp;

  main_window = get_widget("main_window");
  dlg = gtk_message_dialog_new(GTK_WINDOW(main_window),
			       GTK_DIALOG_MODAL,
			       type,
			       buttons,
			       "%s",
			       msg);
  if (title != NULL)
    gtk_window_set_title(GTK_WINDOW(dlg), title);
  resp = gtk_dialog_run(GTK_DIALOG(dlg));
  gtk_widget_destroy(dlg);
  return resp;
}

static void capture_message(char *msg)
{
  strncpy(captured_msgs + captured_msgs_size,
	  msg,
	  sizeof(captured_msgs) - captured_msgs_size);
  captured_msgs[sizeof(captured_msgs)-1] = '\0';
  captured_msgs_size += strlen(captured_msgs + captured_msgs_size);
}

void start_msg_capture(void)
{
  captured_msgs[0] = '\0';
  captured_msgs_size = 0;
  capturing_msgs = 1;
}

char *end_msg_capture(void)
{
  capturing_msgs = 0;
  return captured_msgs;
}

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

  if (capturing_msgs)
    capture_message(msg);
  else
    show_dialog_message(title, msg, GTK_MESSAGE_INFO, GTK_BUTTONS_OK);
}

void show_error(const char *title, const char *s, ...)
{
  va_list args;
  char msg[256];

  va_start(args, s);
  vsnprintf(msg, sizeof(msg), s, args);
  va_end(args);

  if (capturing_msgs)
    capture_message(msg);
  else
    show_dialog_message(title, msg, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK);
}
