/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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

#include <inviwo/core/datastructures/datamapper.h>
#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/glmvec.h>

#include <modules/opengl/util/glformatutils.h>

namespace inviwo {

TEST(RangeConversion, DoubleIdentity) {
    const DataFormatBase* dstFormat = DataFormatBase::get(DataFormatId::Float32);

    DataMapper srcMap{dvec2{0.0, 1.0}, dvec2{0.0, 1.0}};
    DataMapper dstMap{dvec2{0.0, 1.0}, dvec2{0.0, 1.0}};

    const auto conversion = utilgl::createGLFormatConversion(srcMap, dstMap, dstFormat);

    EXPECT_DOUBLE_EQ(0.5, utilgl::mapFromNormalizedGLToValue(0.5, conversion));

    EXPECT_DOUBLE_EQ(0.0, utilgl::mapFromNormalizedGLToValue(0.0, conversion));

    EXPECT_DOUBLE_EQ(0.5, utilgl::mapFromValueToGLOutput(0.5, conversion));
    EXPECT_DOUBLE_EQ(-1.0, utilgl::mapFromValueToGLOutput(-1.0, conversion));
    EXPECT_DOUBLE_EQ(1.0, utilgl::mapFromValueToGLOutput(1.0, conversion));
    EXPECT_DOUBLE_EQ(2.0, utilgl::mapFromValueToGLOutput(2.0, conversion));
}

TEST(RangeConversion, DoubleSymmetric) {
    const DataFormatBase* dstFormat = DataFormatBase::get(DataFormatId::Float32);

    DataMapper srcMap{dvec2{-1.0, 1.0}, dvec2{-1.0, 1.0}};
    DataMapper dstMap{dvec2{-1.0, 1.0}, dvec2{-1.0, 1.0}};

    const auto conversion = utilgl::createGLFormatConversion(srcMap, dstMap, dstFormat);

    EXPECT_DOUBLE_EQ(0.0, utilgl::mapFromNormalizedGLToValue(0.5, conversion));
    EXPECT_DOUBLE_EQ(-1.0, utilgl::mapFromNormalizedGLToValue(0.0, conversion));

    EXPECT_DOUBLE_EQ(0.0, utilgl::mapFromValueToGLOutput(0.0, conversion));
    EXPECT_DOUBLE_EQ(0.5, utilgl::mapFromValueToGLOutput(0.5, conversion));
    EXPECT_DOUBLE_EQ(1.5, utilgl::mapFromValueToGLOutput(1.5, conversion));
    EXPECT_DOUBLE_EQ(-1.5, utilgl::mapFromValueToGLOutput(-1.5, conversion));
}

TEST(RangeConversion, DoubleSignedToPositive) {
    const DataFormatBase* dstFormat = DataFormatBase::get(DataFormatId::Float32);

    DataMapper srcMap{dvec2{-1.0, 1.0}, dvec2{-1.0, 1.0}};
    DataMapper dstMap{dvec2{0.0, 1.0}, dvec2{0.0, 2.0}};

    const auto conversion = utilgl::createGLFormatConversion(srcMap, dstMap, dstFormat);

    EXPECT_DOUBLE_EQ(0.0, utilgl::mapFromNormalizedGLToValue(0.5, conversion));
    EXPECT_DOUBLE_EQ(-1.0, utilgl::mapFromNormalizedGLToValue(0.0, conversion));

    EXPECT_DOUBLE_EQ(0.0, utilgl::mapFromValueToGLOutput(0.0, conversion));
    EXPECT_DOUBLE_EQ(0.5, utilgl::mapFromValueToGLOutput(1.0, conversion));
    EXPECT_DOUBLE_EQ(-0.25, utilgl::mapFromValueToGLOutput(-0.5, conversion));
    EXPECT_DOUBLE_EQ(1.5, utilgl::mapFromValueToGLOutput(3.0, conversion));
}

TEST(RangeConversion, Int16) {
    using enum DataMapper::SignedNormalization;

    const DataFormatBase* srcFormat = DataFormatBase::get(DataFormatId::Int16);
    const DataFormatBase* dstFormat = DataFormatBase::get(DataFormatId::Int16);

    DataMapper srcMap{DataMapper::defaultDataRangeFor(srcFormat, Symmetric), dvec2{-5.0, 5.0}};
    DataMapper dstMap{DataMapper::defaultDataRangeFor(dstFormat, Symmetric), dvec2{0.0, 10.0}};

    const auto conversion = utilgl::createGLFormatConversion(srcMap, dstMap, dstFormat);

    EXPECT_DOUBLE_EQ(0.0, utilgl::mapFromNormalizedGLToValue(0.5, conversion));
    EXPECT_DOUBLE_EQ(-5.0, utilgl::mapFromNormalizedGLToValue(0.0, conversion));

    EXPECT_DOUBLE_EQ(-1.0, utilgl::mapFromValueToGLOutput(0.0, conversion));
    EXPECT_DOUBLE_EQ(1.0, utilgl::mapFromValueToGLOutput(10.0, conversion));
}

TEST(RangeConversion, Int16Symmetric) {
    using enum DataMapper::SignedNormalization;

    const DataFormatBase* srcFormat = DataFormatBase::get(DataFormatId::Int16);
    const DataFormatBase* dstFormat = DataFormatBase::get(DataFormatId::Int16);

    DataMapper srcMap{DataMapper::defaultDataRangeFor(srcFormat, Symmetric), dvec2{-5.0, 5.0}};
    DataMapper dstMap{DataMapper::defaultDataRangeFor(dstFormat, Symmetric), dvec2{-10.0, 10.0}};

    const auto conversion = utilgl::createGLFormatConversion(srcMap, dstMap, dstFormat);

    EXPECT_DOUBLE_EQ(-1.0, utilgl::mapFromValueToGLOutput(-10.0, conversion));
    EXPECT_DOUBLE_EQ(0.0, utilgl::mapFromValueToGLOutput(0.0, conversion));
    EXPECT_DOUBLE_EQ(1.0, utilgl::mapFromValueToGLOutput(10.0, conversion));
}

TEST(RangeConversion, Int16Positive) {
    using enum DataMapper::SignedNormalization;

    const DataFormatBase* srcFormat = DataFormatBase::get(DataFormatId::Int16);
    const DataFormatBase* dstFormat = DataFormatBase::get(DataFormatId::Int16);

    DataMapper srcMap{DataMapper::defaultDataRangeFor(srcFormat, Symmetric), dvec2{-5.0, 5.0}};
    DataMapper dstMap{dvec2{0.0, dstFormat->getMax()}, dvec2{0.0, 10.0}};

    const auto conversion = utilgl::createGLFormatConversion(srcMap, dstMap, dstFormat);

    EXPECT_DOUBLE_EQ(-1.0, utilgl::mapFromValueToGLOutput(-10.0, conversion));
    EXPECT_DOUBLE_EQ(0.0, utilgl::mapFromValueToGLOutput(0.0, conversion));
    EXPECT_DOUBLE_EQ(1.0, utilgl::mapFromValueToGLOutput(10.0, conversion));
}

TEST(RangeConversion, UInt16) {
    using enum DataMapper::SignedNormalization;

    const DataFormatBase* srcFormat = DataFormatBase::get(DataFormatId::UInt16);
    const DataFormatBase* dstFormat = DataFormatBase::get(DataFormatId::UInt16);

    DataMapper srcMap{DataMapper::defaultDataRangeFor(srcFormat, Symmetric), dvec2{-5.0, 5.0}};
    DataMapper dstMap{DataMapper::defaultDataRangeFor(dstFormat, Symmetric), dvec2{0.0, 10.0}};

    const auto conversion = utilgl::createGLFormatConversion(srcMap, dstMap, dstFormat);

    EXPECT_DOUBLE_EQ(0.0, utilgl::mapFromNormalizedGLToValue(0.5, conversion));
    EXPECT_DOUBLE_EQ(-5.0, utilgl::mapFromNormalizedGLToValue(0.0, conversion));

    EXPECT_DOUBLE_EQ(-0.1, utilgl::mapFromValueToGLOutput(-1.0, conversion));
    EXPECT_DOUBLE_EQ(0.0, utilgl::mapFromValueToGLOutput(0.0, conversion));
    EXPECT_DOUBLE_EQ(1.0, utilgl::mapFromValueToGLOutput(10.0, conversion));
}

TEST(RangeConversion, Int32) {
    using enum DataMapper::SignedNormalization;

    const DataFormatBase* srcFormat = DataFormatBase::get(DataFormatId::Int32);
    const DataFormatBase* dstFormat = DataFormatBase::get(DataFormatId::Int32);

    DataMapper srcMap{DataMapper::defaultDataRangeFor(srcFormat, Symmetric), dvec2{-5.0, 5.0}};
    DataMapper dstMap{DataMapper::defaultDataRangeFor(dstFormat, Symmetric), dvec2{0.0, 10.0}};

    const auto conversion = utilgl::createGLFormatConversion(srcMap, dstMap, dstFormat);

    EXPECT_DOUBLE_EQ(0.0, utilgl::mapFromNormalizedGLToValue(0.5, conversion));
    EXPECT_DOUBLE_EQ(-5.0, utilgl::mapFromNormalizedGLToValue(0.0, conversion));

    EXPECT_DOUBLE_EQ(dstMap.dataRange.x, utilgl::mapFromValueToGLOutput(0.0, conversion));
    EXPECT_DOUBLE_EQ(dstMap.dataRange.y, utilgl::mapFromValueToGLOutput(10.0, conversion));
}

TEST(RangeConversion, Int32Symmetric) {
    using enum DataMapper::SignedNormalization;

    const DataFormatBase* srcFormat = DataFormatBase::get(DataFormatId::Int32);
    const DataFormatBase* dstFormat = DataFormatBase::get(DataFormatId::Int32);

    DataMapper srcMap{DataMapper::defaultDataRangeFor(srcFormat, Symmetric), dvec2{-5.0, 5.0}};
    DataMapper dstMap{DataMapper::defaultDataRangeFor(dstFormat, Symmetric), dvec2{-10.0, 10.0}};

    const auto conversion = utilgl::createGLFormatConversion(srcMap, dstMap, dstFormat);

    EXPECT_DOUBLE_EQ(dstMap.dataRange.x, utilgl::mapFromValueToGLOutput(-10.0, conversion));
    EXPECT_DOUBLE_EQ(0.0, utilgl::mapFromValueToGLOutput(0.0, conversion));
    EXPECT_DOUBLE_EQ(dstMap.dataRange.y, utilgl::mapFromValueToGLOutput(10.0, conversion));
}

TEST(RangeConversion, UInt32) {
    using enum DataMapper::SignedNormalization;

    const DataFormatBase* srcFormat = DataFormatBase::get(DataFormatId::UInt32);
    const DataFormatBase* dstFormat = DataFormatBase::get(DataFormatId::UInt32);

    DataMapper srcMap{DataMapper::defaultDataRangeFor(srcFormat, Symmetric), dvec2{-5.0, 5.0}};
    DataMapper dstMap{DataMapper::defaultDataRangeFor(dstFormat, Symmetric), dvec2{0.0, 10.0}};

    const auto conversion = utilgl::createGLFormatConversion(srcMap, dstMap, dstFormat);

    EXPECT_DOUBLE_EQ(0.0, utilgl::mapFromNormalizedGLToValue(0.5, conversion));
    EXPECT_DOUBLE_EQ(-5.0, utilgl::mapFromNormalizedGLToValue(0.0, conversion));

    EXPECT_DOUBLE_EQ(dstMap.dataRange.x, utilgl::mapFromValueToGLOutput(0.0, conversion));
    EXPECT_DOUBLE_EQ(dstMap.dataRange.y, utilgl::mapFromValueToGLOutput(10.0, conversion));
}

}  // namespace inviwo
