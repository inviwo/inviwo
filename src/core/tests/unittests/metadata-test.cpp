/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2021 Inviwo Foundation
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
#include <inviwo/core/io/serialization/serializable.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/filesystem.h>

#include <string>

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

namespace inviwo {

template <typename T, typename M>
void testserialization(T def, T in) {
    T indata = in;
    T outdata1 = def;

    std::string filename = filesystem::findBasePath();
    MetaDataOwner mdo1;
    mdo1.setMetaData<M>("data", indata);
    EXPECT_TRUE(mdo1.hasMetaData<M>("data"));
    outdata1 = mdo1.getMetaData<M>("data", outdata1);
    EXPECT_EQ(indata, outdata1);

    Serializer serializer(filename);
    mdo1.getMetaDataMap()->serialize(serializer);
    std::stringstream ss;
    serializer.writeFile(ss);
    auto wm = InviwoApplication::getPtr()->getWorkspaceManager();
    auto deserializer = wm->createWorkspaceDeserializer(ss, filename);
    MetaDataOwner mdo2;
    mdo2.getMetaDataMap()->deserialize(deserializer);
    EXPECT_TRUE(mdo2.hasMetaData<M>("data"));

    T outdata2 = def;
    outdata2 = mdo2.getMetaData<M>("data", outdata2);
    EXPECT_EQ(indata, outdata2);
}

#define MetaDataMacro(n, t, d, v) \
    TEST(MetaDataTest, n##SerializationTest) { testserialization<t, n##MetaData>(d, v); }

MetaDataMacro(Bool, bool, false, true);
MetaDataMacro(Int, int, 0, 1);
MetaDataMacro(Float, float, 0.0f, 1.0f);
MetaDataMacro(Double, double, 0.0f, 1.0f);
MetaDataMacro(String, std::string, "", "test");
MetaDataMacro(IntVec2, ivec2, ivec2(0), ivec2(1, 2));
MetaDataMacro(IntVec3, ivec3, ivec3(0), ivec3(1, 2, 3));
MetaDataMacro(IntVec4, ivec4, ivec4(0), ivec4(1, 2, 3, 4));
MetaDataMacro(FloatVec2, vec2, vec2(0), vec2(0.0f, 1.0f));
MetaDataMacro(FloatVec3, vec3, vec3(0), vec3(0.0f, 1.0f, 2.0f));
MetaDataMacro(FloatVec4, vec4, vec4(0), vec4(0.0f, 1.0f, 2.0f, 3.0f));
MetaDataMacro(DoubleVec2, dvec2, dvec2(0), dvec2(0.0, 1.0));
MetaDataMacro(DoubleVec3, dvec3, dvec3(0), dvec3(0.0, 1.0, 2.0));
MetaDataMacro(DoubleVec4, dvec4, dvec4(0), dvec4(0.0, 1.0, 2.0, 3.0));
MetaDataMacro(UIntVec2, uvec2, uvec2(0), uvec2(1, 2));
MetaDataMacro(UIntVec3, uvec3, uvec3(0), uvec3(1, 2, 3));
MetaDataMacro(UIntVec4, uvec4, uvec4(0), uvec4(1, 2, 3, 4));
MetaDataMacro(FloatMat2, mat2, mat2(0), mat2(1.0));
MetaDataMacro(FloatMat3, mat3, mat3(0), mat3(1.0));
MetaDataMacro(FloatMat4, mat4, mat4(0), mat4(1.0));
#undef MetaDataMacro

}  // namespace inviwo
