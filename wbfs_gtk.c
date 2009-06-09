/* wbfs_gtk.c
 *
 * Copyright (C) 2009 Ricardo Massaro
 *
 * Licensed under the terms of the GNU GPL, version 2
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <pwd.h>

#include <gtk/gtk.h>
#include <glade/glade.h>

#include "libwbfs.h"
#include "app_state.h"
#include "list_dir.h"

#define GLADE_XML_FILE "wbfs_gui.glade"

static GladeXML *glade_xml;
static char cur_directory[PATH_MAX];
static DIR_ITEM cur_dir_list[1024];

void show_dialog_message(const char *title, const char *msg, int is_error)
{
  GtkWidget *main_window;
  GtkWidget *dlg;

  main_window = glade_xml_get_widget(glade_xml, "main_window");
  dlg = gtk_message_dialog_new(GTK_WINDOW(main_window),
			       GTK_DIALOG_MODAL,
			       (is_error) ? GTK_MESSAGE_ERROR : GTK_MESSAGE_INFO,
			       GTK_BUTTONS_OK,
			       "%s",
			       msg);
  if (title != NULL)
    gtk_window_set_title(GTK_WINDOW(dlg), title);
  gtk_dialog_run(GTK_DIALOG(dlg));
  gtk_widget_destroy(dlg);
}

void show_message(const char *title, const char *s, ...)
{
  va_list args;
  char msg[256];

  va_start(args, s);
  vsnprintf(msg, sizeof(msg), s, args);
  va_end(args);

  show_dialog_message(title, msg, 0);
}

void show_error(const char *title, const char *s, ...)
{
  va_list args;
  char msg[256];

  va_start(args, s);
  vsnprintf(msg, sizeof(msg), s, args);
  va_end(args);

  show_dialog_message(title, msg, 1);
}

/**
 * Update the filesystem directory list.
 */
static void update_fs_list(void)
{
  GtkListStore *store;
  GtkTreeIter iter;
  GtkWidget *widget;
  GtkTreeView *fs_list;
  GtkEntry *fs_cur_dir;
  int i;

  widget = glade_xml_get_widget(glade_xml, "fs_list");
  fs_list = GTK_TREE_VIEW(widget);
  store = GTK_LIST_STORE(gtk_tree_view_get_model(fs_list));
  widget = glade_xml_get_widget(glade_xml, "fs_cur_dir");
  fs_cur_dir = GTK_ENTRY(widget);

  /* get current directory and copy it to current dir display */
  if (getcwd(cur_directory, sizeof(cur_directory)) == NULL)
    strcpy(cur_directory, "/");
  gtk_entry_set_text(fs_cur_dir, cur_directory);
  
  gtk_list_store_clear(store);

  /* free old dir list */
  for (i = 0; cur_dir_list[i].name != NULL; i++) {
    free(cur_dir_list[i].name);
    cur_dir_list[i].name = NULL;
  }

  /* read new dir list */
  if (list_dir_attr(cur_directory, "iso", cur_dir_list, sizeof(cur_dir_list)/sizeof(cur_dir_list[0])) == 0) {
    for (i = 0; cur_dir_list[i].name != NULL; i++) {
      char size[32];

      if (cur_dir_list[i].is_dir)
        size[0] = '\0';
      else {
        if (cur_dir_list[i].size < 1024)
          snprintf(size, sizeof(size), "%llu B", cur_dir_list[i].size);
        else if (cur_dir_list[i].size < 1024*1024)
          snprintf(size, sizeof(size), "%.2f kB", cur_dir_list[i].size / 1024.);
        else if (cur_dir_list[i].size < 1024*1024*1024)
          snprintf(size, sizeof(size), "%.2f MB", cur_dir_list[i].size / 1024. / 1024.);
        else
          snprintf(size, sizeof(size), "%.2f GB", cur_dir_list[i].size / 1024. / 1024. / 1024.);
      }

      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter,
			 0, cur_dir_list[i].is_dir,
			 1, cur_dir_list[i].name,
			 2, size,
			 -1);
    }
  }
  if (GTK_WIDGET_REALIZED(fs_list))
    gtk_tree_view_scroll_to_point(fs_list, 0, 0);
}

