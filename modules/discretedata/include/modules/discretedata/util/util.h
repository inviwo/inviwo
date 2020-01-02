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

#pragma once

#include <modules/discretedata/connectivity/connectivity.h>
#include <modules/discretedata/channels/datachannel.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>

namespace inviwo {
namespace discretedata {
namespace dd_util {

inline double tetrahedronVolume(double corners[4][3]) {
    const glm::dvec3 a(corners[0][0] - corners[1][0], corners[0][1] - corners[1][1],
                       corners[0][2] - corners[1][2]);
    const glm::dvec3 b(corners[0][0] - corners[2][0], corners[0][1] - corners[2][1],
                       corners[0][2] - corners[2][2]);
    const glm::dvec3 c(corners[0][0] - corners[3][0], corners[0][1] - corners[3][1],
                       corners[0][2] - corners[3][2]);

    return fabs(glm::dot(glm::cross(a, b), c) / 6.0);
}

template <typename ToType>
struct ChannelToBufferDispatcher {

    template <typename T, ind N>
    std::shared_ptr<BufferBase> operator()(const DataChannel<T, N>* positions) {
        typedef typename DataChannel<T, N>::DefaultVec DefaultVec;
        ind numElements = positions->size();
        std::vector<DefaultVec> data(numElements);
        positions->fill(*data.data(), 0, numElements);

        std::vector<ToType> convBuffer;
        convBuffer.reserve(data.size());

        for (const DefaultVec& val : data) convBuffer.push_back(util::glm_convert<ToType>(val));

        auto buffer = util::makeBuffer(std::move(convBuffer));
        return buffer;
    }
};

template <typename Type>
struct VectorHash {
    std::size_t operator()(std::vector<Type> const& vec) const {
        std::size_t seed = vec.size();
        for (auto& i : vec) {
            seed ^= static_cast<size_t>(i) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};

template <typename Type>
struct VectorCompare {
    bool operator()(const std::vector<Type>& lhs, const std::vector<Type>& rhs) const {
        if (lhs.size() != rhs.size()) return false;
        for (size_t i = 0; i < lhs.size(); ++i)
            if (lhs[i] != rhs[i]) return false;
        return true;
    }
};

bool nextNchooseK(std::vector<ind>& chosenK, ind N) {
    for (ind e = 1; e <= chosenK.size(); ++e) {
        if (chosenK[N - e] != N - e) {
            chosenK[N - e]++;
            ind valE = chosenK[N - e];
            for (ind afterE = N - e + 1; afterE < N; ++afterE) chosenK[afterE] = ++valE;
            break;
        }
        return false;
    }
    return true;
}

}  // namespace dd_util
}  // namespace discretedata
}  // namespace inviwo
