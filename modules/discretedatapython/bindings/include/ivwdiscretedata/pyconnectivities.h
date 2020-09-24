/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2020 Inviwo Foundation
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
#pragma once

#include <warn/push>
#include <warn/ignore/shadow>
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <warn/pop>

#include <modules/discretedata/connectivity/structuredgrid.h>

namespace inviwo {
using namespace discretedata;

namespace discretepyutil {
namespace detail {

struct IVW_MODULE_DISCRETEDATA_API CreateStructuredGridDispatcher {

    template <typename Result, int N>
    Result operator()(const pybind11::array_t<int>& vertData) {
        auto ndim = vertData.ndim();
        ivwAssert(ndim == N, "Given data size does not match template dimension.");

        std::array<ind, N> numVertices;
        for (ind n = 0; n < N; ++n) numVertices[n] = static_cast<ind>(*vertData.data(N - 1 - n));
        std::shared_ptr<Connectivity> grid =
            std::make_shared<StructuredGrid<ind(N)>>(std::move(numVertices));
        return grid;
        // std::array<ind, N> numVertices;
        // for (ind n = 0; n < N; ++n) numVertices[n] = vertData.shape(n);
        // // ind product =
        // //     std::accumulate(numVertices.begin(), numVertices.end(), 1,
        // std::multiplies<ind>());

        // auto channel = std::make_shared<NumPyChannel<T, N>>(vertData, name,
        // GridPrimitive::Vertex);

        // // std::array<ind, N> numCellsArray;
        // // for (size_t n = 0; n < N; ++n) {
        // //     numCellsArray[n] = vertData.shape(n);
        // // }
        // auto grid = std::make_shared<StructuredGrid<ind(N)>>(std::move(numVertices));

        // auto dataSet = std::make_shared<DataSet>(grid);
        // dataSet->addChannel(channel);
        // return dataSet;
        // return std::make_shared<DataSet>(grid);
    }
};

}  // namespace detail

std::shared_ptr<Connectivity> createStructuredGrid(const pybind11::array_t<int>& data);
}  // namespace discretepyutil

void exposeConnectivities(pybind11::module& m);

}  // namespace inviwo
