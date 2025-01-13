/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2025 Inviwo Foundation
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

#include <inviwo/core/util/formatdispatching.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>

namespace inviwo {

using res_t = std::tuple<DataFormatId, NumericType, size_t, size_t>;

template <template <class> class Predicate>
res_t test(DataFormatId id) {
    return dispatching::singleDispatch<res_t, Predicate>(id, []<typename T>() -> res_t {
        return res_t{DataFormat<T>::id(), DataFormat<T>::numericType(), DataFormat<T>::components(),
                     DataFormat<T>::precision()};
    });
}

TEST(DispatchTests, Test1) {
    auto&& [dataFormatId, numericType, components, precision] =
        test<dispatching::filter::Vecs>(DataFormatId::Vec3Float32);

    EXPECT_EQ(DataFormatId::Vec3Float32, dataFormatId);
    EXPECT_EQ(NumericType::Float, numericType);
    EXPECT_EQ(3, components);
    EXPECT_EQ(32, precision);
}

TEST(DispatchTests, Test2) {
    auto&& [dataFormatId, numericType, components, precision] =
        test<dispatching::filter::Integers>(DataFormatId::Vec3Int32);

    EXPECT_EQ(DataFormatId::Vec3Int32, dataFormatId);
    EXPECT_EQ(NumericType::SignedInteger, numericType);
    EXPECT_EQ(3, components);
    EXPECT_EQ(32, precision);
}

TEST(DispatchTests, ThrowTest1) {
    EXPECT_THROW(test<dispatching::filter::Integers>(DataFormatId::Vec3Float32),
                 dispatching::DispatchException);
}
TEST(DispatchTests, ThrowTest2) {
    EXPECT_THROW(test<dispatching::filter::Floats>(DataFormatId::Vec3Int32),
                 dispatching::DispatchException);
}

TEST(DispatchTests, InstantiationTest1) {
    auto buf = createBufferRAM(10, DataFormatBase::get(DataFormatId::Float32), BufferUsage::Static,
                               BufferTarget::Data);

    auto res = dispatching::singleDispatch<float, dispatching::filter::Scalars>(
        buf->getDataFormat()->getId(), [&]<typename T>() {
            using D = util::value_type_t<T>;
            D v1{0};
            D v2{1};
            auto min = std::min(v1, v2);
            return static_cast<float>(min);
        });
    EXPECT_EQ(0.0f, res);
}

TEST(DispatchTests, InstantiationTest2) {
    auto buf = createBufferRAM(10, DataFormatBase::get(DataFormatId::Vec3Float32),
                               BufferUsage::Static, BufferTarget::Data);

    auto res = dispatching::singleDispatch<float, dispatching::filter::Vecs>(
        buf->getDataFormat()->getId(), [&]<typename T>() {
            using D = util::value_type_t<T>;
            D v1{0};
            D v2{1};
            auto min = std::min(v1, v2);
            return static_cast<float>(min);
        });
    EXPECT_EQ(0.0f, res);
}

}  // namespace inviwo
