/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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

#ifndef IVW_PYTHONREGTESTPROCESSOR_H
#define IVW_PYTHONREGTESTPROCESSOR_H

#include <modules/python3/python3moduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/ports/imageport.h>
#include <modules/python3/pythonscript.h>

namespace inviwo {

class IVW_MODULE_PYTHON3_API PythonRegTestProcessor : public Processor { 
public:
    PythonRegTestProcessor();
    virtual ~PythonRegTestProcessor() = default;
     
    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
private:
    ImageOutport outport_;
    

    std::string basePath_;
    PythonScriptDisk scriptGrabReturnValue_;
    PythonScriptDisk scriptPassValues_;
    PythonScriptDisk scriptSimpleBufferTest_;
    PythonScriptDisk scriptBufferTypes_;
    PythonScriptDisk scriptLayerTypes_;
    PythonScriptDisk scriptVolumeTypes_;



    DataVec4UInt8::type* data_;
    size_t testID_;

    void updateImage(bool passed) {
        static const DataVec4UInt8::type PASS(0, 128, 0, 255);
        static const DataVec4UInt8::type FAIL(200, 0, 0, 255);

        data_[testID_++] = passed ? PASS : FAIL;
    }

    template <typename T, typename Callback>
    void expectEqCmp(T expected, T actual, Callback comp, std::string testname) {

        bool passed = comp(expected, actual);
        if (!passed) {
            LogError("Test " << testname << " failed, got '" << actual << "', expected '"
                << expected << "'");
        }
        else {
            LogInfo("Test " << testname << " passed");
        }

        updateImage(passed);
        
    }

    template<typename T>
    void expectEQ(T expected, T actual, std::string testname = "") {
        static const auto testEQ = [](auto a, auto b) -> bool {return a == b; };
        return expectEqCmp(expected,actual,testEQ,testname);
    }


    template<typename T>
    void expectEQFloat(T expected, T actual, std::string testname = "") {
        static const auto testEQ = [](auto a, auto b) -> bool {
            return std::abs(a - b) < std::numeric_limits<decltype(a)>::epsilon();
        };
        return expectEqCmp(expected, actual, testEQ, testname);
    }

};

} // namespace

#endif // IVW_PYTHONREGTESTPROCESSOR_H

