/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2020 Inviwo Foundation
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

#include <modules/discretedata/connectivity/structuredgrid.h>

namespace inviwo {
namespace discretedata {

TEST(DataSet, StructuredGrid) {
#if true
    StructuredGrid<3> grid(3, 4, 5);

    // Size.
    EXPECT_EQ(grid.numPrimitives_[0], 3);
    EXPECT_EQ(grid.numPrimitives_[1], 4);
    EXPECT_EQ(grid.numPrimitives_[2], 5);

    // Number of total primitives of each type.
    EXPECT_EQ(grid.getNumElements(GridPrimitive::Vertex), 60);
    EXPECT_EQ(grid.getNumElements(GridPrimitive::Edge), 133);
    EXPECT_EQ(grid.getNumElements(GridPrimitive::Face), 98);
    EXPECT_EQ(grid.getNumElements(GridPrimitive::Volume), 24);

    // // Indexing by direction tuples.
    EXPECT_EQ(grid.numPrimitives_.getDirectionsIndex<GridPrimitive::Vertex>({}), 0);
    EXPECT_EQ(grid.numPrimitives_.getDirectionsIndex<GridPrimitive::Edge>({0}), 0);
    EXPECT_EQ(grid.numPrimitives_.getDirectionsIndex<GridPrimitive::Edge>({1}), 1);
    EXPECT_EQ(grid.numPrimitives_.getDirectionsIndex<GridPrimitive::Edge>({2}), 2);
    EXPECT_EQ(grid.numPrimitives_.getDirectionsIndex<GridPrimitive::Face>({0, 1}), 0);
    EXPECT_EQ(grid.numPrimitives_.getDirectionsIndex<GridPrimitive::Face>({0, 2}), 1);
    EXPECT_EQ(grid.numPrimitives_.getDirectionsIndex<GridPrimitive::Face>({1, 2}), 2);
    EXPECT_EQ(grid.numPrimitives_.getDirectionsIndex<GridPrimitive::Volume>({0, 1, 2}), 0);

    // Offsets of a certain direction tuple withing the primitive type.
    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Vertex>({}), 0);
    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Vertex>(0), 0);

    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Edge>({0}), 0);
    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Edge>(0), 0);
    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Edge>({1}), 40);
    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Edge>(1), 40);
    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Edge>({2}), 85);
    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Edge>(2), 85);

    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Face>({0, 1}), 0);
    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Face>(0), 0);
    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Face>({0, 2}), 30);
    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Face>(1), 30);
    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Face>({1, 2}), 62);
    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Face>(2), 62);

    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Volume>({0, 1, 2}), 0);
    EXPECT_EQ(*grid.numPrimitives_.getOffset<GridPrimitive::Volume>(0), 0);

    auto offsets = dd_util::binomialCoefficientOffsets<3>();
    std::cout << "  Primitive offsets: [" << offsets[0] << ", " << offsets[1] << ", " << offsets[2]
              << ", " << offsets[3] << "]" << std::endl;

    // Number *of primitives in a certain direction.
    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Vertex>(0), 60);
    std::cout << "0" << std::endl;
    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Vertex>({0}), 60);
    std::cout << "{0}" << std::endl;

    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Edge>(0), 40);
    std::cout << "0" << std::endl;
    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Edge>({0}), 40);
    std::cout << "{0}" << std::endl;
    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Edge>(1), 45);
    std::cout << "1" << std::endl;
    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Edge>({1}), 45);
    std::cout << "{1}" << std::endl;
    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Edge>(2), 48);
    std::cout << "2" << std::endl;
    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Edge>({2}), 48);
    std::cout << "{2}" << std::endl;

    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Face>({0, 1}), 30);
    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Face>(0), 30);
    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Face>({0, 2}), 32);
    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Face>(1), 32);
    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Face>({1, 2}), 36);
    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Face>(2), 36);
    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Volume>({0, 1, 2}), 24);
    EXPECT_EQ(grid.numPrimitives_.getSize<GridPrimitive::Volume>(0), 24);

    // globalIndexFromCoordinates
#endif
}

}  // namespace discretedata
}  // namespace inviwo
