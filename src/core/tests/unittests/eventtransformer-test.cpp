/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <inviwo/core/util/transformiterator.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/util/indexmapper.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/datastructures/image/layerram.h>

#include <inviwo/core/interaction/eventtransformer.h>

#include <span>
#include <tuple>
#include <vector>
#include <string_view>

#include <fmt/format.h>
#include <fmt/ranges.h>

namespace inviwo {

auto strToLayer(std::string_view str) {
    auto colAndRow = [line = size_t{0},
                      col = size_t{0}](char c) mutable -> std::tuple<char, size_t, size_t> {
        auto curCol = col;
        if (c == '\n') {
            ++line;
            col = 0;
        } else {
            ++col;
        }
        return {c, curCol, line};
    };

    std::vector<size2_t> pos;
    for (auto&& [c, col, row] : util::transformRange(str, colAndRow)) {
        if (c == 'X') pos.emplace_back(col, row);
    }

    if (pos.size() != 2) {
        throw Exception("Image Frame not found");
    }

    const auto upperLeft = pos[0];
    const auto lowerRight = pos[1];

    auto lines = util::splitStringView(str, '\n');

    std::span<std::string_view> imgRows{lines.data() + upperLeft.y + 1,
                                        lowerRight.y - upperLeft.y - 1};

    size2_t dims = size2_t{lowerRight.x - upperLeft.x - 1, lowerRight.y - upperLeft.y - 1};

    LayerRAMPrecision<short> layer{dims};
    auto* data = layer.getDataTyped();

    for (auto [rowNumber, row] :
         util::enumerate(util::as_range(imgRows.rbegin(), imgRows.rend()))) {
        auto cols = row.substr(upperLeft.x + 1, lowerRight.x - upperLeft.x - 1);
        std::transform(cols.begin(), cols.end(), data + rowNumber * dims.x, [](auto c) {
            if (c < '9' && c >= '0') {
                return static_cast<short>(c - '0');
            } else {
                return short{0};
            }
        });
    }
    return layer;
}

constexpr std::string_view img1 = R"( Picking index 1, 2, 3, 4
 25 -X----------------------------------------------------------------------+
     |0000000000000000000000000000000000000000000000000000000000000000000000|
     |0000000000000000000000000000000000000000000000000000000000000000000000|
     |0000000000000000000000000000000000000000000000000000000000000000000000|
     |0000000000000000002222222222222222200000000000000000000000000000000000|
 20 -+0000000000000000002222222222222222200000000000000000000000000000000000|
     |0000000000000000002222222222222222200000000000000000000000000000000000|
     |0000000000000000002222222222222222244444444444444444000000000000000000|
     |0000000000000000002222222222222222244444444444444444000000000000000000|
     |0000000333333333333333332222222222244444444444444444000000000000000000|
 15 -+0000000333333333333333332222222222244444444444441111111111111111100000|
     |0000000333333333333333332222222222244444444444441111111111111111100000|
     |0000000333333333333333332222222222244444444444441111111111111111100000|
     |0000000333333333333333330000000000044444444444441111111111111111100000|
     |0000000333333333333333330000000000044444444444441111111111111111100000|
 10 -+0000000333333333333333330000000000044444444444441111111111111111100000|
     |0000000333333333333333330000000000000000000000001111111111111111100000|
     |0000000333333333333333330000000000000000000000001111111111111111100000|
     |0000000000000000000000000000000000000000000000001111111111111111100000|
     |0000000000000000000000000000000000000000000000000000000000000000000000|
 5  -+0000000000000000000000000000000000000000000000000000000000000000000000|
     |0000000000000000000000000000000000000000000000000000000000000000000000|
     |0000000000000000000000000000000000000000000000000000000000000000000000|
     |0000000000000000000000000000000000000000000000000000000000000000000000|
     |0000000000000000000000000000000000000000000000000000000000000000000000|
 0  -+0000000000000000000000000000000000000000000000000000000000000000000000|
     ++----+----+----+----+----+----+----+----+----+----+----+----+----+----X
      |    |    |    |    |    |    |    |    |    |    |    |    |    |    |
      0    5   10   15   20   25   30   35   40   45   50   55   60   65   70
)";

TEST(EventTransform, setup) {
    const auto layer = strToLayer(img1);
    const auto im = util::IndexMapper2D(layer.getDimensions());
    auto data = layer.getDataTyped();
    std::span s(data, glm::compMul(layer.getDimensions()));

    EXPECT_EQ(layer.getDimensions(), size2_t(70, 25));

    EXPECT_EQ(data[im(0, 0)], size_t{0});
    EXPECT_EQ(data[im(69, 24)], size_t{0});
    EXPECT_EQ(data[im(34, 15)], size_t{2});
    EXPECT_EQ(data[im(35, 15)], size_t{4});
    EXPECT_EQ(data[im(7, 16)], size_t{3});
}

TEST(EventTransform, construct) {

    EventTransformerView view{.isNdcInView = [](const dvec3&) { return true; },
                              .globalNdcToLocalNdc = [](const dvec3& ndc) { return ndc; },
                              .propagateEvent = [](Event*) {},
                              .viewSize = []() { return size2_t(70, 25); }};

    EventTransformer transformer{{view}};
}

TEST(EventTransform, Events) {

    std::unique_ptr<Event> e1;
    std::unique_ptr<Event> e2;

    EventTransformerView view1{.isNdcInView = [](const dvec3& ndc) { return ndc.x < 0.0; },
                               .globalNdcToLocalNdc =
                                   [](const dvec3& ndc) {
                                       return dvec3{ndc.x * 2.0 + 1.0, ndc.y, ndc.z};
                                   },
                               .propagateEvent =
                                   [&](Event* event) {
                                       fmt::println("Event 1 {}", *event);
                                       e1.reset(event->clone());
                                   },
                               .viewSize = []() { return size2_t(35, 25); }};

    EventTransformerView view2{.isNdcInView = [](const dvec3& ndc) { return ndc.x >= 0.0; },
                               .globalNdcToLocalNdc =
                                   [](const dvec3& ndc) {
                                       return dvec3{ndc.x * 2.0 - 1.0, ndc.y, ndc.z};
                                   },
                               .propagateEvent =
                                   [&](Event* event) {
                                       fmt::println("Event 2 {}", *event);
                                       e2.reset(event->clone());
                                   },
                               .viewSize = []() { return size2_t(35, 25); }};

    EventTransformer transformer{{view1, view2}};

    MouseEvent mouseEvent{MouseButton::None,
                          MouseState::Move,
                          MouseButtons{},
                          KeyModifiers{},
                          {0.25, 0.5},
                          size2_t(70, 25),
                          1.0};

    transformer.propagateEvent(&mouseEvent, nullptr);
    
    ASSERT_TRUE(e1);
    auto m1 = e1->getAs<MouseEvent>();
    ASSERT_TRUE(m1);
    
    EXPECT_EQ(m1->canvasSize(), uvec2(35, 25));
    EXPECT_NEAR(m1->ndc().x, 0.0, 0.00001);
    
}

}  // namespace inviwo
