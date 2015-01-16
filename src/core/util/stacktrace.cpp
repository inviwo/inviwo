/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

#include <inviwo/core/util/stacktrace.h>

#if defined(__unix__)
#include <execinfo.h>
#include <cxxabi.h>

#include <stdio.h>
#include <stdlib.h>
#elif defined(_MSC_VER)
#include <stackwalker/StackWalker.h>
#endif

#include <inviwo/core/util/stringconversion.h>



#if defined(_MSC_VER)
class StackWalkerToVector : public StackWalker {
public:
    StackWalkerToVector(StackWalkOptions level , std::vector<std::string>* vector):StackWalker(level),vector_(vector) {
    }
protected:
    virtual void OnOutput(LPCSTR szText) {
        std::string str(szText);

        if (str.find("StackWalker") != std::string::npos) {
            vector_->clear();
        } else if (str.find("ERROR: ") != std::string::npos) {
           // return;
        } else if (str.find("filename not available") != std::string::npos) {
           // return;
        }

        inviwo::replaceInString(str,"\n",""); //remove new line character at the end of the string that we get fro stack walker
        vector_->push_back(str);
    }

private:
    std::vector<std::string>* vector_;
};
#endif


namespace inviwo {

std::vector<std::string> getStackTrace() {
    std::vector<std::string> lines;
#if defined(__unix__)
    void* array[100];
    size_t size;
    char** strings;
    size = backtrace(array, 100);
    strings = backtrace_symbols(array, size);

    for (size_t i = 0; i < size; i++) {
        int status;
        std::string line = strings[i];
        size_t start = line.find("(");
        size_t end    = line.find("+");
        std::string className = line.substr(start+1,end - start - 1);
        char* fixedClass = abi::__cxa_demangle(className.c_str(),0,0,&status);

        if (!status && fixedClass) {
            replaceInString(line,className,std::string(fixedClass));
            free(fixedClass);
        }

        lines.push_back(line);
        //    free(line);
    }

    free(strings);
#elif defined(_MSC_VER)
    StackWalkerToVector sw(StackWalker::OptionsAll,&lines);
    sw.LoadModules();
    sw.ShowCallstack();
#endif
    return lines;
}


} // namespace