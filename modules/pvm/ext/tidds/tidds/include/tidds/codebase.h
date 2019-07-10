/*
* Copyright (c) by Stefan Roettger
* All rights reserved.
*
* From the V^3 volume renderer <http://stereofx.org/volume.html>, license
* change to LGPL with kind permission from Stefan Roettger
*
* This library is free software; you can redistribute it and/or modify it under
* the terms of the GNU Lesser General Public License version 2.1 as published
* by the Free Software Foundation.
*
* This library is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
* details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this library; if not, write to the Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef CODEBASE_H
#define CODEBASE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <float.h>
#include <cmath>

#if defined(IRIX) || defined(LINUX) || defined(MACOSX)
#define UNIX
#endif

#ifdef _WIN32
#define WINOS
#endif

#include <time.h>
#ifdef UNIX
#include <sys/time.h>
#endif
#ifdef WINOS
#define NOMINMAX
#include <windows.h>
#include <winbase.h>
#endif

#ifdef UNIX
#include <sys/types.h>
#include <sys/stat.h>
#endif

#ifndef NULL
#define NULL (0)
#endif

#define BOOLINT char

#ifndef TRUE
#define TRUE (1)
#endif
#ifndef FALSE
#define FALSE (0)
#endif

#define ERRORMSG() errormsg(__FILE__,__LINE__)

inline void errormsg(const char *file, int line)
{
    fprintf(stderr, "fatal error in <%s> at line %d!\n", file, line);
    exit(EXIT_FAILURE);
}

#define PI (3.141593f)
#define RAD (PI/180.0f)

#ifndef MAXFLOAT
#define MAXFLOAT (FLT_MAX)
#endif

#undef ffloor
#define ffloor(x) floor((double)(x))
#undef fceil
#define fceil(x) ceil((double)(x))
#define ftrc(x) (int)ffloor(x)

inline double FABS(const double x) { return((x<0.0) ? -x : x); }
#define fabs(x) FABS(x)

inline int min(const int a, const int b) { return((a<b) ? a : b); }
inline double FMIN(const double a, const double b) { return((a<b) ? a : b); }
#define fmin(a,b) FMIN(a,b)

inline int max(const int a, const int b) { return((a>b) ? a : b); }
inline double FMAX(const double a, const double b) { return((a>b) ? a : b); }
#define fmax(a,b) FMAX(a,b)

inline int sqr(const int x) { return(x*x); }
inline double fsqr(const double x) { return(x*x); }

#undef fsqrt
#define fsqrt(x) sqrt((double)(x))

#undef fsin
#define fsin(x) sin((double)(x))
#undef fcos
#define fcos(x) cos((double)(x))
#undef ftan
#define ftan(x) tan((double)(x))

#undef fasin
#define fasin(x) asin((double)(x))
#undef facos
#define facos(x) acos((double)(x))
#undef fatan
#define fatan(x) atan((double)(x))

#undef fexp
#define fexp(x) exp((double)(x))
#undef flog
#define flog(x) log((double)(x))
#undef fpow
#define fpow(x,y) pow((double)(x),(double)(y))

#ifdef UNIX
#define GETRANDOM() drand48()
#endif
#ifdef WINOS
#define GETRANDOM() ((double)rand()/RAND_MAX)
#endif

inline double GETTIME()
{
#ifdef UNIX
    struct timeval t;
    gettimeofday(&t, NULL);
    return(t.tv_sec + t.tv_usec / 1.0E6);
#endif
#ifdef WINOS
    static int cpus = 0;
    if (cpus == 0)
    {
        SYSTEM_INFO SystemInfo;
        GetSystemInfo(&SystemInfo);
        cpus = SystemInfo.dwNumberOfProcessors;
    }
    if (cpus == 1)
    {
        LARGE_INTEGER freq, count;
        if (QueryPerformanceFrequency(&freq) == 0) ERRORMSG();
        QueryPerformanceCounter(&count);
        return((double)count.QuadPart / freq.QuadPart);
    }
    return((double)clock() / CLOCKS_PER_SEC);
#endif
    return 0.0;
}

inline double gettime()
{
    static double time;
    static BOOLINT settime = FALSE;

    if (!settime)
    {
        time = GETTIME();
        settime = TRUE;
    }

    return(GETTIME() - time);
}

inline void waitfor(double secs)
{
#ifdef UNIX
    struct timespec dt, rt;
    dt.tv_sec = ftrc(secs);
    dt.tv_nsec = ftrc(1.0E9*(secs - ftrc(secs)));
    while (nanosleep(&dt, &rt) != 0) dt = rt;
#else
    double time = gettime() + secs;
    while (gettime()<time);
#endif
}

inline double getclockticks()
{
    static double clockticks;
    static BOOLINT setclockticks = FALSE;

    if (!setclockticks)
    {
        double time = gettime();
        while (time == gettime());
        clockticks = 1.0 / (gettime() - time);
        setclockticks = TRUE;
    }

    return(clockticks);
}

#ifdef WIN32_VC

inline char *strdup(const char *str)
{
    int len;
    char *dup;
    if ((dup = (char *)malloc(len = strlen(str) + 1)) == NULL) ERRORMSG();
    memcpy(dup, str, len);
    return(dup);
}

inline int strcasecmp(const char *str1, const char *str2)
{
    const char *ptr1, *ptr2;
    for (ptr1 = str1, ptr2 = str2; tolower(*ptr1) == tolower(*ptr2) && *ptr1 != '\0' && *ptr2 != '\0'; ptr1++, ptr2++);
    return(*ptr2 - *ptr1);
}

#define snprintf _snprintf

#endif

#endif