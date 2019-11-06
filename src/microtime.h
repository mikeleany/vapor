/******************************************************************************
 * $Id: microtime.h 10 2010-04-24 17:09:01Z mike $
 * Project: Vapor Chess
 * Purpose: Functions and macros for handling time in microsecond.
 * 
 * Copyright 2010 by Michael Leany
 * All rights reserved
 */

#ifndef VAPOR__MICROTIME_H
#define VAPOR__MICROTIME_H

#include "vapor.h"

#include <sys/time.h>

/* micro-second time */
typedef int64 microtime;

static inline microtime getMicroTime(void)
{
  struct timeval Time;

  gettimeofday(&Time, NULL);

  return (microtime)Time.tv_sec*1000000 + Time.tv_usec;
}

#define MILLISEC_PER_SEC  1000
#define CENTISEC_PER_SEC  100
#define SEC_PER_MIN   60
#define MIN_PER_HR    60
#define SEC_PER_HR    (SEC_PER_MIN*MIN_PER_HR)
#define ONE_MILLISEC  1000
#define ONE_CENTISEC  10000
#define ONE_SEC       1000000
#define ONE_MIN       (SEC_PER_MIN*ONE_SEC)
#define ONE_HR        (MIN_PER_HR*ONE_MIN)

#define toMillisec(t)   ((int64)(t)/ONE_MILLISEC)
#define toCentisec(t)   ((int64)(t)/ONE_CENTISEC)
#define toSeconds(t)    ((int64)(t)/ONE_SEC)
#define toMinutes(t)    ((int64)(t)/ONE_MIN)
#define toHours(t)      ((int64)(t)/ONE_HR)

#define mSecPart(t) (toMillisec(t)%MILLISEC_PER_SEC)
#define cSecPart(t) (toCentisec(t)%CENTISEC_PER_SEC)
#define secondsPart(t)  (toSeconds(t)%SEC_PER_MIN)
#define minutesPart(t)  (toMinutes(t)%MIN_PER_HR)

#endif // #ifndef VAPOR__MICROTIME_H

/* end of file */
