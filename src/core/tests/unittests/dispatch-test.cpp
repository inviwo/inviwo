/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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
#include <inviwo/core/util/formatdispatching.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>

namespace inviwo {

struct RamDispatcher {
    template <typename Result, typename Format, typename Callable, typename... Args>
    Result operator()(Callable &&obj, BufferRAM *bufferram, Args... args) {
        return obj(
            static_cast<BufferRAMPrecision<typename Format::type, BufferTarget::Data> *>(bufferram),
            std::forward<Args>(args)...);
    }
};
template <typename Result, template <class> class Predicate = dispatching::filter::All,
          typename Callable, typename... Args>
Result dispatch(BufferRAM *bufferram, Callable &&obj, Args... args) {
    RamDispatcher disp;
    return dispatching::dispatch<Result, Predicate>(bufferram->getDataFormatId(), disp,
                                                    std::forward<Callable>(obj), bufferram,
                                                    std::forward<Args>(args)...);
}

using res_t = std::tuple<DataFormatId, NumericType, size_t, size_t>;

template <template <class> class Predicate>
res_t test(DataFormatId id) {
    auto buf =
        createBufferRAM(10, DataFormatBase::get(id), BufferUsage::Static, BufferTarget::Data);

    return dispatch<res_t, Predicate>(buf.get(), [](auto b) -> res_t {
        IVW_UNUSED_PARAM(b);
        using BT = typename std::decay<decltype(*b)>::type;
        using DT = typename BT::type;
        return res_t{DataFormat<DT>::id(), DataFormat<DT>::numericType(),
                     DataFormat<DT>::components(), DataFormat<DT>::precision()};
    });
}

TEST(DispatchTests, Test1) {
    auto res = test<dispatching::filter::Vecs>(DataFormatId::Vec3Float32);

    EXPECT_EQ(DataFormatId::Vec3Float32, std::get<0>(res));
    EXPECT_EQ(NumericType::Float, std::get<1>(res));
    EXPECT_EQ(3, std::get<2>(res));
    EXPECT_EQ(32, std::get<3>(res));
}

TEST(DispatchTests, Test2) {
    auto res = test<dispatching::filter::Integers>(DataFormatId::Vec3Int32);

    EXPECT_EQ(DataFormatId::Vec3Int32, std::get<0>(res));
    EXPECT_EQ(NumericType::SignedInteger, std::get<1>(res));
    EXPECT_EQ(3, std::get<2>(res));
    EXPECT_EQ(32, std::get<3>(res));
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

    auto res = dispatch<float, dispatching::filter::Scalars>(buf.get(), [](auto b) {
        IVW_UNUSED_PARAM(b);
        using BT = typename std::decay<decltype(*b)>::type;
        using DT = typename BT::type;

        DT v1{0};
        DT v2{1};

        auto min = std::min(v1, v2);
        return static_cast<float>(min);
    });
    EXPECT_EQ(0.0f, res);
}

TEST(DispatchTests, InstantiationTest2) {
    auto buf = createBufferRAM(10, DataFormatBase::get(DataFormatId::Vec3Float32),
                               BufferUsage::Static, BufferTarget::Data);

    auto res = dispatch<float, dispatching::filter::Vecs>(buf.get(), [](auto b) {
        IVW_UNUSED_PARAM(b);
        using BT = typename std::decay<decltype(*b)>::type;
        using DT = typename BT::type;

        DT v1{0};
        DT v2{1};

        auto min = glm::min(v1, v2);
        return static_cast<float>(min[0]);
    });
    EXPECT_EQ(0.0f, res);
}

}  // namespace inviwo
