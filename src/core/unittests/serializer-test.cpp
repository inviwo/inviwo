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

#include <modules/unittests/unittestsmoduledefine.h>
#include <inviwo/core/io/serialization/ivwserializable.h>
#include <inviwo/core/common/inviwoapplication.h>

namespace inviwo {

#define SERIALITION_FILE_NAME                                             \
    InviwoApplication::getPtr()->getPath(InviwoApplication::PATH_MODULES, \
                                         "unittests/tmpfiles/serilizationtests.xml")

TEST(SerialitionTest, initTest) {
    std::string filename = SERIALITION_FILE_NAME;
    IvwSerializer serializer(filename);
    EXPECT_TRUE(true);
}

TEST(SerialitionTest, writeFile) {
    std::string filename = SERIALITION_FILE_NAME;
    IvwSerializer serializer(filename);
    serializer.serialize("test", 3.1415f);
    serializer.writeFile();
    EXPECT_TRUE(filesystem::fileExists(filename));
}

template <typename T>
T serializationOfType(T inValue) {
    std::string filename = SERIALITION_FILE_NAME;
    IvwSerializer serializer(filename);
    serializer.serialize("serializedValue", inValue);
    serializer.writeFile();
    IvwDeserializer deserializer(filename);
    T outValue;
    deserializer.deserialize("serializedValue", outValue);
    return outValue;
}

#define TYPE_TEST(n, T, v)                                                                   \
    TEST(SerialitionTest, n##TypeTest) {                                                     \
        EXPECT_NEAR((T)(v), serializationOfType((T)(v)), std::numeric_limits<T>::epsilon()); \
    }
#define MIN_TEST(n, T)                                                  \
    TEST(SerialitionTest, n##MinTest) {                                 \
        EXPECT_NEAR(std::numeric_limits<T>::min(),                      \
                    serializationOfType(std::numeric_limits<T>::min()), \
                    std::numeric_limits<T>::epsilon());                 \
    }
#define MAX_TEST(n, T)                                                  \
    TEST(SerialitionTest, n##MaxTest) {                                 \
        EXPECT_NEAR(std::numeric_limits<T>::max(),                      \
                    serializationOfType(std::numeric_limits<T>::max()), \
                    std::numeric_limits<T>::epsilon());                 \
    }
#define EPSILON_TEST(n, T)                                                  \
    TEST(SerialitionTest, n##EpsilonTest) {                                 \
        EXPECT_NEAR(std::numeric_limits<T>::epsilon(),                      \
                    serializationOfType(std::numeric_limits<T>::epsilon()), \
                    std::numeric_limits<T>::epsilon());                     \
    }

#define NUMERIC_TESTS(n, T, v) TYPE_TEST(n, T, v) MIN_TEST(n, T) MAX_TEST(n, T) EPSILON_TEST(n, T)

static const float oneMinusEpsilonF = 1.0f - std::numeric_limits<float>::epsilon();
static const double oneMinusEpsilonD = 1.0 - std::numeric_limits<double>::epsilon();

NUMERIC_TESTS(floatSerialization, float, 3.14f);
NUMERIC_TESTS(doubleSerializationTest, double, 6.28);
TYPE_TEST(oneMinusEpsilonFloatTest, float, oneMinusEpsilonF);
TYPE_TEST(oneMinusEpsilonDobuleTest, double, oneMinusEpsilonD);

#undef TYPE_TEST
#undef MIN_TEST
#undef MAX_TEST
#undef EPSILON_TEST

#define TYPE_TEST(n, T, v) \
    TEST(SerialitionTest, n##TypeTest) { EXPECT_EQ((T)(v), serializationOfType((T)(v))); }
#define MIN_TEST(n, T)                                                 \
    TEST(SerialitionTest, n##MinTest) {                                \
        EXPECT_EQ(std::numeric_limits<T>::min(),                       \
                  serializationOfType(std::numeric_limits<T>::min())); \
    }
#define MAX_TEST(n, T)                                                 \
    TEST(SerialitionTest, n##MaxTest) {                                \
        EXPECT_EQ(std::numeric_limits<T>::max(),                       \
                  serializationOfType(std::numeric_limits<T>::max())); \
    }
#define EPSILON_TEST(n, T)                                                 \
    TEST(SerialitionTest, n##EpsilonTest) {                                \
        EXPECT_EQ(std::numeric_limits<T>::epsilon(),                       \
                  serializationOfType(std::numeric_limits<T>::epsilon())); \
    }

NUMERIC_TESTS(signedCharSerializationTest, signed char, 3);
NUMERIC_TESTS(charSerializationTest, char, 3);
NUMERIC_TESTS(unsignedCharSerializationTest, unsigned char, 3);

TYPE_TEST(letterCharSerializationTest1, char, 'b');
TYPE_TEST(letterCharSerializationTest2, char, 't');

NUMERIC_TESTS(shortSerializationTest, short, -1065);
NUMERIC_TESTS(unsignedShortSerializationTest, unsigned short, 1065);

NUMERIC_TESTS(intSerializationTest, int, -65124);
NUMERIC_TESTS(unsignedIntSerializationTest, unsigned int, 45654);

NUMERIC_TESTS(longSerializationTest, long, 650004);
NUMERIC_TESTS(longLongSerializationTest, long long, 6700089);
NUMERIC_TESTS(unsignedLongLongSerializationTest, unsigned long long, 99996789);

class IVW_MODULE_UNITTESTS_API MinimumSerilizableClass : public IvwSerializable {
public:
    MinimumSerilizableClass(float v = 0) : value_(v) {}

    virtual void serialize(IvwSerializer& s) const { s.serialize("classVariable", value_); }

    virtual void deserialize(IvwDeserializer& d) { d.deserialize("classVariable", value_); }

    bool operator==(const MinimumSerilizableClass& v) const { return value_ == v.value_; }

public:
    float value_;
};

TEST(SerialitionTest, IvwSerializableClassTest) {
    MinimumSerilizableClass inValue(12), outValue;
    std::string filename = SERIALITION_FILE_NAME;
    IvwSerializer serializer(filename);
    serializer.serialize("serializedValue", inValue);
    serializer.writeFile();
    IvwDeserializer deserializer(filename);
    deserializer.deserialize("serializedValue", outValue);
    EXPECT_EQ(inValue.value_, 12);
    EXPECT_NE(outValue.value_, 0);
    EXPECT_EQ(inValue, outValue);
}

TEST(SerialitionTest, IvwSerializableClassAsPointerTest) {
    MinimumSerilizableClass* inValue = new MinimumSerilizableClass(12), * outValue = 0;
    std::string filename = SERIALITION_FILE_NAME;
    IvwSerializer serializer(filename);
    serializer.serialize("serializedValue", inValue);
    serializer.writeFile();
    IvwDeserializer deserializer(filename);
    deserializer.deserialize("serializedValue", outValue);
    EXPECT_EQ(inValue->value_, 12);
    EXPECT_NE(outValue->value_, 0);
    EXPECT_EQ(inValue->value_, outValue->value_);
    delete inValue;
    delete outValue;
}

TEST(SerialitionTest, floatVectorTest) {
    std::vector<float> inVector, outVector;
    inVector.push_back(0.1f);
    inVector.push_back(0.2f);
    inVector.push_back(0.3f);
    std::string filename = SERIALITION_FILE_NAME;
    IvwSerializer serializer(filename);
    serializer.serialize("serializedVector", inVector, "value");
    serializer.writeFile();
    IvwDeserializer deserializer(filename);
    deserializer.deserialize("serializedVector", outVector, "value");
    ASSERT_EQ(inVector.size(), outVector.size());

    for (size_t i = 0; i < inVector.size(); i++) EXPECT_EQ(inVector[i], outVector[i]);
}

TEST(SerialitionTest, vectorOfNonPointersTest) {
    std::vector<MinimumSerilizableClass> inVector, outVector;
    inVector.push_back(MinimumSerilizableClass(0.1f));
    inVector.push_back(MinimumSerilizableClass(0.2f));
    inVector.push_back(MinimumSerilizableClass(0.3f));
    std::string filename = SERIALITION_FILE_NAME;
    IvwSerializer serializer(filename);
    serializer.serialize("serializedVector", inVector, "value");
    serializer.writeFile();
    IvwDeserializer deserializer(filename);
    deserializer.deserialize("serializedVector", outVector, "value");
    ASSERT_EQ(inVector.size(), outVector.size());

    for (size_t i = 0; i < inVector.size(); i++) EXPECT_EQ(inVector[i], outVector[i]);
}

TEST(SerialitionTest, vectorOfPointersTest) {
    std::vector<MinimumSerilizableClass*> inVector, outVector;
    inVector.push_back(new MinimumSerilizableClass(0.1f));
    inVector.push_back(new MinimumSerilizableClass(0.2f));
    inVector.push_back(new MinimumSerilizableClass(0.3f));
    std::string filename = SERIALITION_FILE_NAME;
    IvwSerializer serializer(filename);
    serializer.serialize("serializedVector", inVector, "value");
    serializer.writeFile();
    IvwDeserializer deserializer(filename);
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

TEST(SerialitionTest, vec2Tests) {
    vec2 inVec(1.1f, 2.2f), outVec;
    outVec = serializationOfType(inVec);
    EXPECT_EQ(inVec.x, outVec.x);
    EXPECT_EQ(inVec.y, outVec.y);
}

TEST(SerialitionTest, vec3Tests) {
    vec3 inVec(1.1f, 2.2f, 3.3f), outVec;
    outVec = serializationOfType(inVec);
    EXPECT_EQ(inVec.x, outVec.x);
    EXPECT_EQ(inVec.y, outVec.y);
    EXPECT_EQ(inVec.z, outVec.z);
}

TEST(SerialitionTest, vec4Tests) {
    vec4 inVec(1.1f, 2.2f, 3.3f, 4.4f), outVec;
    outVec = serializationOfType(inVec);
    EXPECT_EQ(inVec.x, outVec.x);
    EXPECT_EQ(inVec.y, outVec.y);
    EXPECT_EQ(inVec.z, outVec.z);
    EXPECT_EQ(inVec.w, outVec.w);
}

TEST(SerialitionTest, ivec2Tests) {
    ivec2 inVec(1, 2), outVec;
    outVec = serializationOfType(inVec);
    EXPECT_EQ(inVec.x, outVec.x);
    EXPECT_EQ(inVec.y, outVec.y);
}

TEST(SerialitionTest, ivec3Tests) {
    ivec3 inVec(1, 2, 3), outVec;
    outVec = serializationOfType(inVec);
    EXPECT_EQ(inVec.x, outVec.x);
    EXPECT_EQ(inVec.y, outVec.y);
    EXPECT_EQ(inVec.z, outVec.z);
}

TEST(SerialitionTest, ivec4Tests) {
    ivec4 inVec(1, 2, 3, 4), outVec;
    outVec = serializationOfType(inVec);
    EXPECT_EQ(inVec.x, outVec.x);
    EXPECT_EQ(inVec.y, outVec.y);
    EXPECT_EQ(inVec.z, outVec.z);
    EXPECT_EQ(inVec.w, outVec.w);
}

TEST(SerialitionTest, uvec2Tests) {
    uvec2 inVec(1, 2), outVec;
    outVec = serializationOfType(inVec);
    EXPECT_EQ(inVec.x, outVec.x);
    EXPECT_EQ(inVec.y, outVec.y);
}

TEST(SerialitionTest, uvec3Tests) {
    uvec3 inVec(1, 2, 3), outVec;
    outVec = serializationOfType(inVec);
    EXPECT_EQ(inVec.x, outVec.x);
    EXPECT_EQ(inVec.y, outVec.y);
    EXPECT_EQ(inVec.z, outVec.z);
}

TEST(SerialitionTest, uvec4Tests) {
    uvec4 inVec(1, 2, 3, 4), outVec;
    outVec = serializationOfType(inVec);
    EXPECT_EQ(inVec.x, outVec.x);
    EXPECT_EQ(inVec.y, outVec.y);
    EXPECT_EQ(inVec.z, outVec.z);
    EXPECT_EQ(inVec.w, outVec.w);
}

TEST(SerialitionTest, bvec2Tests) {
    bvec2 inVec(false, true), outVec;
    outVec = serializationOfType(inVec);
    EXPECT_EQ(inVec.x, outVec.x);
    EXPECT_EQ(inVec.y, outVec.y);
}

TEST(SerialitionTest, bvec3Tests) {
    bvec3 inVec(false, true, false), outVec;
    outVec = serializationOfType(inVec);
    EXPECT_EQ(inVec.x, outVec.x);
    EXPECT_EQ(inVec.y, outVec.y);
    EXPECT_EQ(inVec.z, outVec.z);
}

TEST(SerialitionTest, bvec4Tests) {
    bvec4 inVec(false, true, false, true), outVec;
    outVec = serializationOfType(inVec);
    EXPECT_EQ(inVec.x, outVec.x);
    EXPECT_EQ(inVec.y, outVec.y);
    EXPECT_EQ(inVec.z, outVec.z);
    EXPECT_EQ(inVec.w, outVec.w);
}

TEST(SerialitionTest, dvec2Tests) {
    dvec2 inVec(1.1, 2.2), outVec;
    outVec = serializationOfType(inVec);
    EXPECT_EQ(inVec.x, outVec.x);
    EXPECT_EQ(inVec.y, outVec.y);
}

TEST(SerialitionTest, dvec3Tests) {
    dvec3 inVec(1.1, 2.2, 3.3), outVec;
    outVec = serializationOfType(inVec);
    EXPECT_EQ(inVec.x, outVec.x);
    EXPECT_EQ(inVec.y, outVec.y);
    EXPECT_EQ(inVec.z, outVec.z);
}

TEST(SerialitionTest, dvec4Tests) {
    dvec4 inVec(1.1, 2.2, 3.3, 4.4), outVec;
    outVec = serializationOfType(inVec);
    EXPECT_EQ(inVec.x, outVec.x);
    EXPECT_EQ(inVec.y, outVec.y);
    EXPECT_EQ(inVec.z, outVec.z);
    EXPECT_EQ(inVec.w, outVec.w);
}

TEST(SerialitionTest, mat2Tests) {
    mat2 inMat(0.0f, 1.0f, 2.0f, 3.0f), outMat;
    outMat = serializationOfType(inMat);
    int s = 2;

    for (int i = 0; i < s; i++)
        for (int j = 0; j < s; j++) EXPECT_EQ(inMat[i][j], outMat[i][j]);
}

TEST(SerialitionTest, mat3Tests) {
    mat3 inMat(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f), outMat;
    outMat = serializationOfType(inMat);
    int s = 3;

    for (int i = 0; i < s; i++)
        for (int j = 0; j < s; j++) EXPECT_EQ(inMat[i][j], outMat[i][j]);
}

TEST(SerialitionTest, mat4Tests) {
    mat4 inMat(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f,
               13.0f, 14.0f, 15.0f),
        outMat;
    outMat = serializationOfType(inMat);
    int s = 4;

    for (int i = 0; i < s; i++)
        for (int j = 0; j < s; j++) EXPECT_EQ(inMat[i][j], outMat[i][j]);
}
}