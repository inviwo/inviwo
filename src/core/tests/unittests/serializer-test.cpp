/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2018 Inviwo Foundation
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
#include <inviwo/core/io/serialization/serializable.h>
#include <inviwo/core/util/filesystem.h>

namespace inviwo {

TEST(SerializationTest, initTest) {
    std::string refpath = filesystem::findBasePath();
    std::stringstream ss;
    Serializer serializer(refpath);
    serializer.writeFile(ss);
    EXPECT_TRUE(true);
}

template <typename T>
T serializationOfType(T inValue) {
    std::string refpath = filesystem::findBasePath();
    std::stringstream ss;
    Serializer serializer(refpath);
    serializer.serialize("serializedValue", inValue);
    serializer.writeFile(ss);
    Deserializer deserializer(ss, refpath);
    T outValue;
    deserializer.deserialize("serializedValue", outValue);
    return outValue;
}

#define TYPE_TEST(n, T, v)                                                                   \
    TEST(SerializationTest, n##TypeTest) {                                                   \
        EXPECT_NEAR((T)(v), serializationOfType((T)(v)), std::numeric_limits<T>::epsilon()); \
    }
#define MIN_TEST(n, T)                                                  \
    TEST(SerializationTest, n##MinTest) {                               \
        EXPECT_NEAR(std::numeric_limits<T>::min(),                      \
                    serializationOfType(std::numeric_limits<T>::min()), \
                    std::numeric_limits<T>::epsilon());                 \
    }
#define MAX_TEST(n, T)                                                  \
    TEST(SerializationTest, n##MaxTest) {                               \
        EXPECT_NEAR(std::numeric_limits<T>::max(),                      \
                    serializationOfType(std::numeric_limits<T>::max()), \
                    std::numeric_limits<T>::epsilon());                 \
    }
#define EPSILON_TEST(n, T)                                                  \
    TEST(SerializationTest, n##EpsilonTest) {                               \
        EXPECT_NEAR(std::numeric_limits<T>::epsilon(),                      \
                    serializationOfType(std::numeric_limits<T>::epsilon()), \
                    std::numeric_limits<T>::epsilon());                     \
    }

#define NUMERIC_TESTS(n, T, v) TYPE_TEST(n, T, v) MIN_TEST(n, T) MAX_TEST(n, T) EPSILON_TEST(n, T)

static const float oneMinusEpsilonF = 1.0f - std::numeric_limits<float>::epsilon();
static const double oneMinusEpsilonD = 1.0 - std::numeric_limits<double>::epsilon();

NUMERIC_TESTS(floatSerialization, float, 3.14f)
NUMERIC_TESTS(doubleSerializationTest, double, 6.28)
TYPE_TEST(oneMinusEpsilonFloatTest, float, oneMinusEpsilonF)
TYPE_TEST(oneMinusEpsilonDobuleTest, double, oneMinusEpsilonD)

#undef TYPE_TEST
#undef MIN_TEST
#undef MAX_TEST
#undef EPSILON_TEST

#define TYPE_TEST(n, T, v) \
    TEST(SerializationTest, n##TypeTest) { EXPECT_EQ((T)(v), serializationOfType((T)(v))); }
#define MIN_TEST(n, T)                                                 \
    TEST(SerializationTest, n##MinTest) {                              \
        EXPECT_EQ(std::numeric_limits<T>::min(),                       \
                  serializationOfType(std::numeric_limits<T>::min())); \
    }
#define MAX_TEST(n, T)                                                 \
    TEST(SerializationTest, n##MaxTest) {                              \
        EXPECT_EQ(std::numeric_limits<T>::max(),                       \
                  serializationOfType(std::numeric_limits<T>::max())); \
    }
#define EPSILON_TEST(n, T)                                                 \
    TEST(SerializationTest, n##EpsilonTest) {                              \
        EXPECT_EQ(std::numeric_limits<T>::epsilon(),                       \
                  serializationOfType(std::numeric_limits<T>::epsilon())); \
    }

NUMERIC_TESTS(signedCharSerializationTest, signed char, 3)
NUMERIC_TESTS(charSerializationTest, char, 3)
NUMERIC_TESTS(unsignedCharSerializationTest, unsigned char, 3)

TYPE_TEST(letterCharSerializationTest1, char, 'b')
TYPE_TEST(letterCharSerializationTest2, char, 't')

NUMERIC_TESTS(shortSerializationTest, short, -1065)
NUMERIC_TESTS(unsignedShortSerializationTest, unsigned short, 1065)

NUMERIC_TESTS(intSerializationTest, int, -65124)
NUMERIC_TESTS(unsignedIntSerializationTest, unsigned int, 45654)

NUMERIC_TESTS(longSerializationTest, long, 650004)
NUMERIC_TESTS(longLongSerializationTest, long long, 6700089)
NUMERIC_TESTS(unsignedLongLongSerializationTest, unsigned long long, 99996789)

class MinimumSerilizableClass : public Serializable {
public:
    MinimumSerilizableClass(float v = 0) : value_(v) {}

    virtual void serialize(Serializer& s) const { s.serialize("classVariable", value_); }

    virtual void deserialize(Deserializer& d) { d.deserialize("classVariable", value_); }

    bool operator==(const MinimumSerilizableClass& v) const { return value_ == v.value_; }

public:
    float value_;
};

TEST(SerializationTest, IvwSerializableClassTest) {
    MinimumSerilizableClass inValue(12), outValue;
    std::string refpath = filesystem::findBasePath();
    std::stringstream ss;
    Serializer serializer(refpath);
    serializer.serialize("serializedValue", inValue);
    serializer.writeFile(ss);
    Deserializer deserializer(ss, refpath);
    deserializer.deserialize("serializedValue", outValue);
    EXPECT_EQ(inValue.value_, 12);
    EXPECT_NE(outValue.value_, 0);
    EXPECT_EQ(inValue, outValue);
}

TEST(SerializationTest, IvwSerializableClassAsPointerTest) {
    MinimumSerilizableClass *inValue = new MinimumSerilizableClass(12), *outValue = 0;
    std::string refpath = filesystem::findBasePath();
    std::stringstream ss;
    Serializer serializer(refpath);
    serializer.serialize("serializedValue", inValue);
    serializer.writeFile(ss);
    Deserializer deserializer(ss, refpath);
    deserializer.deserialize("serializedValue", outValue);
    EXPECT_EQ(inValue->value_, 12);
    EXPECT_NE(outValue->value_, 0);
    EXPECT_EQ(inValue->value_, outValue->value_);
    delete inValue;
    delete outValue;
}

TEST(SerializationTest, floatVectorTest) {
    std::vector<float> inVector, outVector;
    inVector.push_back(0.1f);
    inVector.push_back(0.2f);
    inVector.push_back(0.3f);
    std::string refpath = filesystem::findBasePath();
    std::stringstream ss;
    Serializer serializer(refpath);
    serializer.serialize("serializedVector", inVector, "value");
    serializer.writeFile(ss);
    Deserializer deserializer(ss, refpath);
    deserializer.deserialize("serializedVector", outVector, "value");
    ASSERT_EQ(inVector.size(), outVector.size());

    for (size_t i = 0; i < inVector.size(); i++) EXPECT_EQ(inVector[i], outVector[i]);
}

TEST(SerializationTest, vectorOfNonPointersTest) {
    std::vector<MinimumSerilizableClass> inVector, outVector;
    inVector.push_back(MinimumSerilizableClass(0.1f));
    inVector.push_back(MinimumSerilizableClass(0.2f));
    inVector.push_back(MinimumSerilizableClass(0.3f));
    std::string refpath = filesystem::findBasePath();
    std::stringstream ss;
    Serializer serializer(refpath);
    serializer.serialize("serializedVector", inVector, "value");
    serializer.writeFile(ss);
    Deserializer deserializer(ss, refpath);
    deserializer.deserialize("serializedVector", outVector, "value");
    ASSERT_EQ(inVector.size(), outVector.size());

    for (size_t i = 0; i < inVector.size(); i++) EXPECT_EQ(inVector[i], outVector[i]);
}

TEST(SerializationTest, vectorOfPointersTest) {
    std::vector<MinimumSerilizableClass*> inVector, outVector;
    inVector.push_back(new MinimumSerilizableClass(0.1f));
    inVector.push_back(new MinimumSerilizableClass(0.2f));
    inVector.push_back(new MinimumSerilizableClass(0.3f));
    std::string refpath = filesystem::findBasePath();
    std::stringstream ss;
    Serializer serializer(refpath);
    serializer.serialize("serializedVector", inVector, "value");
    serializer.writeFile(ss);
    Deserializer deserializer(ss, refpath);
    deserializer.deserialize("serializedVector", outVector, "value");
    ASSERT_EQ(inVector.size(), outVector.size());

    for (size_t i = 0; i < inVector.size(); i++)
        EXPECT_EQ(inVector[i]->value_, outVector[i]->value_);

    delete inVector[0];
    delete inVector[1];
    delete inVector[2];
    delete outVector[0];
    delete outVector[1];
    delete outVector[2];
}

TEST(SerializationTest, vec2Tests) {
    vec2 inVec(1.1f, 2.2f), outVec;
    outVec = serializationOfType(inVec);
    EXPECT_EQ(inVec.x, outVec.x);
    EXPECT_EQ(inVec.y, outVec.y);
}

TEST(SerializationTest, vec3Tests) {
    vec3 inVec(1.1f, 2.2f, 3.3f), outVec;
    outVec = serializationOfType(inVec);
    EXPECT_EQ(inVec.x, outVec.x);
    EXPECT_EQ(inVec.y, outVec.y);
    EXPECT_EQ(inVec.z, outVec.z);
}

TEST(SerializationTest, vec4Tests) {
    vec4 inVec(1.1f, 2.2f, 3.3f, 4.4f), outVec;
    outVec = serializationOfType(inVec);
    EXPECT_EQ(inVec.x, outVec.x);
    EXPECT_EQ(inVec.y, outVec.y);
    EXPECT_EQ(inVec.z, outVec.z);
    EXPECT_EQ(inVec.w, outVec.w);
}

TEST(SerializationTest, ivec2Tests) {
    ivec2 inVec(1, 2), outVec;
    outVec = serializationOfType(inVec);
    EXPECT_EQ(inVec.x, outVec.x);
    EXPECT_EQ(inVec.y, outVec.y);
}

TEST(SerializationTest, ivec3Tests) {
    ivec3 inVec(1, 2, 3), outVec;
    outVec = serializationOfType(inVec);
    EXPECT_EQ(inVec.x, outVec.x);
    EXPECT_EQ(inVec.y, outVec.y);
    EXPECT_EQ(inVec.z, outVec.z);
}

TEST(SerializationTest, ivec4Tests) {
    ivec4 inVec(1, 2, 3, 4), outVec;
    outVec = serializationOfType(inVec);
    EXPECT_EQ(inVec.x, outVec.x);
    EXPECT_EQ(inVec.y, outVec.y);
    EXPECT_EQ(inVec.z, outVec.z);
    EXPECT_EQ(inVec.w, outVec.w);
}

TEST(SerializationTest, uvec2Tests) {
    uvec2 inVec(1, 2), outVec;
    outVec = serializationOfType(inVec);
    EXPECT_EQ(inVec.x, outVec.x);
    EXPECT_EQ(inVec.y, outVec.y);
}

TEST(SerializationTest, uvec3Tests) {
    uvec3 inVec(1, 2, 3), outVec;
    outVec = serializationOfType(inVec);
    EXPECT_EQ(inVec.x, outVec.x);
    EXPECT_EQ(inVec.y, outVec.y);
    EXPECT_EQ(inVec.z, outVec.z);
}

TEST(SerializationTest, uvec4Tests) {
    uvec4 inVec(1, 2, 3, 4), outVec;
    outVec = serializationOfType(inVec);
    EXPECT_EQ(inVec.x, outVec.x);
    EXPECT_EQ(inVec.y, outVec.y);
    EXPECT_EQ(inVec.z, outVec.z);
    EXPECT_EQ(inVec.w, outVec.w);
}

TEST(SerializationTest, bvec2Tests) {
    bvec2 inVec(false, true), outVec;
    outVec = serializationOfType(inVec);
    EXPECT_EQ(inVec.x, outVec.x);
    EXPECT_EQ(inVec.y, outVec.y);
}

TEST(SerializationTest, bvec3Tests) {
    bvec3 inVec(false, true, false), outVec;
    outVec = serializationOfType(inVec);
    EXPECT_EQ(inVec.x, outVec.x);
    EXPECT_EQ(inVec.y, outVec.y);
    EXPECT_EQ(inVec.z, outVec.z);
}

TEST(SerializationTest, bvec4Tests) {
    bvec4 inVec(false, true, false, true), outVec;
    outVec = serializationOfType(inVec);
    EXPECT_EQ(inVec.x, outVec.x);
    EXPECT_EQ(inVec.y, outVec.y);
    EXPECT_EQ(inVec.z, outVec.z);
    EXPECT_EQ(inVec.w, outVec.w);
}

TEST(SerializationTest, dvec2Tests) {
    dvec2 inVec(1.1, 2.2), outVec;
    outVec = serializationOfType(inVec);
    EXPECT_EQ(inVec.x, outVec.x);
    EXPECT_EQ(inVec.y, outVec.y);
}

TEST(SerializationTest, dvec3Tests) {
    dvec3 inVec(1.1, 2.2, 3.3), outVec;
    outVec = serializationOfType(inVec);
    EXPECT_EQ(inVec.x, outVec.x);
    EXPECT_EQ(inVec.y, outVec.y);
    EXPECT_EQ(inVec.z, outVec.z);
}

TEST(SerializationTest, dvec4Tests) {
    dvec4 inVec(1.1, 2.2, 3.3, 4.4), outVec;
    outVec = serializationOfType(inVec);
    EXPECT_EQ(inVec.x, outVec.x);
    EXPECT_EQ(inVec.y, outVec.y);
    EXPECT_EQ(inVec.z, outVec.z);
    EXPECT_EQ(inVec.w, outVec.w);
}

TEST(SerializationTest, mat2Tests) {
    mat2 inMat(0.0f, 1.0f, 2.0f, 3.0f), outMat;
    outMat = serializationOfType(inMat);
    int s = 2;

    for (int i = 0; i < s; i++)
        for (int j = 0; j < s; j++) EXPECT_EQ(inMat[i][j], outMat[i][j]);
}

TEST(SerializationTest, mat3Tests) {
    mat3 inMat(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f), outMat;
    outMat = serializationOfType(inMat);
    int s = 3;

    for (int i = 0; i < s; i++)
        for (int j = 0; j < s; j++) EXPECT_EQ(inMat[i][j], outMat[i][j]);
}

TEST(SerializationTest, mat4Tests) {
    mat4 inMat(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f,
               13.0f, 14.0f, 15.0f),
        outMat;
    outMat = serializationOfType(inMat);
    int s = 4;

    for (int i = 0; i < s; i++)
        for (int j = 0; j < s; j++) EXPECT_EQ(inMat[i][j], outMat[i][j]);
}
}  // namespace inviwo