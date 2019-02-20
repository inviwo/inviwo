/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#include <modules/discretedata/dataset.h>
#include <modules/discretedata/channels/bufferchannel.h>
#include <modules/discretedata/channels/analyticchannel.h>
#include <modules/discretedata/connectivity/connectivity.h>
#include <modules/discretedata/connectivity/elementiterator.h>
#include <modules/discretedata/connectivity/connectioniterator.h>
#include <modules/discretedata/connectivity/structuredgrid.h>

namespace inviwo {
namespace discretedata {

TEST(AccessingData, Connectivity) {
    std::vector<ind> size = {4, 5, 6};
    auto grid = std::make_shared<StructuredGrid>(GridPrimitive::Volume, size);
    DataSet data(grid);  // Assume there is data.

    bool allFine = true;
    std::vector<ind> neighbors;

    for (ElementIterator cell : grid->all(GridPrimitive::Volume)) {
        // Now, we basically have a GridPrimitive/index object.
        for (ElementIterator vert : cell.connection(GridPrimitive::Vertex)) {
            // Iterate over all vertices of the respective cell.
            // Basically a fromGridPrimitive/toGridPrimitive/fromIndex/toIndex object.
            // Do something with them.
            ind idx = vert.getIndex();

            // Make sure the gird is bi-directional.
            grid->getConnections(neighbors, idx, vert.getType(), cell.getType());
            if (std::find(neighbors.cbegin(), neighbors.cend(), cell.getIndex()) ==
                neighbors.cend()) {
                EXPECT_TRUE(allFine);
                allFine = false;
                break;
            }
        }
    }

    EXPECT_TRUE(allFine && "Connectivity is not bi-directional.");
}

}  // namespace discretedata
}  // namespace inviwo
