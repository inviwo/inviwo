/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2017 Inviwo Foundation
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

#include <modules/fancymeshrenderer/HalfEdges.h>
//#include <modules/fancymeshrenderer/HalfEdges.cpp>

namespace inviwo {

// 0<=x<=width, 0<=y<=height
int toLinearIndex(int x, int y, int width, int height) { return x + y * (width + 1); }

std::shared_ptr<IndexBuffer> createPlane(int width, int height) {
    std::shared_ptr<IndexBuffer> b = std::make_shared<IndexBuffer>();
    auto indices = b->getEditableRAMRepresentation();

    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            indices->add(toLinearIndex(x, y, width, height));
            indices->add(toLinearIndex(x, y + 1, width, height));
            indices->add(toLinearIndex(x + 1, y, width, height));

            indices->add(toLinearIndex(x, y + 1, width, height));
            indices->add(toLinearIndex(x + 1, y + 1, width, height));
            indices->add(toLinearIndex(x + 1, y, width, height));
        }
    }

    return b;
}

/**
 * Create a plane
 */
TEST(HalfEdges, plane) {
    int width = 7;
    int height = 5;
    std::shared_ptr<IndexBuffer> b = createPlane(width, height);
    ASSERT_EQ(3 * 2 * width * height, b->getSize());

    HalfEdges edges(b.get());
    std::shared_ptr<IndexBuffer> b2 = edges.createIndexBufferWithAdjacency();
    ASSERT_EQ(6 * 2 * width * height, b2->getSize());

    auto indices = b2->getRAMRepresentation();
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            // I know that the implementation rotates all triangles one step around,
            // keep that in mind when testing the indices.

            // first triangle
            int tri1 = 2 * (y + x * height);
            // base triangle
            EXPECT_EQ(toLinearIndex(x, y + 1, width, height), indices->get(6 * tri1));
            EXPECT_EQ(toLinearIndex(x + 1, y, width, height), indices->get(6 * tri1 + 2));
            EXPECT_EQ(toLinearIndex(x, y, width, height), indices->get(6 * tri1 + 4));

            // neighbors
            EXPECT_EQ(toLinearIndex(x, y + 1, width, height), indices->get(6 * tri1));
            EXPECT_EQ(toLinearIndex(x + 1, y + 1, width, height), indices->get(6 * tri1 + 1));
            EXPECT_EQ(toLinearIndex(x + 1, y, width, height), indices->get(6 * tri1 + 2));

            EXPECT_EQ(toLinearIndex(x + 1, y, width, height), indices->get(6 * tri1 + 2));
            if (y > 0)
                EXPECT_EQ(toLinearIndex(x + 1, y - 1, width, height), indices->get(6 * tri1 + 3));
            else
                EXPECT_EQ(toLinearIndex(x, y + 1, width, height), indices->get(6 * tri1 + 3));
            EXPECT_EQ(toLinearIndex(x, y, width, height), indices->get(6 * tri1 + 4));

            EXPECT_EQ(toLinearIndex(x, y, width, height), indices->get(6 * tri1 + 4));
            if (x > 0)
                EXPECT_EQ(toLinearIndex(x - 1, y + 1, width, height), indices->get(6 * tri1 + 5));
            else
                EXPECT_EQ(toLinearIndex(x + 1, y, width, height), indices->get(6 * tri1 + 5));
            EXPECT_EQ(toLinearIndex(x, y + 1, width, height), indices->get(6 * tri1));

            // second triangle
            int tri2 = 2 * (y + x * height) + 1;
            EXPECT_EQ(toLinearIndex(x + 1, y + 1, width, height), indices->get(6 * tri2));
            EXPECT_EQ(toLinearIndex(x + 1, y, width, height), indices->get(6 * tri2 + 2));
            EXPECT_EQ(toLinearIndex(x, y + 1, width, height), indices->get(6 * tri2 + 4));

            // neighbors
            EXPECT_EQ(toLinearIndex(x + 1, y + 1, width, height), indices->get(6 * tri2));
            if (x < width - 1)
                EXPECT_EQ(toLinearIndex(x + 2, y, width, height), indices->get(6 * tri2 + 1));
            else
                EXPECT_EQ(toLinearIndex(x, y + 1, width, height), indices->get(6 * tri2 + 1));
            EXPECT_EQ(toLinearIndex(x + 1, y, width, height), indices->get(6 * tri2 + 2));

            EXPECT_EQ(toLinearIndex(x + 1, y, width, height), indices->get(6 * tri2 + 2));
            EXPECT_EQ(toLinearIndex(x, y, width, height), indices->get(6 * tri2 + 3));
            EXPECT_EQ(toLinearIndex(x, y + 1, width, height), indices->get(6 * tri2 + 4));

            EXPECT_EQ(toLinearIndex(x, y + 1, width, height), indices->get(6 * tri2 + 4));
            if (y < height - 1)
                EXPECT_EQ(toLinearIndex(x, y + 2, width, height), indices->get(6 * tri2 + 5));
            else
                EXPECT_EQ(toLinearIndex(x + 1, y, width, height), indices->get(6 * tri2 + 5));
            EXPECT_EQ(toLinearIndex(x + 1, y + 1, width, height), indices->get(6 * tri2 + 0));
        }
    }
    ASSERT_TRUE(true);
}

}  // namespace inviwo
