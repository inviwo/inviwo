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
#include <inviwo/core/util/indexmapper.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/datastructures/image/layerram.h>

#include <span>
#include <tuple>
#include <vector>
#include <string_view>

namespace inviwo {

constexpr std::string_view img1 = R"(
 25 ─X──────────────────────────────────────────────────────────────────────┐
     │0000000000000000000000000000000000000000000000000000000000000000000000│
     │0000000000000000000000000000000000000000000000000000000000000000000000│
     │0000000000000000000000000000000000000000000000000000000000000000000000│
     │0000000000000000002222222222222222200000000000000000000000000000000000│
 20 ─┤0000000000000000002222222222222222200000000000000000000000000000000000│
     │0000000000000000002222222222222222200000000000000000000000000000000000│
     │0000000000000000002222222222222222244444444444444444000000000000000000│
     │0000000000000000002222222222222222244444444444444444000000000000000000│
     │0000000333333333333333332222222222244444444444444444000000000000000000│
 15 ─┤0000000333333333333333332222222222244444444444441111111111111111100000│
     │0000000333333333333333332222222222244444444444441111111111111111100000│
     │0000000333333333333333332222222222244444444444441111111111111111100000│
     │0000000333333333333333330000000000044444444444441111111111111111100000│
     │0000000333333333333333330000000000044444444444441111111111111111100000│
 10 ─┤0000000333333333333333330000000000044444444444441111111111111111100000│
     │0000000333333333333333330000000000000000000000001111111111111111100000│
     │0000000333333333333333330000000000000000000000001111111111111111100000│
     │0000000000000000000000000000000000000000000000001111111111111111100000│
     │0000000000000000000000000000000000000000000000000000000000000000000000│
 5  ─┤0000000000000000000000000000000000000000000000000000000000000000000000│
     │0000000000000000000000000000000000000000000000000000000000000000000000│
     │0000000000000000000000000000000000000000000000000000000000000000000000│
     │0000000000000000000000000000000000000000000000000000000000000000000000│
     │0000000000000000000000000000000000000000000000000000000000000000000000│
 0  ─┤0000000000000000000000000000000000000000000000000000000000000000000000│
     └┬────┬────┬────┬────┬────┬────┬────┬────┬────┬────┬────┬────┬────┬────X
      │    │    │    │    │    │    │    │    │    │    │    │    │    │    │
      0    5   10   15   20   25   30   35   40   45   50   55   60   65   70
)";

auto strToLayer(std::string_view str) {

    auto colAndLine = [line = size_t{0},
                       col = size_t{0}](char c) mutable -> std::tuple<char, size_t, size_t> {
        auto cur = col;
        if (c == '\n') {
            ++line;
            col = 0;
        } else {
            ++col;
        }
        return {c, line, cur};
    };

    std::vector<size2_t> pos;
    for (auto&& [c, line, col] : util::transformRange(str, colAndLine)) {
         if (c == 'X') pos.emplace_back(line, col);
    }

    if (pos.size() != 2) {
        throw Exception("Image Frame not found");
    }

    const auto begin = pos[0];
    const auto end = pos[1];

    auto lines = util::splitStringView(str, '\n');

    std::span<std::string_view> imgRows{lines.data() + begin.x, end.x - begin.x};

    LayerRAMPrecision<size_t> layer{size2_t{end.y - begin.y, end.x - begin.x}};
    auto* data = layer.getDataTyped();

    for (auto row : util::as_range(imgRows.rbegin(), imgRows.rend())) {
        auto cols = row.substr(begin.y, end.y - begin.y);
        std::transform(cols.begin(), cols.end(), data, [](auto c) {
            if (c < '9' && c >= '0') {
                return static_cast<size_t>(c - '0');
            } else {
                return size_t{0};
            }
        });
    }
    return layer;
    
}

TEST(EventTransform, setup) {

    //const auto layer = strToLayer(img1);
    //const auto im = util::IndexMapper2D(layer.getDimensions());
    //auto data = layer.getDataTyped();
    
    //EXPECT_EQ(layer.getDimensions(), size2_t(25,70));
    
    //EXPECT_EQ(data[im(0,0)], size_t{0});
    //EXPECT_EQ(data[im(69,24)], size_t{0});
    //EXPECT_EQ(data[im(34,15)], size_t{2});
    //EXPECT_EQ(data[im(35,15)], size_t{3});
    
}

}  // namespace inviwo
