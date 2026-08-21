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

    EventTransformer::View view{.globalNdcToLocalNdc = [](const dvec3& ndc) { return ndc; },
                                .propagateEvent = [](Event*, Outport*) {},
                                .size = []() { return size2_t(70, 25); }};

    EventTransformer transformer{{view}};
}

// Helper: create two side-by-side views splitting at ndc.x == 0
// Left view: ndc.x in [-1, 0)  mapped to [-1, 1]
// Right view: ndc.x in [0, 1]  mapped to [-1, 1]
struct TwoViewFixture {
    std::unique_ptr<Event> e1;
    std::unique_ptr<Event> e2;
    int e1Count = 0;
    int e2Count = 0;

    EventTransformer::View view1{.globalNdcToLocalNdc =
                                     [](const dvec3& ndc) {
                                         return dvec3{ndc.x * 2.0 + 1.0, ndc.y, ndc.z};
                                     },
                                 .propagateEvent =
                                     [this](Event* event, Outport*) {
                                         e1.reset(event->clone());
                                         ++e1Count;
                                     },
                                 .size = []() { return size2_t(35, 25); }};

    EventTransformer::View view2{.globalNdcToLocalNdc =
                                     [](const dvec3& ndc) {
                                         return dvec3{ndc.x * 2.0 - 1.0, ndc.y, ndc.z};
                                     },
                                 .propagateEvent =
                                     [this](Event* event, Outport*) {
                                         e2.reset(event->clone());
                                         ++e2Count;
                                     },
                                 .size = []() { return size2_t(35, 25); }};

    EventTransformer transformer{{view1, view2}};

    void reset() {
        e1.reset();
        e2.reset();
        e1Count = 0;
        e2Count = 0;
    }
};

TEST(EventTransform, MoveInLeftView) {
    TwoViewFixture f;

    // Move at normalized position (0.25, 0.5) -> ndc.x = 2*0.25 - 1 = -0.5 -> left view
    MouseEvent moveEvent{MouseButton::None,
                         MouseState::Move,
                         MouseButtons{},
                         KeyModifiers{},
                         {0.25, 0.5},
                         size2_t(70, 25),
                         1.0};

    f.transformer.propagateEvent(&moveEvent, nullptr);

    ASSERT_TRUE(f.e1);
    EXPECT_FALSE(f.e2);

    auto* m1 = f.e1->getAs<MouseEvent>();
    ASSERT_TRUE(m1);
    EXPECT_EQ(m1->canvasSize(), uvec2(35, 25));
    // Global ndc.x = -0.5, localNdc.x = -0.5*2+1 = 0.0, normalized = (0+1)/2 = 0.5
    EXPECT_NEAR(m1->posNormalized().x, 0.5, 1e-5);
    EXPECT_NEAR(m1->posNormalized().y, 0.5, 1e-5);
    EXPECT_EQ(m1->state(), MouseState::Move);
}

TEST(EventTransform, MoveInRightView) {
    TwoViewFixture f;

    // Move at normalized position (0.75, 0.5) -> ndc.x = 2*0.75 - 1 = 0.5 -> right view
    MouseEvent moveEvent{MouseButton::None,
                         MouseState::Move,
                         MouseButtons{},
                         KeyModifiers{},
                         {0.75, 0.5},
                         size2_t(70, 25),
                         1.0};

    f.transformer.propagateEvent(&moveEvent, nullptr);

    EXPECT_FALSE(f.e1);
    ASSERT_TRUE(f.e2);

    auto* m2 = f.e2->getAs<MouseEvent>();
    ASSERT_TRUE(m2);
    EXPECT_EQ(m2->canvasSize(), uvec2(35, 25));
    // Global ndc.x = 0.5, localNdc.x = 0.5*2-1 = 0.0, normalized = (0+1)/2 = 0.5
    EXPECT_NEAR(m2->posNormalized().x, 0.5, 1e-5);
    EXPECT_NEAR(m2->posNormalized().y, 0.5, 1e-5);
}

