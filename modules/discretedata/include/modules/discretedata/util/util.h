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
#include <inviwo/core/util/glm.h>

#include <bitset>

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

struct GetMinMaxDispatcher {
    template <typename T, ind N>
    std::pair<dvec4, dvec4> operator()(const DataChannel<T, N>* channel) {
        glm::vec<N, T> min, max;
        channel->getMinMax(min, max);

        std::pair<dvec4, dvec4> minMax;
        minMax.first = util::glm_convert<dvec4>(min);
        minMax.second = util::glm_convert<dvec4>(max);
        return minMax;
    }
};

inline std::pair<dvec4, dvec4> getMinMax(const Channel* channel) {

    GetMinMaxDispatcher dispatcher;
    return channel->dispatch<std::pair<dvec4, dvec4>>(dispatcher);
}

/** Get the initial array for choosing Choose many elements (the first Choose numbers). **/
template <size_t Choose>
static typename std::array<size_t, Choose> initNchooseK() {
    typename std::array<size_t, Choose> init;
    std::iota(init.begin(), init.end(), 0);
    return init;
}

/** Given a previous chosen selection, return the next one in the sequence. Returns false if the end
 * is reached. **/
template <size_t Choose>
static constexpr bool nextNchooseK(size_t from, typename std::array<size_t, Choose>& choice) {
    for (size_t k = 1; k <= Choose; ++k) {
        if (choice[Choose - k] != from - k) {
            choice[Choose - k]++;
            ind valE = choice[from - k];
            for (size_t afterC = from - k + 1; afterC < Choose; ++afterC) choice[afterC] = ++valE;
            return true;
        }
    }
    return false;
}

template <size_t N>
static constexpr bool nextBitset(std::bitset<N>& bitset) {
    for (size_t n = 0; n < N; ++n) {
        if ((bitset[n] = bitset[n] ^ 0x1) == 0x1) {
            return true;
        }
    }
    return false;
}

/** Get an array containing the Binomial coefficients for N elements. **/
template <size_t N>
static constexpr typename std::array<size_t, N> binomialCoefficients() {
    typename std::array<size_t, N> coeffs;
    if (N == 0) return coeffs;

    coeffs[0] = 1;
    for (size_t c = 1; c < N; ++c) {
        coeffs[c] = (coeffs[c - 1] * (N + 1 - c)) / c;
    }
    return coeffs;
}

/** Get an array containing offsets which have binomial coefficients steps. **/
template <size_t N>
static constexpr typename std::array<size_t, N + 1> binomialCoefficientOffsets() {
    typename std::array<size_t, N + 1> coeffs{};
    coeffs[0] = 0;
    size_t prevSum = 0;

    size_t lastCoeff = 1;
    for (size_t c = 1; c <= N; ++c) {
        coeffs[c] = prevSum + lastCoeff;
        prevSum += lastCoeff;
        lastCoeff = (lastCoeff * (N + 1 - c)) / c;
    }
    return coeffs;
}

/** From a list of sorted indices, fill a bitset where exactly those indices are set. **/
template <size_t N, size_t K>
static constexpr typename std::bitset<N> indicesToBitset(const std::array<size_t, K>& indices) {
    std::bitset<N> bitset;
    auto indexIt = indices.begin();
    for (size_t n = 0; n < N; ++n) {
        if (indexIt != indices.end() && *indexIt == n) {
            bitset[n] = true;
            indexIt++;
        }
    }
    ivwAssert(indexIt == indices.end(), "Indices were not unique and sorted.");
    return bitset;
}

}  // namespace dd_util
}  // namespace discretedata
}  // namespace inviwo
