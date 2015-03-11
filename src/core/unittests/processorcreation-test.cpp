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

#include <gtest/gtest.h>


#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/processors/processorfactory.h>
#include <modules/unittests/logerrorcounter.h>
#include "inviwo/core/datastructures/transferfunction.h"
#include "inviwo/core/properties/transferfunctionproperty.h"
#include <inviwo/core/common/inviwoapplication.h>

namespace inviwo{

class ProcessorCreationTests : public ::testing::TestWithParam<std::string> {
protected:
    ProcessorCreationTests() : p(0) {}

    virtual ~ProcessorCreationTests() {
        EXPECT_TRUE(p==0);
    }

    virtual void SetUp() {
        size_t warnCount = LogErrorCounter::getPtr()->getWarnCount();
        size_t errCount = LogErrorCounter::getPtr()->getErrorCount();

        IvwSerializable *s = ProcessorFactory::getPtr()->create(GetParam());
        ASSERT_TRUE(s != NULL);

        p = static_cast<Processor *>(s);
        ASSERT_TRUE(p != NULL);

        EXPECT_EQ(warnCount, LogErrorCounter::getPtr()->getWarnCount());
        EXPECT_EQ(errCount, LogErrorCounter::getPtr()->getErrorCount());

        isAdded_ = false;
    }

    virtual void TearDown() {
        if(isAdded_){
            ProcessorNetwork *pn = InviwoApplication::getPtr()->getProcessorNetwork();
            size_t sizeBefore = pn->getProcessors().size();

            pn->removeAndDeleteProcessor(p);

            size_t sizeAfter = pn->getProcessors().size();
            EXPECT_EQ(sizeBefore,sizeAfter+1);
            p = 0;
        }else{
            p->deinitialize();
            delete p;
            p = 0;
        }
    }

    void initialize() {
        size_t warnCount = LogErrorCounter::getPtr()->getWarnCount();
        size_t errCount = LogErrorCounter::getPtr()->getErrorCount();

        p->initialize();
        EXPECT_EQ(warnCount, LogErrorCounter::getPtr()->getWarnCount());
        EXPECT_EQ(errCount, LogErrorCounter::getPtr()->getErrorCount());
    }

    void resetAllPoperties() {
        size_t warnCount = LogErrorCounter::getPtr()->getWarnCount();
        size_t errCount = LogErrorCounter::getPtr()->getErrorCount();

        p->resetAllPoperties();
        EXPECT_EQ(warnCount, LogErrorCounter::getPtr()->getWarnCount());
        EXPECT_EQ(errCount, LogErrorCounter::getPtr()->getErrorCount());
    }

    void addProcessor() {
        size_t warnCount = LogErrorCounter::getPtr()->getWarnCount();
        size_t errCount = LogErrorCounter::getPtr()->getErrorCount();

        InviwoApplication::getPtr()->getProcessorNetwork()->addProcessor(p);
        EXPECT_EQ(warnCount, LogErrorCounter::getPtr()->getWarnCount());
        EXPECT_EQ(errCount, LogErrorCounter::getPtr()->getErrorCount());
        isAdded_ = true;
    }

    Processor *p;
    bool isAdded_;
};

std::vector<std::string> theVec;
const std::vector<std::string> &getListOfProcessors(){
    theVec.clear();
    std::vector<InviwoModule*>::const_iterator it = InviwoApplication::getPtr()->getModules().begin();
    std::vector<InviwoModule*>::const_iterator endIT = InviwoApplication::getPtr()->getModules().end();
    for(;it != endIT; ++it){
        std::vector<ProcessorFactoryObject*>::const_iterator processor = (*it)->getProcessors().begin();
        std::vector<ProcessorFactoryObject*>::const_iterator processorEnd = (*it)->getProcessors().end();
        for(;processor != processorEnd;++processor){
            theVec.push_back((*processor)->getClassIdentifier());
        }
    }
    return theVec;
}

//
//TEST_P(ProcessorCreationTests,ProcesorCreate){
//    initialize();
//}
//
//TEST_P(ProcessorCreationTests,ProcesorCreateAndReset){ 
//    initialize();
//    resetAllPoperties();
//}
//
//
//TEST_P(ProcessorCreationTests,ProcesorCreateAndAddToNetwork){ 
//    initialize();
//    addProcessor();
//}

// disabled the 3 test above since they are only needed when the following test fails 

TEST_P(ProcessorCreationTests,ProcesorCreateAndResetAndAddToNetwork){
    initialize();
    resetAllPoperties();
    addProcessor();
}

INSTANTIATE_TEST_CASE_P(RegisteredProcessors,ProcessorCreationTests,::testing::ValuesIn(getListOfProcessors()));

}
