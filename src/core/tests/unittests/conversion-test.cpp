/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/glm.h>

#include <limits>
#include <type_traits>

namespace inviwo {

using namespace util;

template <typename T, typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
T minv() { return T{0.0}; }

template <typename T, typename std::enable_if<!std::is_floating_point<T>::value, int>::type = 0>
T minv() { return std::numeric_limits<T>::lowest(); }

template <typename T, typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
T maxv() { return T{1.0}; }

template <typename T, typename std::enable_if<!std::is_floating_point<T>::value, int>::type = 0>
T maxv() { return std::numeric_limits<T>::max(); }

template <typename To, typename From>
void testmax() {
    auto res = glm_convert_normalized<To>(maxv<From>());
    EXPECT_EQ(maxv<To>(), res);
}

template <typename To, typename From>
void testmin() {
    auto res = glm_convert_normalized<To>(minv<From>());
    EXPECT_EQ(minv<To>(), res);
}

#define CONV_TEST(Name, From, To)                             \
    TEST(ConversionTests, Min##Name) { testmin<To, From>(); } \
    TEST(ConversionTests, Max##Name) { testmax<To, From>(); }

/* Mathematica code to generate tests.

types = {"float", "double", "unsigned char", "unsigned short", "unsigned int",
    "unsigned long long",
   "signed char", "signed short", "signed int", "signed long long"};
pairs = Tuples[types, {2}];
StringJoin[
 Map["CONV_TEST(" <> StringReplace[#[[1]], " " -> "_"] <> "2" <> 
    StringReplace[#[[2]], " " -> "_"] <> ", " <> #[[1]] <> ", " <> #[[2]] <> 
    ")\n" &, pairs]]

*/

// Floating point to higer precision ints fails since:
// 1.0 * longlong::max() can't be represented exactly as a double...
// hence it will not be equal to longlong::max() when casted to a longlong...

CONV_TEST(float2float, float, float)
CONV_TEST(float2double, float, double)
CONV_TEST(float2unsigned_char, float, unsigned char)
CONV_TEST(float2unsigned_short, float, unsigned short)
//CONV_TEST(float2unsigned_int, float, unsigned int)
//CONV_TEST(float2unsigned_long_long, float, unsigned long long)
CONV_TEST(float2signed_char, float, signed char)
CONV_TEST(float2signed_short, float, signed short)
//CONV_TEST(float2signed_int, float, signed int)
//CONV_TEST(float2signed_long_long, float, signed long long)

CONV_TEST(double2float, double, float)
CONV_TEST(double2double, double, double)
CONV_TEST(double2unsigned_char, double, unsigned char)
CONV_TEST(double2unsigned_short, double, unsigned short)
CONV_TEST(double2unsigned_int, double, unsigned int)
//CONV_TEST(double2unsigned_long_long, double, unsigned long long)
CONV_TEST(double2signed_char, double, signed char)
CONV_TEST(double2signed_short, double, signed short)
CONV_TEST(double2signed_int, double, signed int)
//CONV_TEST(double2signed_long_long, double, signed long long)

CONV_TEST(unsigned_char2float, unsigned char, float)
CONV_TEST(unsigned_char2double, unsigned char, double)
CONV_TEST(unsigned_char2unsigned_char, unsigned char, unsigned char)
CONV_TEST(unsigned_char2unsigned_short, unsigned char, unsigned short)
CONV_TEST(unsigned_char2unsigned_int, unsigned char, unsigned int)
CONV_TEST(unsigned_char2unsigned_long_long, unsigned char, unsigned long long)
CONV_TEST(unsigned_char2signed_char, unsigned char, signed char)
CONV_TEST(unsigned_char2signed_short, unsigned char, signed short)
//CONV_TEST(unsigned_char2signed_int, unsigned char, signed int)
//CONV_TEST(unsigned_char2signed_long_long, unsigned char, signed long long)

CONV_TEST(unsigned_short2float, unsigned short, float)
CONV_TEST(unsigned_short2double, unsigned short, double)
CONV_TEST(unsigned_short2unsigned_char, unsigned short, unsigned char)
CONV_TEST(unsigned_short2unsigned_short, unsigned short, unsigned short)
CONV_TEST(unsigned_short2unsigned_int, unsigned short, unsigned int)
CONV_TEST(unsigned_short2unsigned_long_long, unsigned short, unsigned long long)
CONV_TEST(unsigned_short2signed_char, unsigned short, signed char)
CONV_TEST(unsigned_short2signed_short, unsigned short, signed short)
//CONV_TEST(unsigned_short2signed_int, unsigned short, signed int)
//CONV_TEST(unsigned_short2signed_long_long, unsigned short, signed long long)

CONV_TEST(unsigned_int2float, unsigned int, float)
CONV_TEST(unsigned_int2double, unsigned int, double)
CONV_TEST(unsigned_int2unsigned_char, unsigned int, unsigned char)
CONV_TEST(unsigned_int2unsigned_short, unsigned int, unsigned short)
CONV_TEST(unsigned_int2unsigned_int, unsigned int, unsigned int)
CONV_TEST(unsigned_int2unsigned_long_long, unsigned int, unsigned long long)
CONV_TEST(unsigned_int2signed_char, unsigned int, signed char)
CONV_TEST(unsigned_int2signed_short, unsigned int, signed short)
CONV_TEST(unsigned_int2signed_int, unsigned int, signed int)
//CONV_TEST(unsigned_int2signed_long_long, unsigned int, signed long long)

CONV_TEST(unsigned_long_long2float, unsigned long long, float)
CONV_TEST(unsigned_long_long2double, unsigned long long, double)
CONV_TEST(unsigned_long_long2unsigned_char, unsigned long long, unsigned char)
CONV_TEST(unsigned_long_long2unsigned_short, unsigned long long, unsigned short)
CONV_TEST(unsigned_long_long2unsigned_int, unsigned long long, unsigned int)
CONV_TEST(unsigned_long_long2unsigned_long_long, unsigned long long, unsigned long long)
CONV_TEST(unsigned_long_long2signed_char, unsigned long long, signed char)
CONV_TEST(unsigned_long_long2signed_short, unsigned long long, signed short)
CONV_TEST(unsigned_long_long2signed_int, unsigned long long, signed int)
CONV_TEST(unsigned_long_long2signed_long_long, unsigned long long, signed long long)

CONV_TEST(signed_char2float, signed char, float)
CONV_TEST(signed_char2double, signed char, double)
CONV_TEST(signed_char2unsigned_char, signed char, unsigned char)
CONV_TEST(signed_char2unsigned_short, signed char, unsigned short)
CONV_TEST(signed_char2unsigned_int, signed char, unsigned int)
CONV_TEST(signed_char2unsigned_long_long, signed char, unsigned long long)

CONV_TEST(signed_char2signed_char, signed char, signed char)
CONV_TEST(signed_char2signed_short, signed char, signed short)
CONV_TEST(signed_char2signed_int, signed char, signed int)
CONV_TEST(signed_char2signed_long_long, signed char, signed long long)

CONV_TEST(signed_short2float, signed short, float)
CONV_TEST(signed_short2double, signed short, double)
CONV_TEST(signed_short2unsigned_char, signed short, unsigned char)
CONV_TEST(signed_short2unsigned_short, signed short, unsigned short)
CONV_TEST(signed_short2unsigned_int, signed short, unsigned int)
CONV_TEST(signed_short2unsigned_long_long, signed short, unsigned long long)
CONV_TEST(signed_short2signed_char, signed short, signed char)
CONV_TEST(signed_short2signed_short, signed short, signed short)
CONV_TEST(signed_short2signed_int, signed short, signed int)
CONV_TEST(signed_short2signed_long_long, signed short, signed long long)

CONV_TEST(signed_int2float, signed int, float)
CONV_TEST(signed_int2double, signed int, double)
CONV_TEST(signed_int2unsigned_char, signed int, unsigned char)
CONV_TEST(signed_int2unsigned_short, signed int, unsigned short)
CONV_TEST(signed_int2unsigned_int, signed int, unsigned int)
CONV_TEST(signed_int2unsigned_long_long, signed int, unsigned long long)
CONV_TEST(signed_int2signed_char, signed int, signed char)
CONV_TEST(signed_int2signed_short, signed int, signed short)
CONV_TEST(signed_int2signed_int, signed int, signed int)
CONV_TEST(signed_int2signed_long_long, signed int, signed long long)

CONV_TEST(signed_long_long2float, signed long long, float)
CONV_TEST(signed_long_long2double, signed long long, double)
CONV_TEST(signed_long_long2unsigned_char, signed long long, unsigned char)
CONV_TEST(signed_long_long2unsigned_short, signed long long, unsigned short)
CONV_TEST(signed_long_long2unsigned_int, signed long long, unsigned int)
CONV_TEST(signed_long_long2unsigned_long_long, signed long long, unsigned long long)

//CONV_TEST(signed_long_long2signed_char, signed long long, signed char) can't grow long long
//CONV_TEST(signed_long_long2signed_short, signed long long, signed short)
//CONV_TEST(signed_long_long2signed_int, signed long long, signed int)

CONV_TEST(signed_long_long2signed_long_long, signed long long, signed long long)


}