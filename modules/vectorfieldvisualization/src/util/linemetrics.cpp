/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2021 Inviwo Foundation
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

#include <modules/vectorfieldvisualization/util/linemetrics.h>

namespace inviwo {
namespace util {

void addVelocity(IntegralLineSet& lineSet) {
    for (auto& line : lineSet) {
        const auto& posVec = line.getPositions();
        size_t size = posVec.size();
        auto& veloVec = line.getMetaData<dvec3>("velocity", true);
        veloVec.resize(size);
        if (size < 3) continue;

        for (size_t p = 0; p < size; ++p) {
            if (p == 0) {
                veloVec[0] = -posVec[0] * 1.5 + posVec[1] * 2.0 - posVec[2] * 0.5;
            } else if (p == size - 1) {
                veloVec[size - 1] =
                    posVec[size - 3] * 0.5 - posVec[size - 2] * 2.0 + posVec[size - 1] * 1.5;
            } else {
                veloVec[p] = (posVec[p + 1] - posVec[p - 1]) * 0.5;
            }
        }
    }
}
// Either first derivative of velocities
// or second derivative of positions, refer to:
// https://en.wikipedia.org/wiki/Finite_difference_coefficient
bool addAcceleration(IntegralLineSet& lineSet, bool fromVelocity, const std::string& name) {
    for (auto& line : lineSet) {
        if (fromVelocity && !line.hasMetaData("velocity")) {
            return false;
        }
        const auto& posVec = line.getPositions();
        size_t size = posVec.size();
        auto& accVec = line.getMetaData<dvec3>(name, true);
        accVec.resize(size);

        if (fromVelocity) {
            if (size < 3) continue;
            const auto& veloVec = line.getMetaData<dvec3>("velocity");
            for (size_t p = 0; p < size; ++p) {
                if (p == 0) {
                    accVec[0] = -veloVec[0] * 1.5 + veloVec[1] * 2.0 - veloVec[2] * 0.5;
                } else if (p == size - 1) {
                    accVec[size - 1] =
                        veloVec[size - 3] * 0.5 - veloVec[size - 2] * 2.0 + veloVec[size - 1] * 1.5;
                } else {
                    accVec[p] = (veloVec[p + 1] - veloVec[p - 1]) * 0.5;
                }
            }
        } else {
            if (size < 4) continue;
            for (size_t p = 0; p < size; ++p) {
                if (p == 0) {
                    accVec[0] = posVec[0] * 2.0 - posVec[1] * 5.0 + posVec[2] * 4.0 - posVec[3];
                } else if (p == size - 1) {
                    accVec[size - 1] = -posVec[size - 4] + posVec[size - 3] * 4.0 -
                                       posVec[size - 2] * 5.0 + posVec[size - 1] * 2.0;
                } else {
                    accVec[p] = posVec[p - 1] - posVec[p] * 2.0 + posVec[p + 1];
                }
            }
        }
    }
}

}  // namespace util
}  // namespace inviwo