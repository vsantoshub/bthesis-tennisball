/*
 * File:	debug.h
 * Module:
 * Project:	
 * Author:   Victor Santos (viic.santos@gmail.com)
 * Target:
 * Comment:
 *
 */

#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef __cplusplus
extern "C" {
#endif

#define DBG_NONE      0
#define DBG_PANIC     1
#define DBG_EXCEPT    2
#define DBG_ERROR     3
#define DBG_WARNING   4
#define DBG_TRACE     5
#define DBG_INFO      6
#define DBG_MSG       7

#ifdef DEBUG_LEVEL
#ifndef DEBUG
#define DEBUG
#endif
#endif

#ifdef DEBUG 
#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL DBG_TRACE
#endif
#endif

#ifdef DEBUG
#include <stdio.h>
#include <stdlib.h>

static volatile int DBG_IDENT_VAR = 0;
static volatile int DBG_COND_VAR = 1;

#define DBG(LEVEL, __FMT, ...) if (1) { if (LEVEL <= DEBUG_LEVEL)  { \
		fprintf(stderr, "%s: ", __FUNCTION__); \
		if (LEVEL == DBG_PANIC) fprintf(stderr, "PANIC: "); \
		if (LEVEL == DBG_ERROR) fprintf(stderr, "ERROR: "); \
		if (LEVEL == DBG_WARNING) fprintf(stderr, "WARNING: "); \
		if (LEVEL == DBG_TRACE) fprintf(stderr, "TRACE: "); \
		if (LEVEL == DBG_INFO) fprintf(stderr, "INFO: "); \
		if (LEVEL == DBG_MSG) fprintf(stderr, "MSG: "); \
		fprintf(stderr, __FMT, ## __VA_ARGS__); \
		fprintf(stderr, "\n"); }} else (void) 0

#define DBG_DUMP(LEVEL, P, N) if (1) { if (LEVEL <= DEBUG_LEVEL)  { \
		fprintf(stderr, "%s: ", __FUNCTION__); \
		if (LEVEL == DBG_PANIC) fprintf(stderr, "PANIC:"); \
		if (LEVEL == DBG_ERROR) fprintf(stderr, "ERROR:"); \
		if (LEVEL == DBG_WARNING) fprintf(stderr, "WARNING:"); \
		if (LEVEL == DBG_TRACE) fprintf(stderr, "TRACE:"); \
		if (LEVEL == DBG_INFO) fprintf(stderr, "INFO:"); \
		if (LEVEL == DBG_MSG) fprintf(stderr, "MSG: "); \
		{ int __i; for (__i = 0; __i < N; __i++) \
			fprintf(stderr, " %02x", ((unsigned char *)P)[__i]); } \
		fprintf(stderr, "\n"); }} else (void) 0

#define DBG_PUTC(LEVEL, C) if (1) { if (LEVEL <= DEBUG_LEVEL)  { \
		fputc((C), stderr); }} else (void) 0

#define DBG_PUTS(LEVEL, S) if (1) { if (LEVEL <= DEBUG_LEVEL)  { \
		fputs((S), stderr); }} else (void) 0

#define DBG_PRINTF(LEVEL, __FMT, ...) if (1) { if (LEVEL <= DEBUG_LEVEL)  { \
	fprintf(stderr, __FMT, ## __VA_ARGS__); }} else (void) 0

#define DBG_IDENT_INC(LEVEL)  if (1) { if (LEVEL <= DEBUG_LEVEL)  { \
		DBG_IDENT_VAR++; }} else (void) 0

#define DBG_IDENT_DEC(LEVEL)  if (1) { if (LEVEL <= DEBUG_LEVEL)  { \
		DBG_IDENT_VAR--; }} else (void) 0

#define DBG_IDENT(LEVEL)    if (1) { if (LEVEL <= DEBUG_LEVEL)  { \
		{ int __i; for (__i = 0; __i < DBG_IDENT_VAR; __i++) \
			fprintf(stderr, "\t"); }}} else (void) 0

#define DBG_SETCOND(LEVEL, N)  if (1) { if (LEVEL <= DEBUG_LEVEL)  { \
		DBG_COND_VAR = N; }} else (void) 0

#define DBG_COND(LEVEL, __FMT, ...) if (1) { if (LEVEL <= DEBUG_LEVEL)  { \
        if (DBG_COND_VAR == 1) fprintf(stderr, __FMT, ## __VA_ARGS__); }} else (void) 0


#else /* not DEBUG */
#define DBG(LEVEL, __FMT, ...)
#define DBG_PRINTF(LEVEL, __FMT, ...)
#define DBG_DUMP(LEVEL, P, N)
#define DBG_PUTC(LEVEL, C)
#define DBG_PUTS(LEVEL, S)
#define DBG_IDENT(LEVEL)
#define DBG_IDENT_INC(LEVEL)
#define DBG_IDENT_DEC(LEVEL)
#define DBG_SETCOND(LEVEL, N)
#define DBG_COND(LEVEL, __FMT, ...)
#endif /* not DEBUG */

#ifdef __cplusplus
}
#endif

#endif /* not __DEBUG_H__ */