/**
 * Update the device ROM list
 */
static void update_rom_list(void)
{
  GtkListStore *store;
  GtkTreeIter iter;
  GtkWidget *widget;
  GtkTreeView *rom_list;
  u8 *buf;
  u32 size;
  int i, n;

  widget = glade_xml_get_widget(glade_xml, "rom_list");
  rom_list = GTK_TREE_VIEW(widget);
  store = GTK_LIST_STORE(gtk_tree_view_get_model(rom_list));

  gtk_list_store_clear(store);

  if (app_state.wbfs == NULL)
    return;

  n = wbfs_count_discs(app_state.wbfs);
  buf = wbfs_ioalloc(0x100);
  for (i = 0; i < n; i++)
    if (wbfs_get_disc_info(app_state.wbfs, i, buf, 0x100, &size) == 0) {
      u8 code_txt[6], size_txt[20], *name_txt;

      memcpy(code_txt, buf, 5);
      code_txt[5] = '\0';
      snprintf((char *)size_txt, sizeof(size_txt), "%.2f GB", (size * 4ULL) / 1024.0 / 1024.0 / 1024.0);
      name_txt = buf + 0x20;

      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter,
			 0, code_txt,
			 1, name_txt,
			 2, size_txt,
			 -1);
    }
  wbfs_iofree(buf);
}

/**
 * Change the current directory to the directory in the "fs_cur_dir" entry.
 */
static void change_cur_dir(void)
{
  GtkWidget *widget;
  GtkEntry *fs_cur_dir;
  char full_path[PATH_MAX];

  widget = glade_xml_get_widget(glade_xml, "fs_cur_dir");
  fs_cur_dir = GTK_ENTRY(widget);
  strncpy(full_path, gtk_entry_get_text(fs_cur_dir), sizeof(full_path));
  full_path[sizeof(full_path)-1] = '\0';

  if (chdir(full_path) != 0)
    show_error("Error", "Can't change directory to '%s'.", full_path);
  else
    update_fs_list();
}

/**
 * Load the device 'app_state.cur_dev'
 */
static void reload_device(void)
{
  /* close current device */
  if (app_state.wbfs != NULL)
    wbfs_close(app_state.wbfs);
  app_state.wbfs = NULL;
  update_rom_list();
  if (app_state.cur_dev < 0)
    return;

  /* open new device */
  app_state.wbfs = wbfs_try_open_partition(app_state.dev[app_state.cur_dev], 0);
  if (app_state.wbfs == NULL) {
    show_error("ERROR", "Can't open device '%s'.", app_state.dev[app_state.cur_dev]);
    return;
  }
  
  update_rom_list();
}

/**
 * Reload the list of devices
 */
static void reload_device_list(void)
{
  int i;
  GtkListStore *store;
  GtkTreeIter iter;
  GtkWidget *widget;
  GtkComboBox *dev_list;
  int new_sel_index;

  app_reload_device_list();

  widget = glade_xml_get_widget(glade_xml, "device_list");
  dev_list = GTK_COMBO_BOX(widget);
  store = GTK_LIST_STORE(gtk_combo_box_get_model(dev_list));

  /* get new selection index */
  new_sel_index = -1;
  if (gtk_combo_box_get_active_iter(dev_list, &iter)) {
    char *cur_sel = NULL;
    gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, 0, &cur_sel, -1);
    for (i = 0; i < app_state.num_devs; i++) {
      if (cur_sel != NULL && strcmp(cur_sel, app_state.dev[i]) == 0) {
	new_sel_index = i;
	break;
      }
    }
    if (cur_sel != NULL)
      g_free(cur_sel);
  }

  /* re-fill list */
  gtk_list_store_clear(store);
  for (i = 0; i < app_state.num_devs; i++) {
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, app_state.dev[i], -1);
  }

  /* set selection */
  if (new_sel_index < 0)
    new_sel_index = 0;
  gtk_combo_box_set_active(dev_list, new_sel_index);
}

