///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002, Industrial Light & Magic, a division of Lucas
// Digital Ltd. LLC
// 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// *       Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// *       Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
// *       Neither the name of Industrial Light & Magic nor the names of
// its contributors may be used to endorse or promote products derived
// from this software without specific prior written permission. 
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////



#ifndef INCLUDED_IMATHEXC_H
#define INCLUDED_IMATHEXC_H


//-----------------------------------------------
//
//	Imath library-specific exceptions
//
//-----------------------------------------------

#include "IexBaseExc.h"

#ifdef OPENEXR_DLL
    #ifdef IMATH_EXPORTS
        #define IMATH_EXPORT __declspec(dllexport)
        #define IMATH_EXPORT_CONST extern __declspec(dllexport)
    #else
        #define IMATH_EXPORT __declspec(dllimport)
        #define IMATH_EXPORT_CONST extern __declspec(dllimport)
    #endif
#else
    #define IMATH_EXPORT
    #define IMATH_EXPORT_CONST extern const
#endif

//-----------------------------------------------------
// A macro to save typing when declararing an exception
// class derived directly or indirectly from BaseExc:
//-----------------------------------------------------

#define DEFINE_IMATH_EXC(name, base)				        \
class IMATH_EXPORT name: public base				        \
    {							        \
    public:                                                   \
    name (const char* text=0)      throw(): base (text) {}	\
    name (const std::string &text) throw(): base (text) {}	\
    name (std::stringstream &text) throw(): base (text) {}	\
    };

namespace Imath {


DEFINE_IMATH_EXC (NullVecExc, ::Iex::MathExc)		// Attempt to normalize
						// null vector

DEFINE_IMATH_EXC (InfPointExc, ::Iex::MathExc)	// Attempt to normalize
                                                // a point at infinity

DEFINE_IMATH_EXC (NullQuatExc, ::Iex::MathExc) 	// Attempt to normalize
						// null quaternion

DEFINE_IMATH_EXC (SingMatrixExc, ::Iex::MathExc)	// Attempt to invert
						// singular matrix

DEFINE_IMATH_EXC (ZeroScaleExc, ::Iex::MathExc)	// Attempt to remove zero
						// scaling from matrix

DEFINE_IMATH_EXC (IntVecNormalizeExc, ::Iex::MathExc)	// Attempt to normalize
						// a vector of whose elements
                                                // are an integer type

} // namespace Imath

#endif
