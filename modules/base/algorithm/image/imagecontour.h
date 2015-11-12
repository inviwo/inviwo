/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#ifndef IVW_IMAGECONTOUR_H
#define IVW_IMAGECONTOUR_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>
#include <inviwo/core/datastructures/image/layerrepresentation.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>
#include <inviwo/core/util/indexmapper.h>
#include <inviwo/core/util/interpolation.h>

namespace inviwo {

/**
 * \class ImageContour
 * \brief VERY_BRIEFLY_DESCRIBE_THE_CLASS
 * DESCRIBE_THE_CLASS
 */
class IVW_MODULE_BASE_API ImageContour {
public:
    static std::shared_ptr<Mesh> apply(const LayerRepresentation* in, double isoValue,
                                       vec4 color = vec4(1.0));
};

namespace detail {
struct IVW_MODULE_BASE_API ImageContourDispatcher {
    using type = std::shared_ptr<Mesh>;
    template <class T>
    std::shared_ptr<Mesh> dispatch(const LayerRepresentation* in, double isoValue, vec4 color);
};

template <class DataType>
std::shared_ptr<Mesh> ImageContourDispatcher::dispatch(const LayerRepresentation* in,
                                                       double isoValue, vec4 color) {
    static const std::vector<std::vector<int>> caseTable = {
        std::vector<int>(),                          // case 0
        std::vector<int>({0, 1, 0, 3}),              // case 1
        std::vector<int>({0, 1, 1, 2}),              // case 2
        std::vector<int>({0, 3, 1, 2}),              // case 3
        std::vector<int>({2, 3, 2, 1}),              // case 4
        std::vector<int>({0, 3, 3, 2, 2, 1, 1, 0}),  // case 5
        std::vector<int>({0, 1, 2, 3}),              // case 6
        std::vector<int>({0, 3, 3, 2}),              // case 7
        std::vector<int>({0, 3, 0, 1, 1, 2, 2, 3})};

    auto mesh = std::make_shared<BasicMesh>();
    auto indices = mesh->addIndexBuffer(DrawType::LINES, ConnectivityType::NONE);

    using T = typename DataType::type;
    using P = typename DataType::primitive;
    using D = typename util::same_extent<P, double>::type;

    const LayerRAMPrecision<T>* ram = dynamic_cast<const LayerRAMPrecision<T>*>(in);
    if (!ram) return nullptr;

    auto data = static_cast<const T*>(ram->getData());

    auto dim = ram->getDimensions();

    if (dim.x == 0 || dim.y == 0) return nullptr;

    D vals[4];
    vec3 outPos[4];
    vec3 outPosScale =
        vec3(1.0f / static_cast<float>(dim.x - 1), 1.0f / static_cast<float>(dim.y - 1), 1);
    util::IndexMapper2D index(dim);
    for (size_t y = 0; y < dim.y - 1; y++) {
        for (size_t x = 0; x < dim.y - 1; x++) {
            auto idx = index(x, y);
            vals[0] = static_cast<D>(util::glm_convert<P, T>(data[idx]));
            vals[1] = static_cast<D>(util::glm_convert<P, T>(data[idx + 1]));
            vals[2] = static_cast<D>(util::glm_convert<P, T>(data[idx + 1 + dim.x]));
            vals[3] = static_cast<D>(util::glm_convert<P, T>(data[idx + dim.x]));

            int theCase = 0;
            theCase += vals[0] < isoValue ? 0 : 1;
            theCase += vals[1] < isoValue ? 0 : 2;
            theCase += vals[2] < isoValue ? 0 : 4;
            theCase += vals[3] < isoValue ? 0 : 8;

            if (theCase == 0 || theCase == 15) {
                continue;
            } else if (theCase == 5 || theCase == 10) {
                auto m = (vals[0] + vals[1] + vals[2] + vals[3]) * 0.25;
                bool inside = m >= isoValue;
                if (theCase == 5) {
                    theCase = inside ? 5 : 8;
                } else {
                    theCase = !inside ? 5 : 8;
                }
            } else if (theCase > 7) {
                theCase = 15 - theCase;
            }

            outPos[0] = vec3(x, y, 1) * outPosScale;
            outPos[1] = vec3(x + 1, y, 1) * outPosScale;
            outPos[2] = vec3(x + 1, y + 1, 1) * outPosScale;
            outPos[3] = vec3(x, y + 1, 1) * outPosScale;

            auto& edges = caseTable[theCase];
            for (size_t i = 0; i < edges.size(); i += 2) {
                auto t = (isoValue - vals[edges[i]]) / (vals[edges[i + 1]] - vals[edges[i]]);
                auto p = Interpolation<vec3, float>::linear(outPos[edges[i]] , 
                                                            outPos[edges[i + 1]] ,
                                                            static_cast<float>(t));
                indices->add(mesh->addVertex(p, p, p, color));
            }
        }
    }

    return mesh;
}
}

}  // namespace

#endif  // IVW_IMAGECONTOUR_H