TEST(EventTransform, PressMoveRelease) {
    TwoViewFixture f;

    // Press in left view at normalized (0.25, 0.5) -> ndc.x = -0.5
    MouseEvent pressEvent{MouseButton::Left,
                          MouseState::Press,
                          MouseButton::Left,
                          KeyModifiers{},
                          {0.25, 0.5},
                          size2_t(70, 25),
                          1.0};
    f.transformer.propagateEvent(&pressEvent, nullptr);

    ASSERT_TRUE(f.e1);
    EXPECT_FALSE(f.e2);
    EXPECT_EQ(f.e1->getAs<MouseEvent>()->state(), MouseState::Press);
    EXPECT_EQ(f.transformer.getActiveMouseView(), std::optional<size_t>(0));

    f.reset();

    // Move while pressing - should stay on view1 even if position is in right view
    // normalized (0.75, 0.5) -> ndc.x = 0.5 which would be view2,
    // but pressing state should keep view1 active
    MouseEvent moveEvent{MouseButton::None,
                         MouseState::Move,
                         MouseButton::Left,
                         KeyModifiers{},
                         {0.75, 0.5},
                         size2_t(70, 25),
                         1.0};
    f.transformer.propagateEvent(&moveEvent, nullptr);

    // During pressing, active view is NOT updated, so event goes to view1 (the pressed view)
    ASSERT_TRUE(f.e1);
    EXPECT_FALSE(f.e2);
    EXPECT_EQ(f.e1->getAs<MouseEvent>()->state(), MouseState::Move);
    EXPECT_EQ(f.transformer.getActiveMouseView(), std::optional<size_t>(0));

    f.reset();

    // Release - view is updated, so release goes to wherever the cursor actually is
    MouseEvent releaseEvent{MouseButton::Left,
                            MouseState::Release,
                            MouseButtons{},
                            KeyModifiers{},
                            {0.75, 0.5},
                            size2_t(70, 25),
                            1.0};
    f.transformer.propagateEvent(&releaseEvent, nullptr);

    // Release updates the view, so ndc.x=0.5 -> view2
    // The release event should be sent to view2 (the updated view)
    EXPECT_EQ(f.transformer.getActiveMouseView(), std::optional<size_t>(1));
}

TEST(EventTransform, MoveAcrossViews) {
    TwoViewFixture f;

    // Move in left view
    MouseEvent move1{MouseButton::None,
                     MouseState::Move,
                     MouseButtons{},
                     KeyModifiers{},
                     {0.25, 0.5},
                     size2_t(70, 25),
                     1.0};
    f.transformer.propagateEvent(&move1, nullptr);
    EXPECT_EQ(f.e1Count, 1);
    EXPECT_EQ(f.e2Count, 0);
    EXPECT_EQ(f.transformer.getActiveMouseView(), std::optional<size_t>(0));

    f.reset();

    // Move to right view
    MouseEvent move2{MouseButton::None,
                     MouseState::Move,
                     MouseButtons{},
                     KeyModifiers{},
                     {0.75, 0.5},
                     size2_t(70, 25),
                     1.0};
    f.transformer.propagateEvent(&move2, nullptr);
    EXPECT_EQ(f.e1Count, 0);
    EXPECT_EQ(f.e2Count, 1);
    EXPECT_EQ(f.transformer.getActiveMouseView(), std::optional<size_t>(1));
}

TEST(EventTransform, WheelEventInIdle) {
    TwoViewFixture f;

    // Wheel in left view at normalized (0.25, 0.5)
    WheelEvent wheelEvent{MouseButtons{}, KeyModifiers{},  dvec2{0, 120},
                          {0.25, 0.5},    size2_t(70, 25), 1.0};
    f.transformer.propagateEvent(&wheelEvent, nullptr);

    ASSERT_TRUE(f.e1);
    EXPECT_FALSE(f.e2);

    auto* w1 = f.e1->getAs<WheelEvent>();
    ASSERT_TRUE(w1);
    EXPECT_EQ(w1->canvasSize(), uvec2(35, 25));
    EXPECT_NEAR(w1->posNormalized().x, 0.5, 1e-5);
    EXPECT_EQ(w1->delta(), dvec2(0, 120));
}

TEST(EventTransform, WheelEventDuringPress) {
    TwoViewFixture f;

    // Press in left view
    MouseEvent pressEvent{MouseButton::Left,
                          MouseState::Press,
                          MouseButton::Left,
                          KeyModifiers{},
                          {0.25, 0.5},
                          size2_t(70, 25),
                          1.0};
    f.transformer.propagateEvent(&pressEvent, nullptr);
    EXPECT_EQ(f.transformer.getActiveMouseView(), std::optional<size_t>(0));

    f.reset();

    // Wheel event while pressing - should go to the active (pressed) view without updating
    WheelEvent wheelEvent{MouseButton::Left, KeyModifiers{},  dvec2{0, 120},
                          {0.75, 0.5},       size2_t(70, 25), 1.0};
    f.transformer.propagateEvent(&wheelEvent, nullptr);

    // Should go to view1 (active during press) even though position is in view2
    ASSERT_TRUE(f.e1);
    EXPECT_FALSE(f.e2);
    EXPECT_EQ(f.transformer.getActiveMouseView(), std::optional<size_t>(0));
}

