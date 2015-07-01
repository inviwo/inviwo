/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

/*
The MIT License (MIT)

Copyright (c) 2013 Brandon Amos <http://bamos.github.io>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

// Source:
// http://www.daniweb.com/software-development/cpp/code/427500/calculator-using-shunting-yard-algorithm#
// Author: Jesse Brown
// Modifications: Brandon Amos

#ifndef IVW_SHUNTINGYARD_H
#define IVW_SHUNTINGYARD_H

#include <inviwo/core/common/inviwocoredefine.h>

#include <map>
#include <stack>
#include <string>
#include <iostream>
#include <sstream>
#include <queue>

namespace inviwo {
namespace shuntingyard {

struct IVW_CORE_API TokenBase {
    virtual ~TokenBase() {}
};

template <class T>
class Token : public TokenBase {
public:
    Token(T t) : val(t) {}
    T val;
};

using TokenQueue = std::queue<std::unique_ptr<TokenBase>>;

class IVW_CORE_API Calculator {
public:
    static double calculate(std::string expression, std::map<std::string, double>& vars);
    static std::string shaderCode(std::string expression, std::map<std::string, double>& vars,
                                  std::map<std::string, std::string>& symbols);

private:
    inline static bool isvariablechar(char c) { return isalpha(c) || c == '_'; }

    inline static std::string getVariable(std::stringstream& expr) {
        std::stringstream ss;
        char c;
        expr.get(c);
        ss << c;
        while (isvariablechar(expr.peek()) || isdigit(expr.peek())) {
            expr.get(c);
            ss << c;
        }
        std::string key = ss.str();
        return key;
    }

    static TokenQueue toRPN(std::string expression, std::map<std::string, int> opPrecedence);

    static std::map<std::string, int> getOpeatorPrecedence() {
        std::map<std::string, int> opPrecedence;
        opPrecedence["("] = -1;
        opPrecedence["+"] = 2;
        opPrecedence["-"] = 2;
        opPrecedence["*"] = 3;
        opPrecedence["/"] = 3;
        opPrecedence["^"] = 4;

        return opPrecedence;
    }
};

}  // namespace
}  // namespace

#endif  // IVW_SHUNTINGYARD_H
