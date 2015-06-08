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

#include <inviwo/core/metadata/metadataowner.h>
#include <inviwo/core/metadata/metadata.h>
#include <inviwo/core/io/serialization/ivwserializable.h>
#include <string.h>
#include <inviwo/core/common/inviwoapplication.h>


#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

namespace inviwo{


template<typename T, typename M>
void testserialization(T def, T in) {
    T indata = in;
    T outdata1 = def;
    std::string filename =
        InviwoApplication::getPtr()->getPath(InviwoApplication::PATH_MODULES,
                "unittests/tmpfiles/metadatatests.xml");
    MetaDataOwner* mdo1;
    MetaDataOwner* mdo2;
    mdo1 = new MetaDataOwner();
    mdo2 = new MetaDataOwner();
    mdo1->setMetaData<M>("data", indata);
    outdata1 = mdo1->getMetaData<M>("data", outdata1);
    EXPECT_EQ(indata, outdata1);
    IvwSerializer serializer(filename);
    mdo1->getMetaDataMap()->serialize(serializer);
    serializer.writeFile();
    IvwDeserializer deserializer(filename);
    mdo2->getMetaDataMap()->deserialize(deserializer);
    T outdata2 = def;
    outdata2 = mdo2->getMetaData<M>("data", outdata2);
    EXPECT_EQ(indata, outdata2);
    delete mdo1;
    delete mdo2;
}


#define MetaDataMacro(n, t, d, v) \
    TEST(MetaDataTest, n##SerializationTest) { \
        testserialization<t, n##MetaData>(d, v); \
    }
#include <inviwo/core/metadata/metadatadefinefunc.h>

}