static void fs_list_icon_data_func(GtkTreeViewColumn *tree_column,
				   GtkCellRenderer *cell,
				   GtkTreeModel *tree_model,
				   GtkTreeIter *iter,
				   gpointer data)
{
  GValue val = {0};
  int is_dir;
  char *name = NULL;

  gtk_tree_model_get(tree_model, iter, 0, &is_dir, 1, &name, -1);

  /* set 'val' to stock icon to use */
  g_value_init(&val, G_TYPE_STRING);
  if (strcmp(name, ".") == 0)
    g_value_set_static_string(&val, GTK_STOCK_REFRESH);
  else if (strcmp(name, "..") == 0)
    g_value_set_static_string(&val, GTK_STOCK_GO_UP);
  else {
    switch (is_dir) {
    case 0: g_value_set_static_string(&val, GTK_STOCK_FILE); break;
    case 1: g_value_set_static_string(&val, GTK_STOCK_DIRECTORY); break;
    default: g_value_set_static_string(&val, GTK_STOCK_DIALOG_WARNING);
    }
  }

  if (name != NULL)
    g_free(name);

  g_object_set_property(G_OBJECT(cell), "stock-id", &val);
}

static void init_widgets(void)
{
  GtkCellRenderer *renderer;
  GtkListStore *list_store;
  GtkTreeView *rom_list;
  GtkTreeView *fs_list;
  GtkComboBox *dev_list;
  GtkEntry *fs_cur_dir;
  GtkWidget *widget;
  GtkTreeViewColumn *col;

  /* setup device list store */
  widget = glade_xml_get_widget(glade_xml, "device_list");
  dev_list = GTK_COMBO_BOX(widget);
  list_store = gtk_list_store_new(1, G_TYPE_STRING);
  gtk_combo_box_set_model(dev_list, GTK_TREE_MODEL(list_store));
  g_object_unref(list_store);

  /* setup rom list store and model */
  widget = glade_xml_get_widget(glade_xml, "rom_list");
  rom_list = GTK_TREE_VIEW(widget);
  renderer = gtk_cell_renderer_text_new();
  gtk_tree_view_insert_column_with_attributes(rom_list, -1, "Code", renderer, "text", 0, NULL);
  renderer = gtk_cell_renderer_text_new();
  gtk_tree_view_insert_column_with_attributes(rom_list, -1, "Name", renderer, "text", 1, NULL);
  renderer = gtk_cell_renderer_text_new();
  gtk_tree_view_insert_column_with_attributes(rom_list, -1, "Size", renderer, "text", 2, NULL);
  list_store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
  gtk_tree_view_set_model(rom_list, GTK_TREE_MODEL(list_store));
  g_object_unref(list_store);
  col = gtk_tree_view_get_column(rom_list, 1);
  gtk_tree_view_column_set_expand(col, 1);

  /* setup fs list store and model */
  widget = glade_xml_get_widget(glade_xml, "fs_list");
  fs_list = GTK_TREE_VIEW(widget);
  renderer = gtk_cell_renderer_pixbuf_new();
  gtk_tree_view_insert_column_with_data_func(fs_list, -1, "", renderer, fs_list_icon_data_func, NULL, NULL);
  renderer = gtk_cell_renderer_text_new();
  gtk_tree_view_insert_column_with_attributes(fs_list, -1, "Name", renderer, "text", 1, NULL);
  renderer = gtk_cell_renderer_text_new();
  gtk_tree_view_insert_column_with_attributes(fs_list, -1, "Size", renderer, "text", 2, NULL);
  list_store = gtk_list_store_new(3, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING);
  gtk_tree_view_set_model(fs_list, GTK_TREE_MODEL(list_store));
  g_object_unref(list_store);
  col = gtk_tree_view_get_column(fs_list, 1);
  gtk_tree_view_column_set_expand(col, 1);

  /* setup fs dir */
  widget = glade_xml_get_widget(glade_xml, "fs_cur_dir");
  fs_cur_dir = GTK_ENTRY(widget);
  cur_dir_list[0].name = NULL;
  update_fs_list();
}

/* ---- Callbacks -------------------------------------- */

