/******************************************************************************
 * $Id: vapor.h 12 2010-10-10 16:17:26Z mike $
 * Project: Vapor Chess
 * Purpose: DESCRIPTION
 * 
 * Copyright 2010 by Michael Leany
 * All rights reserved
 */

#ifdef VAPOR__TEMPLATE_H
#error "VAPOR__TEMPLATE_H defined."
#endif // #ifdef VAPOR__TEMPLATE_H

#ifndef VAPOR__VAPOR_H
#define VAPOR__VAPOR_H

#include <assert.h>
#include <stddef.h>
#include <inttypes.h>

/* fixed-size integer types */
typedef int8_t int8;
typedef uint8_t uint8;
typedef int16_t int16;
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;
typedef int64_t int64;
typedef uint64_t uint64;

/* printf/scanf codes for fixed-size integer types */
// NOTE: this assumes that the codes are the same for printf and scanf

#define _i8  PRIi8
#define _d8  PRId8
#define _u8  PRIu8
#define _o8  PRIo8
#define _x8  PRIx8
#define _X8  PRIX8

#define _i16 PRIi16
#define _d16 PRId16
#define _u16 PRIu16
#define _o16 PRIo16
#define _x16 PRIx16
#define _X16 PRIX16

#define _i32 PRIi32
#define _d32 PRId32
#define _u32 PRIu32
#define _o32 PRIo32
#define _x32 PRIx32
#define _X32 PRIX32

#define _i64 PRIi64
#define _d64 PRId64
#define _u64 PRIu64
#define _o64 PRIo64
#define _x64 PRIx64
#define _X64 PRIX64

#define STDIN   0
#define STDOUT  1
#define STDERR  2

#ifdef __GNUC__
#define asm __asm__
#endif // #ifdef __GNUC__

static inline int max(int a, int b)
{
  return (a>b)?a:b;
}

static inline int min(int a, int b)
{
  return (a<b)?a:b;
}

#endif // #ifndef VAPOR__VAPOR_H

/* end of file */
