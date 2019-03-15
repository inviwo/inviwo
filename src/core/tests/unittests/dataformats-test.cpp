/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

namespace inviwo {

TEST(DataFormatsTests, Float16Test) {
    auto df = DataFloat16::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(2, df->getSize());
    EXPECT_EQ(16, df->getPrecision());
    EXPECT_EQ(1, df->getComponents());
    EXPECT_STREQ("FLOAT16", df->getString());
    EXPECT_EQ(NumericType::Float, df->getNumericType());
    EXPECT_EQ(DataFormatId::Float16, df->getId());
}

TEST(DataFormatsTests, Float16Vec2Test) {
    auto df = DataVec2Float16::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(4, df->getSize());
    EXPECT_EQ(16, df->getPrecision());
    EXPECT_EQ(2, df->getComponents());
    EXPECT_STREQ("Vec2FLOAT16", df->getString());
    EXPECT_EQ(NumericType::Float, df->getNumericType());
    EXPECT_EQ(DataFormatId::Vec2Float16, df->getId());
}

TEST(DataFormatsTests, Float16Vec3Test) {
    auto df = DataVec3Float16::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(6, df->getSize());
    EXPECT_EQ(16, df->getPrecision());
    EXPECT_EQ(3, df->getComponents());
    EXPECT_STREQ("Vec3FLOAT16", df->getString());
    EXPECT_EQ(NumericType::Float, df->getNumericType());
    EXPECT_EQ(DataFormatId::Vec3Float16, df->getId());
}

TEST(DataFormatsTests, Float16Vec4Test) {
    auto df = DataVec4Float16::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(8, df->getSize());
    EXPECT_EQ(16, df->getPrecision());
    EXPECT_EQ(4, df->getComponents());
    EXPECT_STREQ("Vec4FLOAT16", df->getString());
    EXPECT_EQ(NumericType::Float, df->getNumericType());
    EXPECT_EQ(DataFormatId::Vec4Float16, df->getId());
}

TEST(DataFormatsTests, Float32Test) {
    auto df = DataFloat32::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(4, df->getSize());
    EXPECT_EQ(32, df->getPrecision());
    EXPECT_EQ(1, df->getComponents());
    EXPECT_STREQ("FLOAT32", df->getString());
    EXPECT_EQ(NumericType::Float, df->getNumericType());
    EXPECT_EQ(DataFormatId::Float32, df->getId());
}

TEST(DataFormatsTests, Float32Vec2Test) {
    auto df = DataVec2Float32::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(8, df->getSize());
    EXPECT_EQ(32, df->getPrecision());
    EXPECT_EQ(2, df->getComponents());
    EXPECT_STREQ("Vec2FLOAT32", df->getString());
    EXPECT_EQ(NumericType::Float, df->getNumericType());
    EXPECT_EQ(DataFormatId::Vec2Float32, df->getId());
}

TEST(DataFormatsTests, Float32Vec3Test) {
    auto df = DataVec3Float32::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(12, df->getSize());
    EXPECT_EQ(32, df->getPrecision());
    EXPECT_EQ(3, df->getComponents());
    EXPECT_STREQ("Vec3FLOAT32", df->getString());
    EXPECT_EQ(NumericType::Float, df->getNumericType());
    EXPECT_EQ(DataFormatId::Vec3Float32, df->getId());
}

TEST(DataFormatsTests, Float32Vec4Test) {
    auto df = DataVec4Float32::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(16, df->getSize());
    EXPECT_EQ(32, df->getPrecision());
    EXPECT_EQ(4, df->getComponents());
    EXPECT_STREQ("Vec4FLOAT32", df->getString());
    EXPECT_EQ(NumericType::Float, df->getNumericType());
    EXPECT_EQ(DataFormatId::Vec4Float32, df->getId());
}

TEST(DataFormatsTests, Float64Test) {
    auto df = DataFloat64::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(8, df->getSize());
    EXPECT_EQ(64, df->getPrecision());
    EXPECT_EQ(1, df->getComponents());
    EXPECT_STREQ("FLOAT64", df->getString());
    EXPECT_EQ(NumericType::Float, df->getNumericType());
    EXPECT_EQ(DataFormatId::Float64, df->getId());
}

TEST(DataFormatsTests, Float64Vec2Test) {
    auto df = DataVec2Float64::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(16, df->getSize());
    EXPECT_EQ(64, df->getPrecision());
    EXPECT_EQ(2, df->getComponents());
    EXPECT_STREQ("Vec2FLOAT64", df->getString());
    EXPECT_EQ(NumericType::Float, df->getNumericType());
    EXPECT_EQ(DataFormatId::Vec2Float64, df->getId());
}

TEST(DataFormatsTests, Float64Vec3Test) {
    auto df = DataVec3Float64::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(24, df->getSize());
    EXPECT_EQ(64, df->getPrecision());
    EXPECT_EQ(3, df->getComponents());
    EXPECT_STREQ("Vec3FLOAT64", df->getString());
    EXPECT_EQ(NumericType::Float, df->getNumericType());
    EXPECT_EQ(DataFormatId::Vec3Float64, df->getId());
}

TEST(DataFormatsTests, Float64Vec4Test) {
    auto df = DataVec4Float64::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(32, df->getSize());
    EXPECT_EQ(64, df->getPrecision());
    EXPECT_EQ(4, df->getComponents());
    EXPECT_STREQ("Vec4FLOAT64", df->getString());
    EXPECT_EQ(NumericType::Float, df->getNumericType());
    EXPECT_EQ(DataFormatId::Vec4Float64, df->getId());
}

TEST(DataFormatsTests, Int8Test) {
    auto df = DataInt8::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(1, df->getSize());
    EXPECT_EQ(8, df->getPrecision());
    EXPECT_EQ(1, df->getComponents());
    EXPECT_STREQ("INT8", df->getString());
    EXPECT_EQ(NumericType::SignedInteger, df->getNumericType());
    EXPECT_EQ(DataFormatId::Int8, df->getId());
}

TEST(DataFormatsTests, Int8Vec2Test) {
    auto df = DataVec2Int8::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(2, df->getSize());
    EXPECT_EQ(8, df->getPrecision());
    EXPECT_EQ(2, df->getComponents());
    EXPECT_STREQ("Vec2INT8", df->getString());
    EXPECT_EQ(NumericType::SignedInteger, df->getNumericType());
    EXPECT_EQ(DataFormatId::Vec2Int8, df->getId());
}

TEST(DataFormatsTests, Int8Vec3Test) {
    auto df = DataVec3Int8::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(3, df->getSize());
    EXPECT_EQ(8, df->getPrecision());
    EXPECT_EQ(3, df->getComponents());
    EXPECT_STREQ("Vec3INT8", df->getString());
    EXPECT_EQ(NumericType::SignedInteger, df->getNumericType());
    EXPECT_EQ(DataFormatId::Vec3Int8, df->getId());
}

TEST(DataFormatsTests, Int8Vec4Test) {
    auto df = DataVec4Int8::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(4, df->getSize());
    EXPECT_EQ(8, df->getPrecision());
    EXPECT_EQ(4, df->getComponents());
    EXPECT_STREQ("Vec4INT8", df->getString());
    EXPECT_EQ(NumericType::SignedInteger, df->getNumericType());
    EXPECT_EQ(DataFormatId::Vec4Int8, df->getId());
}

TEST(DataFormatsTests, Int16Test) {
    auto df = DataInt16::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(2, df->getSize());
    EXPECT_EQ(16, df->getPrecision());
    EXPECT_EQ(1, df->getComponents());
    EXPECT_STREQ("INT16", df->getString());
    EXPECT_EQ(NumericType::SignedInteger, df->getNumericType());
    EXPECT_EQ(DataFormatId::Int16, df->getId());
}

TEST(DataFormatsTests, Int16Vec2Test) {
    auto df = DataVec2Int16::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(4, df->getSize());
    EXPECT_EQ(16, df->getPrecision());
    EXPECT_EQ(2, df->getComponents());
    EXPECT_STREQ("Vec2INT16", df->getString());
    EXPECT_EQ(NumericType::SignedInteger, df->getNumericType());
    EXPECT_EQ(DataFormatId::Vec2Int16, df->getId());
}

TEST(DataFormatsTests, Int16Vec3Test) {
    auto df = DataVec3Int16::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(6, df->getSize());
    EXPECT_EQ(16, df->getPrecision());
    EXPECT_EQ(3, df->getComponents());
    EXPECT_STREQ("Vec3INT16", df->getString());
    EXPECT_EQ(NumericType::SignedInteger, df->getNumericType());
    EXPECT_EQ(DataFormatId::Vec3Int16, df->getId());
}

TEST(DataFormatsTests, Int16Vec4Test) {
    auto df = DataVec4Int16::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(8, df->getSize());
    EXPECT_EQ(16, df->getPrecision());
    EXPECT_EQ(4, df->getComponents());
    EXPECT_STREQ("Vec4INT16", df->getString());
    EXPECT_EQ(NumericType::SignedInteger, df->getNumericType());
    EXPECT_EQ(DataFormatId::Vec4Int16, df->getId());
}

TEST(DataFormatsTests, Int32Test) {
    auto df = DataInt32::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(4, df->getSize());
    EXPECT_EQ(32, df->getPrecision());
    EXPECT_EQ(1, df->getComponents());
    EXPECT_STREQ("INT32", df->getString());
    EXPECT_EQ(NumericType::SignedInteger, df->getNumericType());
    EXPECT_EQ(DataFormatId::Int32, df->getId());
}

TEST(DataFormatsTests, Int32Vec2Test) {
    auto df = DataVec2Int32::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(8, df->getSize());
    EXPECT_EQ(32, df->getPrecision());
    EXPECT_EQ(2, df->getComponents());
    EXPECT_STREQ("Vec2INT32", df->getString());
    EXPECT_EQ(NumericType::SignedInteger, df->getNumericType());
    EXPECT_EQ(DataFormatId::Vec2Int32, df->getId());
}

TEST(DataFormatsTests, Int32Vec3Test) {
    auto df = DataVec3Int32::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(12, df->getSize());
    EXPECT_EQ(32, df->getPrecision());
    EXPECT_EQ(3, df->getComponents());
    EXPECT_STREQ("Vec3INT32", df->getString());
    EXPECT_EQ(NumericType::SignedInteger, df->getNumericType());
    EXPECT_EQ(DataFormatId::Vec3Int32, df->getId());
}

TEST(DataFormatsTests, Int32Vec4Test) {
    auto df = DataVec4Int32::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(16, df->getSize());
    EXPECT_EQ(32, df->getPrecision());
    EXPECT_EQ(4, df->getComponents());
    EXPECT_STREQ("Vec4INT32", df->getString());
    EXPECT_EQ(NumericType::SignedInteger, df->getNumericType());
    EXPECT_EQ(DataFormatId::Vec4Int32, df->getId());
}

TEST(DataFormatsTests, Int64Test) {
    auto df = DataInt64::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(8, df->getSize());
    EXPECT_EQ(64, df->getPrecision());
    EXPECT_EQ(1, df->getComponents());
    EXPECT_STREQ("INT64", df->getString());
    EXPECT_EQ(NumericType::SignedInteger, df->getNumericType());
    EXPECT_EQ(DataFormatId::Int64, df->getId());
}

TEST(DataFormatsTests, Int64Vec2Test) {
    auto df = DataVec2Int64::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(16, df->getSize());
    EXPECT_EQ(64, df->getPrecision());
    EXPECT_EQ(2, df->getComponents());
    EXPECT_STREQ("Vec2INT64", df->getString());
    EXPECT_EQ(NumericType::SignedInteger, df->getNumericType());
    EXPECT_EQ(DataFormatId::Vec2Int64, df->getId());
}

TEST(DataFormatsTests, Int64Vec3Test) {
    auto df = DataVec3Int64::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(24, df->getSize());
    EXPECT_EQ(64, df->getPrecision());
    EXPECT_EQ(3, df->getComponents());
    EXPECT_STREQ("Vec3INT64", df->getString());
    EXPECT_EQ(NumericType::SignedInteger, df->getNumericType());
    EXPECT_EQ(DataFormatId::Vec3Int64, df->getId());
}

TEST(DataFormatsTests, Int64Vec4Test) {
    auto df = DataVec4Int64::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(32, df->getSize());
    EXPECT_EQ(64, df->getPrecision());
    EXPECT_EQ(4, df->getComponents());
    EXPECT_STREQ("Vec4INT64", df->getString());
    EXPECT_EQ(NumericType::SignedInteger, df->getNumericType());
    EXPECT_EQ(DataFormatId::Vec4Int64, df->getId());
}

TEST(DataFormatsTests, UInt8Test) {
    auto df = DataUInt8::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(1, df->getSize());
    EXPECT_EQ(8, df->getPrecision());
    EXPECT_EQ(1, df->getComponents());
    EXPECT_STREQ("UINT8", df->getString());
    EXPECT_EQ(NumericType::UnsignedInteger, df->getNumericType());
    EXPECT_EQ(DataFormatId::UInt8, df->getId());
}

TEST(DataFormatsTests, UInt8Vec2Test) {
    auto df = DataVec2UInt8::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(2, df->getSize());
    EXPECT_EQ(8, df->getPrecision());
    EXPECT_EQ(2, df->getComponents());
    EXPECT_STREQ("Vec2UINT8", df->getString());
    EXPECT_EQ(NumericType::UnsignedInteger, df->getNumericType());
    EXPECT_EQ(DataFormatId::Vec2UInt8, df->getId());
}

TEST(DataFormatsTests, UInt8Vec3Test) {
    auto df = DataVec3UInt8::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(3, df->getSize());
    EXPECT_EQ(8, df->getPrecision());
    EXPECT_EQ(3, df->getComponents());
    EXPECT_STREQ("Vec3UINT8", df->getString());
    EXPECT_EQ(NumericType::UnsignedInteger, df->getNumericType());
    EXPECT_EQ(DataFormatId::Vec3UInt8, df->getId());
}

TEST(DataFormatsTests, UInt8Vec4Test) {
    auto df = DataVec4UInt8::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(4, df->getSize());
    EXPECT_EQ(8, df->getPrecision());
    EXPECT_EQ(4, df->getComponents());
    EXPECT_STREQ("Vec4UINT8", df->getString());
    EXPECT_EQ(NumericType::UnsignedInteger, df->getNumericType());
    EXPECT_EQ(DataFormatId::Vec4UInt8, df->getId());
}

TEST(DataFormatsTests, UInt16Test) {
    auto df = DataUInt16::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(2, df->getSize());
    EXPECT_EQ(16, df->getPrecision());
    EXPECT_EQ(1, df->getComponents());
    EXPECT_STREQ("UINT16", df->getString());
    EXPECT_EQ(NumericType::UnsignedInteger, df->getNumericType());
    EXPECT_EQ(DataFormatId::UInt16, df->getId());
}

TEST(DataFormatsTests, UInt16Vec2Test) {
    auto df = DataVec2UInt16::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(4, df->getSize());
    EXPECT_EQ(16, df->getPrecision());
    EXPECT_EQ(2, df->getComponents());
    EXPECT_STREQ("Vec2UINT16", df->getString());
    EXPECT_EQ(NumericType::UnsignedInteger, df->getNumericType());
    EXPECT_EQ(DataFormatId::Vec2UInt16, df->getId());
}

TEST(DataFormatsTests, UInt16Vec3Test) {
    auto df = DataVec3UInt16::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(6, df->getSize());
    EXPECT_EQ(16, df->getPrecision());
    EXPECT_EQ(3, df->getComponents());
    EXPECT_STREQ("Vec3UINT16", df->getString());
    EXPECT_EQ(NumericType::UnsignedInteger, df->getNumericType());
    EXPECT_EQ(DataFormatId::Vec3UInt16, df->getId());
}

TEST(DataFormatsTests, UInt16Vec4Test) {
    auto df = DataVec4UInt16::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(8, df->getSize());
    EXPECT_EQ(16, df->getPrecision());
    EXPECT_EQ(4, df->getComponents());
    EXPECT_STREQ("Vec4UINT16", df->getString());
    EXPECT_EQ(NumericType::UnsignedInteger, df->getNumericType());
    EXPECT_EQ(DataFormatId::Vec4UInt16, df->getId());
}

TEST(DataFormatsTests, UInt32Test) {
    auto df = DataUInt32::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(4, df->getSize());
    EXPECT_EQ(32, df->getPrecision());
    EXPECT_EQ(1, df->getComponents());
    EXPECT_STREQ("UINT32", df->getString());
    EXPECT_EQ(NumericType::UnsignedInteger, df->getNumericType());
    EXPECT_EQ(DataFormatId::UInt32, df->getId());
}

TEST(DataFormatsTests, UInt32Vec2Test) {
    auto df = DataVec2UInt32::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(8, df->getSize());
    EXPECT_EQ(32, df->getPrecision());
    EXPECT_EQ(2, df->getComponents());
    EXPECT_STREQ("Vec2UINT32", df->getString());
    EXPECT_EQ(NumericType::UnsignedInteger, df->getNumericType());
    EXPECT_EQ(DataFormatId::Vec2UInt32, df->getId());
}

TEST(DataFormatsTests, UInt32Vec3Test) {
    auto df = DataVec3UInt32::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(12, df->getSize());
    EXPECT_EQ(32, df->getPrecision());
    EXPECT_EQ(3, df->getComponents());
    EXPECT_STREQ("Vec3UINT32", df->getString());
    EXPECT_EQ(NumericType::UnsignedInteger, df->getNumericType());
    EXPECT_EQ(DataFormatId::Vec3UInt32, df->getId());
}

TEST(DataFormatsTests, UInt32Vec4Test) {
    auto df = DataVec4UInt32::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(16, df->getSize());
    EXPECT_EQ(32, df->getPrecision());
    EXPECT_EQ(4, df->getComponents());
    EXPECT_STREQ("Vec4UINT32", df->getString());
    EXPECT_EQ(NumericType::UnsignedInteger, df->getNumericType());
    EXPECT_EQ(DataFormatId::Vec4UInt32, df->getId());
}

TEST(DataFormatsTests, UInt64Test) {
    auto df = DataUInt64::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(8, df->getSize());
    EXPECT_EQ(64, df->getPrecision());
    EXPECT_EQ(1, df->getComponents());
    EXPECT_STREQ("UINT64", df->getString());
    EXPECT_EQ(NumericType::UnsignedInteger, df->getNumericType());
    EXPECT_EQ(DataFormatId::UInt64, df->getId());
}

TEST(DataFormatsTests, UInt64Vec2Test) {
    auto df = DataVec2UInt64::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(16, df->getSize());
    EXPECT_EQ(64, df->getPrecision());
    EXPECT_EQ(2, df->getComponents());
    EXPECT_STREQ("Vec2UINT64", df->getString());
    EXPECT_EQ(NumericType::UnsignedInteger, df->getNumericType());
    EXPECT_EQ(DataFormatId::Vec2UInt64, df->getId());
}

TEST(DataFormatsTests, UInt64Vec3Test) {
    auto df = DataVec3UInt64::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(24, df->getSize());
    EXPECT_EQ(64, df->getPrecision());
    EXPECT_EQ(3, df->getComponents());
    EXPECT_STREQ("Vec3UINT64", df->getString());
    EXPECT_EQ(NumericType::UnsignedInteger, df->getNumericType());
    EXPECT_EQ(DataFormatId::Vec3UInt64, df->getId());
}

TEST(DataFormatsTests, UInt64Vec4Test) {
    auto df = DataVec4UInt64::get();
    ASSERT_NE(nullptr, df);

    EXPECT_EQ(32, df->getSize());
    EXPECT_EQ(64, df->getPrecision());
    EXPECT_EQ(4, df->getComponents());
    EXPECT_STREQ("Vec4UINT64", df->getString());
    EXPECT_EQ(NumericType::UnsignedInteger, df->getNumericType());
    EXPECT_EQ(DataFormatId::Vec4UInt64, df->getId());
}

}  // namespace inviwo
