/*
Copyright (c) 2012 Matt Janisz

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef TICPP_API_INCLUDED
#define TICPP_API_INCLUDED

#undef TICPP_API

#if defined(_WIN32) | defined(WIN32) | defined(WINDOWS) | defined(_WINDOWS)
// define BUILD_TICPP_DLL when building TiCPP dll.
#ifdef BUILD_TICPP_DLL
#define TICPP_API __declspec(dllexport)
#define TICPP_EXT
#endif

// define TICPP_DLL when linking TiCPP dll.
#ifdef TICPP_DLL
#define TICPP_API __declspec(dllimport)
#define TICPP_EXT extern
#endif
#endif

#ifndef TICPP_API
// linux or windows-static library defines
#define TICPP_API
#define TICPP_EXT extern
#endif

#endif  // TICPP_API_INCLUDED
