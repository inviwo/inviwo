/*********************************************************************************
*
* Inviwo - Interactive Visualization Workshop
*
* Copyright (c) 2012-2018 Inviwo Foundation
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

#include "discretedata/connectivity/connectivity.h"

namespace inviwo {
namespace dd {
namespace util {

/** See std::hash_combine implementation from Boost
*/
template <class T>
inline void CombineHash(std::size_t& hash, const T& value) {
    std::hash<T> hasher;
    hash ^= hasher(value) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
}

/** \class PairHash
*   \brief Functor for pair hashing.
*/
template<typename S, typename T>
struct PairHash {
    inline size_t operator()(const std::pair<S, T> & v) const {
        size_t hash = 0;
        CombineHash(hash, v.first);
        CombineHash(hash, v.second);
        return hash;
    }
};

inline double tetrahedronVolume(double corners[4][3]) {
    const glm::dvec3 a(corners[0][0] - corners[1][0], corners[0][1] - corners[1][1], corners[0][2] - corners[1][2]);
    const glm::dvec3 b(corners[0][0] - corners[2][0], corners[0][1] - corners[2][1], corners[0][2] - corners[2][2]);
    const glm::dvec3 c(corners[0][0] - corners[3][0], corners[0][1] - corners[3][1], corners[0][2] - corners[3][2]);

    return fabs(glm::dot(glm::cross(a, b), c) / 6.0);
}

} // namespace
}
}
