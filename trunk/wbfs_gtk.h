/* wbfs_gtk.h
 *
 * Copyright (C) 2009 Ricardo Massaro
 *
 * Licensed under the terms of the GNU GPL, version 2
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#ifndef WBFS_GTK_H_FILE
#define WBFS_GTK_H_FILE

#include <gtk/gtk.h>
#include <glade/glade.h>

extern GladeXML *glade_xml;

GtkResponseType show_dialog_message(const char *title, const char *msg, GtkMessageType type, GtkButtonsType buttons);

#endif /* WBFS_GTK_H_FILE */
