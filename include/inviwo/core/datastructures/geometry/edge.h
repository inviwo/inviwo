/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2018 Inviwo Foundation
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

#ifndef IVW_EDGE_H
#define IVW_EDGE_H

namespace inviwo {

template <typename T>
class Edge {
public:
    T v1, v2;

    Edge(T in1) : v1(in1), v2(in1){};

    Edge(T in1, T in2) : v1(in1), v2(in2){};

    bool operator==(const Edge<T>& e) const {
        return ((this->v1 == e.v1) && (this->v2 == e.v2)) ||
               ((this->v1 == e.v2) && (this->v2 == e.v1));
    }

protected:
    // Protected as it can't instantiate v1 and v2;
    Edge() {}
};

template <typename T>
class EdgeDataFormat : public Edge<T> {
public:
    EdgeDataFormat() : Edge<T>() {
        this->v1 = util::glm_convert<T>(0.0);
        this->v2 = this->v1;
    }

    EdgeDataFormat(T in1) : Edge<T>(in1){};
    EdgeDataFormat(T in1, T in2) : Edge<T>(in1, in2){};
};

typedef EdgeDataFormat<DataUInt32::type> EdgeIndex;
typedef EdgeDataFormat<DataVec2Float32::type> Edge2D;
typedef EdgeDataFormat<DataVec3Float32::type> Edge3D;

}  // namespace inviwo

#endif  // IVW_EDGE_H