TEST(EventTransform, DoubleClick) {
    TwoViewFixture f;

    // Double click in right view
    MouseEvent dblClickEvent{MouseButton::Left,
                             MouseState::DoubleClick,
                             MouseButton::Left,
                             KeyModifiers{},
                             {0.75, 0.5},
                             size2_t(70, 25),
                             1.0};
    f.transformer.propagateEvent(&dblClickEvent, nullptr);

    EXPECT_FALSE(f.e1);
    ASSERT_TRUE(f.e2);

    auto* m2 = f.e2->getAs<MouseEvent>();
    ASSERT_TRUE(m2);
    EXPECT_EQ(m2->state(), MouseState::DoubleClick);
    EXPECT_EQ(m2->canvasSize(), uvec2(35, 25));
    // Should remain in idle state after double click
    EXPECT_EQ(f.transformer.getActiveMouseView(), std::optional<size_t>(1));
}

TEST(EventTransform, EventUsedPropagation) {
    std::unique_ptr<Event> received;

    EventTransformer::View view{.globalNdcToLocalNdc = [](const dvec3& ndc) { return ndc; },
                                .propagateEvent =
                                    [&](Event* event, Outport*) {
                                        event->markAsUsed();
                                        received.reset(event->clone());
                                    },
                                .size = []() { return size2_t(70, 25); }};

    EventTransformer transformer{{view}};

    MouseEvent moveEvent{MouseButton::None,
                         MouseState::Move,
                         MouseButtons{},
                         KeyModifiers{},
                         {0.5, 0.5},
                         size2_t(70, 25),
                         1.0};
    EXPECT_FALSE(moveEvent.hasBeenUsed());

    transformer.propagateEvent(&moveEvent, nullptr);

    ASSERT_TRUE(received);
    EXPECT_TRUE(moveEvent.hasBeenUsed());
}

TEST(EventTransform, EventNotUsedPropagation) {
    EventTransformer::View view{.globalNdcToLocalNdc = [](const dvec3& ndc) { return ndc; },
                                .propagateEvent = [](Event*, Outport*) { /* don't mark as used */ },
                                .size = []() { return size2_t(70, 25); }};

    EventTransformer transformer{{view}};

    MouseEvent moveEvent{MouseButton::None,
                         MouseState::Move,
                         MouseButtons{},
                         KeyModifiers{},
                         {0.5, 0.5},
                         size2_t(70, 25),
                         1.0};

    transformer.propagateEvent(&moveEvent, nullptr);

    EXPECT_FALSE(moveEvent.hasBeenUsed());
}

TEST(EventTransform, WheelEventUsedPropagation) {
    EventTransformer::View view{
        .globalNdcToLocalNdc = [](const dvec3& ndc) { return ndc; },
        .propagateEvent = [](Event* event, Outport*) { event->markAsUsed(); },
        .size = []() { return size2_t(70, 25); }};

    EventTransformer transformer{{view}};

    WheelEvent wheelEvent{MouseButtons{}, KeyModifiers{},  dvec2{0, 120},
                          {0.5, 0.5},     size2_t(70, 25), 1.0};
    EXPECT_FALSE(wheelEvent.hasBeenUsed());

    transformer.propagateEvent(&wheelEvent, nullptr);
    EXPECT_TRUE(wheelEvent.hasBeenUsed());
}

TEST(EventTransform, CoordinateTransformEdgeCases) {
    std::unique_ptr<Event> received;

    // Single view covering the full range
    EventTransformer::View view{
        .globalNdcToLocalNdc = [](const dvec3& ndc) { return ndc; },
        .propagateEvent = [&](Event* event, Outport*) { received.reset(event->clone()); },
        .size = []() { return size2_t(100, 100); }};

    EventTransformer transformer{{view}};

    // Bottom-left corner: normalized (0,0) -> ndc (-1,-1)
    MouseEvent blEvent{MouseButton::None,
                       MouseState::Move,
                       MouseButtons{},
                       KeyModifiers{},
                       {0.0, 0.0},
                       size2_t(70, 25),
                       1.0};
    transformer.propagateEvent(&blEvent, nullptr);
    ASSERT_TRUE(received);
    auto* m = received->getAs<MouseEvent>();
    ASSERT_TRUE(m);
    EXPECT_EQ(m->canvasSize(), uvec2(100, 100));
    EXPECT_NEAR(m->posNormalized().x, 0.0, 1e-5);
    EXPECT_NEAR(m->posNormalized().y, 0.0, 1e-5);

    received.reset();

    // Top-right corner: normalized (1,1) -> ndc (1,1)
    MouseEvent trEvent{MouseButton::None,
                       MouseState::Move,
                       MouseButtons{},
                       KeyModifiers{},
                       {1.0, 1.0},
                       size2_t(70, 25),
                       1.0};
    transformer.propagateEvent(&trEvent, nullptr);
    ASSERT_TRUE(received);
    m = received->getAs<MouseEvent>();
    ASSERT_TRUE(m);
    EXPECT_NEAR(m->posNormalized().x, 1.0, 1e-5);
    EXPECT_NEAR(m->posNormalized().y, 1.0, 1e-5);
}

}  // namespace inviwo