void fs_list_row_activated_cb(GtkTreeView *tree_view,
                              GtkTreePath *path,
                              GtkTreeViewColumn *column,
                              gpointer user_data)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  char *name;
  int is_dir;
  char full_path[PATH_MAX];

  model = gtk_tree_view_get_model(tree_view);
  gtk_tree_model_get_iter(model, &iter, path);
  gtk_tree_model_get(model, &iter, 0, &is_dir, 1, &name, -1);

  snprintf(full_path, sizeof(full_path), "%s/%s", cur_directory, name);
  g_free(name);

  switch (is_dir) {
  case 0:
    show_message("File Selected", "Selected '%s'.", full_path);
    break;

  case 1:
    if (chdir(full_path) != 0)
      show_error("Error", "Can't change directory to '%s'.", full_path);
    else
      update_fs_list();
    break;
    
  default:
    show_error("Error", "File '%s' has unsupported type.", full_path);
    break;
  }
}

void fs_go_home_clicked_cb(GtkWidget *w, gpointer data)
{
  char *login;
  struct passwd *pw;

  if ((login = getlogin()) == NULL || (pw = getpwnam(login)) == NULL)
    show_error("Error", "Can't get home directory for current user.");
  else if (chdir(pw->pw_dir) != 0)
    show_error("Error", "Can't change to directory '%s'.", pw->pw_dir);
  else
    update_fs_list();
}

void fs_set_dir_clicked_cb(GtkWidget *w, gpointer data)
{
  change_cur_dir();
}

void fs_cur_dir_activate_cb(GtkEntry *e, gpointer data)
{
  change_cur_dir();
}

void reload_device_clicked_cb(GtkWidget *w, gpointer data)
{
  GtkListStore *store;
  GtkTreeIter iter;
  GtkWidget *widget;
  GtkComboBox *dev_list;
  int i;
  char *cur_sel;

  /* get selected device */
  widget = glade_xml_get_widget(glade_xml, "device_list");
  dev_list = GTK_COMBO_BOX(widget);
  store = GTK_LIST_STORE(gtk_combo_box_get_model(dev_list));
  cur_sel = NULL;
  if (gtk_combo_box_get_active_iter(dev_list, &iter))
    gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, 0, &cur_sel, -1);
  if (cur_sel == NULL)
    return;

  /* set selection */
  for (i = 0; i < app_state.num_devs; i++)
    if (strcmp(app_state.dev[i], cur_sel) == 0) {
      app_state.cur_dev = i;
      if (app_state.cur_dev >= 0) {
	char device_name[32];
	widget = glade_xml_get_widget(glade_xml, "device_name");
	snprintf(device_name, sizeof(device_name), "<b>%s</b>", app_state.dev[app_state.cur_dev]);
	gtk_label_set_markup(GTK_LABEL(widget), device_name);
      }
      reload_device();
      return;
    }
}

void reload_device_list_clicked_cb(GtkWidget *w, gpointer data)
{
  reload_device_list();
}

void main_window_realize_cb(GtkWidget *w, gpointer data)
{
  reload_device_list();
}

void main_window_delete_event_cb(GtkWidget *w, gpointer data)
{
  gtk_main_quit();
}

void menu_quit_activate_cb(GtkWidget *w, gpointer data)
{
  gtk_main_quit();
}

void menu_about_activate_cb(GtkWidget *w, gpointer data)
{
  GtkWidget *main_window;
  GtkWidget *about_dialog;

  main_window = glade_xml_get_widget(glade_xml, "main_window");
  about_dialog = glade_xml_get_widget(glade_xml, "about_dialog");
  gtk_window_set_transient_for(GTK_WINDOW(about_dialog), GTK_WINDOW(main_window));
  gtk_dialog_run(GTK_DIALOG(about_dialog));
}

void about_dialog_response_cb(GtkWidget *w, gpointer data)
{
  GtkWidget *about_dialog;

  about_dialog = glade_xml_get_widget(glade_xml, "about_dialog");
  gtk_widget_hide(about_dialog);
}

int main(int argc, char *argv[])
{
  GtkWidget *main_window;

  app_init();
  gtk_init(&argc, &argv);
  glade_init();

  /* load glade XML */
  glade_xml = glade_xml_new(GLADE_XML_FILE, NULL, NULL);
  glade_xml_signal_autoconnect(glade_xml);
  init_widgets();

  /* show main window */
  main_window = glade_xml_get_widget(glade_xml, "main_window");
  gtk_widget_show(main_window);

  gtk_main();

  return 0;
}
