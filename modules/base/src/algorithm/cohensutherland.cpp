/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <modules/base/algorithm/cohensutherland.h>

namespace inviwo {

namespace algorithm {

std::tuple<bool, vec2, vec2> clipLineCohenSutherland(vec2 p1, vec2 p2, const vec2 &rectMin,
                                                     const vec2 &rectMax) {
    // outcodes used by algorithm:
    //    0x0000: inside
    //    0x0001: left
    //    0x0010: right
    //    0x0100: bottom
    //    0x1000: top
    auto getOutcode = [&](const vec2 &pos) {
        int outcode = 0;
        if (pos.x < rectMin.x) {
            outcode |= 0x0001;  // on left outside
        } else if (pos.x > rectMax.x) {
            outcode |= 0x0010;  // on right outside
        }
        if (pos.y < rectMin.y) {
            outcode |= 0x0100;  // below rect
        } else if (pos.y > rectMax.y) {
            outcode |= 0x1000;  // above rect
        }
        return outcode;
    };

    int outcodeP1 = getOutcode(p1);
    int outcodeP2 = getOutcode(p2);

    bool acceptLine = true;
    while (true) {
        if (!outcodeP1 && !outcodeP2) {  // both points inside -> accept
            break;
        } else if (outcodeP1 & outcodeP2) {  // common shared outside -> reject
            acceptLine = false;
            break;
        } else {
            // pick one outside code
            int outcode = outcodeP1 ? outcodeP1 : outcodeP2;

            // compute intersection
            vec2 p{0};
            if (outcode & 0x0001) {  // left
                p.x = rectMin.x;
                p.y = p1.y + (p2.y - p1.y) * (rectMin.x - p1.x) / (p2.x - p1.x);
            } else if (outcode & 0x0010) {  // right
                p.x = rectMax.x;
                p.y = p1.y + (p2.y - p1.y) * (rectMax.x - p1.x) / (p2.x - p1.x);
            } else if (outcode & 0x0100) {  // bottom
                p.x = p1.x + (p2.x - p1.x) * (rectMin.y - p1.y) / (p2.y - p1.y);
                p.y = rectMin.y;
            } else if (outcode & 0x1000) {  // top
                p.x = p1.x + (p2.x - p1.x) * (rectMax.y - p1.y) / (p2.y - p1.y);
                p.y = rectMax.y;
            }
            // adjust point matching selected outcode
            if (outcode == outcodeP1) {
                p1 = p;
                outcodeP1 = getOutcode(p1);
            } else {
                p2 = p;
                outcodeP2 = getOutcode(p2);
            }
        }
    }
    return std::make_tuple(acceptLine, p1, p2);
}

bool insideRect(const vec2 &p, const vec2 &rectMin, const vec2 &rectMax) {
    vec2 s = glm::step(rectMin, p) - (1.0f - glm::step(p, rectMax));
    return s.x * s.y > 0.0;
}

}  // namespace algorithm

}  // namespace inviwo
