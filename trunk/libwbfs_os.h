#ifndef LIBWBFS_OS_H
#define LIBWBFS_OS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include <arpa/inet.h>

/* basic types */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

/* for wbfs_fatal() */

extern jmp_buf fatal_jmp_buf;

void wbfs_fatal(const char *s, ...);
void wbfs_error(const char *s, ...);
void wbfs_warning(const char *s, ...);

#define wbfs_malloc(x) malloc(x)
#define wbfs_free(x) free(x)
#define wbfs_ioalloc(x) malloc(x)
#define wbfs_iofree(x) free(x)

#define wbfs_ntohl(x) ntohl(x)
#define wbfs_ntohs(x) ntohs(x)
#define wbfs_htonl(x) htonl(x)
#define wbfs_htons(x) htons(x)

#define wbfs_memcmp(x,y,z) memcmp(x,y,z)
#define wbfs_memcpy(x,y,z) memcpy(x,y,z)
#define wbfs_memset(x,y,z) memset(x,y,z)

#endif
