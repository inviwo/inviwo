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

namespace inviwo::hdf5 {

TEST(DimSelectionProperty, DefaultSelectsAll) {
    DimSelectionProperty prop{"dim", "dim"};
    prop.update(10);

    const auto sel = prop.getSelection();
    EXPECT_EQ(sel.start, 0u);
    EXPECT_EQ(sel.count, 0u);
    EXPECT_EQ(sel.stride, 1u);
}

TEST(DimSelectionProperty, StartAndCount) {
    DimSelectionProperty prop{"dim", "dim"};
    prop.update(10);
    prop.start.set(2);
    prop.count.set(3);

    const auto sel = prop.getSelection();
    EXPECT_EQ(sel.start, 2u);
    EXPECT_EQ(sel.count, 3u);
    EXPECT_EQ(sel.stride, 1u);
}

}  // namespace inviwo::hdf5
