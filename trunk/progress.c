/* progress.c
 *
 * Copyright (C) 2009 Ricardo Massaro
 *
 * Licensed under the terms of the GNU GPL, version 2
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#include <stdlib.h>
#include <stdio.h>

#include <gtk/gtk.h>
#include <glade/glade.h>

#include "progress.h"
#include "wbfs_gtk.h"
#include "app_state.h"
#include "message.h"

static int *prog_cancel_indicator;
static progress_starter prog_starter;
static void *prog_starter_data;
static int prog_starter_ret;
static progress_updater prog_updater;

static void do_update(int cur, int max)
{
  /* process real updater */
  if (prog_updater)
    prog_updater(cur, max);

  /* process GTK events */
  while (gtk_events_pending())
    gtk_main_iteration();
}

void progress_dialog_close_cb(GtkDialog *d, gpointer data)
{
  *prog_cancel_indicator = 1;
}

void progress_dialog_map_event_cb(GtkWidget *w, gpointer data)
{
  if (prog_starter_data != NULL) {
    GtkWidget *progress_dialog;

    prog_starter_ret = prog_starter(prog_starter_data, do_update);
    progress_dialog = glade_xml_get_widget(glade_xml, "progress_dialog");
    gtk_widget_hide(progress_dialog);
  }
}

void progress_button_clicked_cb(GtkButton *b, gpointer data)
{
  *prog_cancel_indicator = 1;
}

int show_progress_dialog(const char *title,
			 const char *message,
			 progress_starter starter,
			 void *data,
			 progress_updater updater,
			 int *cancel_indicator)
{
  GtkWidget *progress_dialog;
  GtkWidget *widget;
  int resp;

  /* reset progress data */
  prog_cancel_indicator = cancel_indicator;
  prog_starter = starter;
  prog_starter_data = data;
  prog_starter_ret = 0;
  prog_updater = updater;

  /* set message */
  widget = glade_xml_get_widget(glade_xml, "progress_message");
  gtk_label_set_text(GTK_LABEL(widget), message);

  /* empty progress */
  widget = glade_xml_get_widget(glade_xml, "progress_progress");
  gtk_label_set_text(GTK_LABEL(widget), "");
  widget = glade_xml_get_widget(glade_xml, "progress_bar");
  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(widget), 0.);

  /* show dialog */
  progress_dialog = glade_xml_get_widget(glade_xml, "progress_dialog");
  gtk_window_set_title(GTK_WINDOW(progress_dialog), title);
  gtk_widget_show_all(progress_dialog);
  resp = gtk_dialog_run(GTK_DIALOG(progress_dialog));
  gtk_widget_hide(progress_dialog);

  /* reset data */
  prog_cancel_indicator = NULL;
  prog_starter = NULL;
  prog_starter_data = NULL;
  prog_updater = NULL;

  return prog_starter_ret;
}
