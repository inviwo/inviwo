/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2017 Inviwo Foundation
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

namespace inviwo {
namespace util {
IntegralLine curvature(const IntegralLine &line, dmat4 toWorld) {
    IntegralLine copy(line);
    curvature(copy, toWorld);
    return copy;
}
IntegralLineSet curvature(const IntegralLineSet &lines) {
    IntegralLineSet copy(lines);
    curvature(copy);
    return copy;
}

void curvature(IntegralLine &line, dmat4 toWorld) {
    if (line.hasMetaData("curvature")) return;
    auto positions = line.getPositions();  // note, this creates a copy, we modify it below
    float dt = positions.size() - 1;
    dt = 1 / dt;
    size_t idx = 0;

    std::transform(positions.begin(), positions.end(), positions.begin(), [&](dvec3 pos) {
        dvec4 P = toWorld * dvec4(pos, 1);
        return dvec3(P) / P.w;
    });

    auto &K = line.createMetaData("curvature");
    auto &V = line.getMetaData("velocity");

    auto cur = positions.begin();
    auto vel = V.begin();

    for (; cur != positions.end() && vel != V.end(); ++cur, ++vel) {
        if (cur == positions.begin()) {
            // first
            K.emplace_back(0);
        } else if (cur == positions.end() - 1) {
            // last
            K.emplace_back(0);
        } else {
            auto v = *vel;
            auto vm = *(vel - 1);
            auto vp = *(vel + 1);

            auto p = *cur;
            auto pm = *(cur - 1);
            auto pp = *(cur + 1);

            auto t1 = pm - p;
            auto t2 = p - pp;
            auto nt1 = glm::normalize(t1);
            auto nt2 = glm::normalize(t2);
            auto angle = std::acos(glm::dot(nt1, nt2));

            double a = std::abs(0.5 * glm::length(pp - p));
            double b = std::abs(0.5 * glm::length(p - pm));

            K.emplace_back(angle / (a + b));
        }
    }
}
void curvature(IntegralLineSet &lines) {
    for (auto &line : lines) {
        curvature(line, dmat4(lines.getModelMatrix()));
    }
}

IntegralLine tortuosity(const IntegralLine &line, dmat4 toWorld) {
    IntegralLine copy(line);
    tortuosity(copy, toWorld);
    return copy;
}
IntegralLineSet tortuosity(const IntegralLineSet &lines) {
    IntegralLineSet copy(lines);
    tortuosity(copy);
    return copy;
}

void tortuosity(IntegralLine &line, dmat4 toWorld) {
    if (line.hasMetaData("tortuosity")) return;
    auto positions = line.getPositions();  // note, this creates a copy, we modify it below
    float dt = positions.size() - 1;
    dt = 1 / dt;
    size_t idx = 0;

    std::transform(positions.begin(), positions.end(), positions.begin(), [&](dvec3 pos) {
        dvec4 P = toWorld * dvec4(pos, 1);
        return dvec3(P) / P.w;
    });

    auto &K = line.createMetaData("tortuosity");
    double acuDist = 0;

    dvec3 start = positions.front();
    dvec3 prev = start;

    for (auto &p : positions) {
        auto div = [](auto a, auto b) {
            if (b == 0) return 1.0;
            return a / b;
        };

        acuDist += glm::distance(prev, p);
        prev = p;
        K.emplace_back(div(acuDist, glm::distance(start, p)));
    }
}
void tortuosity(IntegralLineSet &lines) {
    for (auto &line : lines) {
        tortuosity(line, dmat4(lines.getModelMatrix()));
    }
}

}  // namespace util
}  // namespace inviwo
