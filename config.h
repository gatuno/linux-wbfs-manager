/* config.c
 *
 * Copyright (C) 2009 Ricardo Massaro
 *
 * Licensed under the terms of the GNU GPL, version 2
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#ifndef CONFIG_H_FILE
#define CONFIG_H_FILE

#ifndef PATH_MAX
/* #warning PATH_MAX not defined */
#define PATH_MAX 1024
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#endif

#endif /* CONFIG_H_FILE */
