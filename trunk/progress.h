/* progress.c
 *
 * Copyright (C) 2009 Ricardo Massaro
 *
 * Licensed under the terms of the GNU GPL, version 2
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#ifndef PROGRESS_H_FILE
#define PROGRESS_H_FILE

typedef void (*progress_updater)(int cur, int max);
typedef int (*progress_starter)(void *data, progress_updater updater);

int show_progress_dialog(const char *title,
			 const char *message,
			 progress_starter starter,
			 void *starter_data,
			 progress_updater updater,
			 int *cancel_indicator);

#endif /* PROGRESS_H_FILE */
