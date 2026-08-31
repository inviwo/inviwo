/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <modules/hdf5/properties/dimselectionproperty.h>
#include <modules/hdf5/properties/dimselectionsproperty.h>
#include <modules/hdf5/datastructures/hdf5selection.h>
#include <modules/hdf5/hdf5utils.h>
#include <modules/hdf5/datastructures/hdf5path.h>

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

#include <vector>

namespace inviwo {

namespace hdf5 {

TEST(DimSelectionProperty, DefaultSelectsAll) {
    DimSelectionProperty prop{"dim", "dim"};
    prop.update(10);

    const auto sel = prop.getSelection();
    // count == 0 means all: {start, dimSize, stride}
    EXPECT_EQ(sel.start, 0u);
    EXPECT_EQ(sel.end, 10u);
    EXPECT_EQ(sel.stride, 1u);
}

TEST(DimSelectionProperty, StartAndCount) {
    DimSelectionProperty prop{"dim", "dim"};
    prop.update(10);
    prop.start_.set(2);
    prop.count_.set(3);

    const auto sel = prop.getSelection();
    // end = start + count * stride = 2 + 3 * 1 = 5, hdf5 count = (5 - 2) / 1 = 3
    EXPECT_EQ(sel.start, 2u);
    EXPECT_EQ(sel.end, 5u);
    EXPECT_EQ(sel.stride, 1u);
    EXPECT_EQ((sel.end - sel.start) / sel.stride, 3u);
}

TEST(DimSelectionProperty, StrideCountsOutputElements) {
    DimSelectionProperty prop{"dim", "dim"};
    prop.update(20);
    prop.start_.set(1);
    prop.count_.set(3);
    prop.stride_.set(2);

    const auto sel = prop.getSelection();
    // end = 1 + 3 * 2 = 7, hdf5 count = (7 - 1) / 2 = 3
    EXPECT_EQ(sel.start, 1u);
    EXPECT_EQ(sel.end, 7u);
    EXPECT_EQ(sel.stride, 2u);
    EXPECT_EQ((sel.end - sel.start) / sel.stride, 3u);
}

TEST(DimSelectionProperty, CountZeroMeansAllFromStart) {
    DimSelectionProperty prop{"dim", "dim"};
    prop.update(10);
    prop.start_.set(3);
    prop.count_.set(size_t{0});

    const auto sel = prop.getSelection();
    EXPECT_EQ(sel.start, 3u);
    EXPECT_EQ(sel.end, 10u);
    EXPECT_EQ(sel.stride, 1u);
}

TEST(DimSelectionProperty, EndClampsToDimSize) {
    DimSelectionProperty prop{"dim", "dim"};
    prop.update(10);
    prop.start_.set(8);
    prop.count_.set(5);

    const auto sel = prop.getSelection();
    // start + count = 13, clamped to dimSize 10
    EXPECT_EQ(sel.start, 8u);
    EXPECT_EQ(sel.end, 10u);
}

TEST(DimSelectionProperty, MaxSelectionCoversWholeDim) {
    DimSelectionProperty prop{"dim", "dim"};
    prop.update(42);

    const auto sel = prop.getMaxSelection();
    EXPECT_EQ(sel.start, 0u);
    EXPECT_EQ(sel.end, 42u);
    EXPECT_EQ(sel.stride, 1u);
}

TEST(DimSelectionsProperty, ShowsTrailingDimsForRank) {
    DimSelectionsProperty prop{"selection", "Selection", 4};

    // Column major dims {4, 5, 6}; DataSetInfo stores row major, so reverse.
    const DataSetInfo meta{Path{"/data"}, nullptr, std::vector<size_t>{6, 5, 4}};
    prop.update(meta);

    const auto sel = prop.getSelection();
    ASSERT_EQ(sel.size(), 3u);
    // default count == 0 => end == dim size, so this verifies the dimension mapping/order
    EXPECT_EQ(sel[0].end, 4u);
    EXPECT_EQ(sel[1].end, 5u);
    EXPECT_EQ(sel[2].end, 6u);
}

TEST(DimSelectionsProperty, MaxSelectionMatchesDims) {
    DimSelectionsProperty prop{"selection", "Selection", 4};

    const DataSetInfo meta{Path{"/data"}, nullptr, std::vector<size_t>{8, 16}};
    prop.update(meta);

    const auto sel = prop.getMaxSelection();
    ASSERT_EQ(sel.size(), 2u);
    EXPECT_EQ(sel[0].end, 16u);
    EXPECT_EQ(sel[1].end, 8u);
}

TEST(DimSelectionsProperty, RankClampedToMaxRank) {
    DimSelectionsProperty prop{"selection", "Selection", 2};

    const DataSetInfo meta{Path{"/data"}, nullptr, std::vector<size_t>{2, 3, 4}};
    prop.update(meta);

    // Only maxRank (2) sub-selections can be active even though the data has rank 3.
    EXPECT_EQ(prop.getSelection().size(), 2u);
}

}  // namespace hdf5

}  // namespace inviwo
