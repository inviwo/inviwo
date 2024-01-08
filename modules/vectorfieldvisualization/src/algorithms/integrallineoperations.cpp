/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2024 Inviwo Foundation
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

#include <modules/vectorfieldvisualization/algorithms/integrallineoperations.h>

#include <inviwo/core/datastructures/buffer/buffer.h>                         // for Buffer
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>             // for BufferRAMPr...
#include <inviwo/core/datastructures/representationconverter.h>               // for Representat...
#include <inviwo/core/datastructures/representationconverterfactory.h>        // for Representat...
#include <inviwo/core/util/glmmat.h>                                          // for dmat4
#include <inviwo/core/util/glmvec.h>                                          // for dvec3, dvec4
#include <inviwo/core/util/logcentral.h>                                      // for LogCentral
#include <modules/vectorfieldvisualization/datastructures/integralline.h>     // for IntegralLine
#include <modules/vectorfieldvisualization/datastructures/integrallineset.h>  // for IntegralLin...

#include <algorithm>      // for transform
#include <cmath>          // for acos
#include <memory>         // for unique_ptr
#include <sstream>        // for basic_strin...
#include <type_traits>    // for remove_exte...
#include <unordered_map>  // for unordered_map
#include <unordered_set>  // for unordered_set
#include <vector>         // for vector

#include <glm/geometric.hpp>  // for distance
#include <glm/mat4x4.hpp>     // for operator*, mat
#include <glm/vec3.hpp>       // for operator*
#include <glm/vec4.hpp>       // for operator*

namespace inviwo {
namespace util {
IntegralLine curvature(const IntegralLine& line, dmat4 toWorld) {
    IntegralLine copy(line);
    curvature(copy, toWorld);
    return copy;
}
IntegralLineSet curvature(const IntegralLineSet& lines) {
    IntegralLineSet copy(lines);
    curvature(copy);
    return copy;
}

void curvature(IntegralLine& line, dmat4 toWorld) {
    if (line.hasMetaData("curvature")) return;
    if (line.getPositions().size() <= 1) return;
    auto positions = line.getPositions();  // note, this creates a copy, we modify it below

    std::transform(positions.begin(), positions.end(), positions.begin(), [&](dvec3 pos) {
        dvec4 P = toWorld * dvec4(pos, 1);
        return dvec3(P) / P.w;
    });

    auto md = line.createMetaData<double>("curvature");
    auto& K = md->getEditableRAMRepresentation()->getDataContainer();
    const auto& V = line.getMetaData<dvec3>("velocity");

    auto cur = positions.begin();
    auto vel = V.begin();

    for (; cur != positions.end() && vel != V.end(); ++cur, ++vel) {
        if (cur == positions.begin()) {
            // first
            K.emplace_back(0);
        } else if (cur == positions.end() - 1) {
            // last, copy second to last
            K.emplace_back(K.back());
        } else {
            const auto p = *cur;
            const auto pm = *(cur - 1);
            const auto pp = *(cur + 1);

            const auto t1 = pm - p;
            const auto t2 = p - pp;

            auto l1 = glm::length(t1);
            auto l2 = glm::length(t2);
            if (l1 == 0 || l2 == 0) {
                K.emplace_back(0);
                LogWarnCustom("util::curvature", "Got zero offset");
                continue;
            }
            const auto nt1 = t1 / l1;  // normalize t1
            const auto nt2 = t2 / l2;  // normalize t2
            const auto dot = glm::dot(nt1, nt2);
            const auto cdot = dot < -1.0 ? -1.0 : (dot > 1.0 ? 1.0 : dot);
            const auto angle = std::acos(cdot);

            const double meanL = 0.5 * (l1 + l2);
            K.emplace_back(angle / meanL);
        }
    }
    K[0] = K[1];  // Copy second to first
}
void curvature(IntegralLineSet& lines) {
    for (auto& line : lines) {
        curvature(line, dmat4(lines.getModelMatrix()));
    }
}

IntegralLine tortuosity(const IntegralLine& line, dmat4 toWorld) {
    IntegralLine copy(line);
    tortuosity(copy, toWorld);
    return copy;
}
IntegralLineSet tortuosity(const IntegralLineSet& lines) {
    IntegralLineSet copy(lines);
    tortuosity(copy);
    return copy;
}

void tortuosity(IntegralLine& line, dmat4 toWorld) {
    if (line.hasMetaData("tortuosity")) return;
    auto positions = line.getPositions();  // note, this creates a copy, we modify it below
    if (positions.size() <= 1) return;

    std::transform(positions.begin(), positions.end(), positions.begin(), [&](dvec3 pos) {
        dvec4 P = toWorld * dvec4(pos, 1);
        return dvec3(P) / P.w;
    });

    auto md = line.createMetaData<double>("tortuosity");
    auto& K = md->getEditableRAMRepresentation()->getDataContainer();
    double acuDist = 0;

    dvec3 start = positions.front();
    dvec3 prev = start;

    for (auto& p : positions) {
        auto div = [](auto a, auto b) {
            if (b == 0) return 1.0;
            return a / b;
        };

        acuDist += glm::distance(prev, p);
        prev = p;
        K.emplace_back(div(acuDist, glm::distance(start, p)));
    }
}
void tortuosity(IntegralLineSet& lines) {
    for (auto& line : lines) {
        tortuosity(line, dmat4(lines.getModelMatrix()));
    }
}

}  // namespace util
}  // namespace inviwo
