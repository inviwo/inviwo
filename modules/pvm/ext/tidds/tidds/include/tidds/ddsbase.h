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

#ifndef DDSBASE_H
#define DDSBASE_H

#undef TIDDS_EXT

#if defined(_WIN32)

#ifdef INVIWO_ALL_DYN_LINK
#ifdef TIDDS_EXPORTS
#define TIDDS_EXT __declspec(dllexport)
#else
#define TIDDS_EXT extern __declspec(dllimport)
#endif
#else
#ifdef TIDDS_EXPORTS
#define TIDDS_EXT
#else
#define TIDDS_EXT extern
#endif
#endif

#else

#if defined(__GNUC__) && __GNUC__>=4
#define TIDDS_EXT extern __attribute__ ((visibility("default")))
#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
#define TIDDS_EXT extern __global
#else
#define TIDDS_EXT extern
#endif

#endif

#ifndef TIDDS_EXT
#define TIDDS_EXT extern
#endif

#include "codebase.h" // universal code base

TIDDS_EXT void writeDDSfile(const char *filename, unsigned char *data, size_t bytes, unsigned int skip = 0, unsigned int strip = 0, int nofree = 0);
TIDDS_EXT unsigned char *readDDSfile(const char *filename, size_t *bytes);

TIDDS_EXT void writeRAWfile(const char *filename, unsigned char *data, size_t bytes, int nofree = 0);
TIDDS_EXT unsigned char *readRAWfile(const char *filename, size_t *bytes);

TIDDS_EXT void writePNMimage(const char *filename, unsigned const char *image, unsigned int width, unsigned int height, unsigned int components, int dds = 0);
TIDDS_EXT unsigned char *readPNMimage(const char *filename, unsigned int *width, unsigned int *height, unsigned int *components);

TIDDS_EXT void writePVMvolume(const char *filename, unsigned const char *volume,
                    unsigned int width,unsigned int height,unsigned int depth,unsigned int components=1,
                    float scalex=1.0f,float scaley=1.0f,float scalez=1.0f,
                    unsigned const char *description=NULL,
                    unsigned const char *courtesy=NULL,
                    unsigned const char *parameter=NULL,
                    unsigned const char *comment=NULL);

TIDDS_EXT unsigned char *readPVMvolume(const char *filename,
                             unsigned int *width,unsigned int *height,unsigned int *depth,unsigned int *components=NULL,
                             float *scalex=NULL,float *scaley=NULL,float *scalez=NULL,
                             unsigned char **description=NULL,
                             unsigned char **courtesy=NULL,
                             unsigned char **parameter=NULL,
                             unsigned char **comment=NULL);

TIDDS_EXT unsigned int checksum(unsigned const char *data, unsigned int bytes);

TIDDS_EXT void swapbytes(unsigned char *data, unsigned int bytes);
TIDDS_EXT void convbytes(unsigned char *data, unsigned int bytes);
TIDDS_EXT void convfloat(unsigned char *data, unsigned int bytes);

TIDDS_EXT unsigned char *quantize(unsigned char *volume,
                        unsigned int width,unsigned int height,unsigned int depth,
                        BOOLINT nofree=FALSE,
                        BOOLINT linear=FALSE,
                        BOOLINT verbose=FALSE);

#endif