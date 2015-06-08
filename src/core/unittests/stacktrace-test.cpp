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


#include <inviwo/core/util/stacktrace.h>

namespace inviwo{

class D {
public:
    void E(std::vector<std::string>& stacktrace) {
        stacktrace = getStackTrace();
    }
};

void C(std::vector<std::string>& stacktrace) {
    D d;
    d.E(stacktrace);
};

void B(std::vector<std::string>& stacktrace) {
    C(stacktrace);
}

void A(std::vector<std::string>& stacktrace) {
    B(stacktrace);
}

void Ap1(std::vector<std::string>& stacktrace) {
    A(stacktrace);
}
//
//TEST(StackTraceTests,StackTraceTest) {
//    std::vector<std::string> stacktrace1;
//    std::vector<std::string> stacktrace2;
//    A(stacktrace1);
//    Ap1(stacktrace2);
//    EXPECT_NE(0,stacktrace1.size());
//    EXPECT_NE(0,stacktrace2.size());
//    EXPECT_EQ(1,stacktrace2.size() - stacktrace1.size());
//}



}