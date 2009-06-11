/* message.h
 *
 * Copyright (C) 2009 Ricardo Massaro
 *
 * Licensed under the terms of the GNU GPL, version 2
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#ifndef MESSAGE_H_FILE
#define MESSAGE_H_FILE

int show_text_input(const char *title, char *input, int input_size, const char *s, ...);

int show_warning_yes_no(const char *title, const char *s, ...);
int show_confirmation(const char *title, const char *s, ...);
void show_message(const char *title, const char *s, ...);
void show_error(const char *title, const char *s, ...);

void start_msg_capture(void);
char *end_msg_capture(void);

#endif /* MESSAGE_H_FILE */
