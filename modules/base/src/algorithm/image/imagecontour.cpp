/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2024 Inviwo Foundation
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

#include <modules/base/algorithm/image/imagecontour.h>

#include <inviwo/core/datastructures/geometry/geometrytype.h>           // for ConnectivityType
#include <inviwo/core/datastructures/geometry/typedmesh.h>              // for BasicMesh, TypedMesh
#include <inviwo/core/datastructures/image/layerram.h>                  // IWYU pragma: keep
#include <inviwo/core/datastructures/image/layerrepresentation.h>       // for LayerRepresentation
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/util/formatdispatching.h>                         // for dispatch, All
#include <inviwo/core/util/formats.h>                                   // for DataFormatBase
#include <inviwo/core/util/glmcomp.h>                                   // for glmcomp
#include <inviwo/core/util/glmconvert.h>                                // for glm_convert
#include <inviwo/core/util/glmutils.h>                                  // for extent
#include <inviwo/core/util/glmvec.h>                                    // for vec3, vec4
#include <inviwo/core/util/indexmapper.h>                               // for IndexMapper, Inde...
#include <inviwo/core/util/interpolation.h>                             // for Interpolation

#include <algorithm>      // for min
#include <type_traits>    // for remove_extent_t
#include <unordered_map>  // for unordered_map
#include <unordered_set>  // for unordered_set
#include <vector>         // for vector

#include <glm/common.hpp>        // for mix
#include <glm/detail/setup.hpp>  // for size_t
#include <glm/vec3.hpp>          // for operator*, operator+

namespace inviwo {
class Mesh;

namespace {

constexpr auto dispatcher = []<typename T>(const LayerRepresentation* in, size_t channel,
                                           double isoValue, vec4 color) -> std::shared_ptr<Mesh> {
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
    auto indices = mesh->addIndexBuffer(DrawType::Lines, ConnectivityType::None);

    channel = std::min(channel, util::extent<T>::value - 1);

    const LayerRAMPrecision<T>* ram = dynamic_cast<const LayerRAMPrecision<T>*>(in);
    if (!ram) return nullptr;

    auto data = static_cast<const T*>(ram->getData());

    auto dim = ram->getDimensions();

    if (dim.x == 0 || dim.y == 0) return nullptr;

    double vals[4];
    vec3 outPos[4];
    vec3 outPosScale =
        vec3(1.0f / static_cast<float>(dim.x - 1), 1.0f / static_cast<float>(dim.y - 1), 1);
    util::IndexMapper2D index(dim);
    for (size_t y = 0; y < dim.y - 1; y++) {
        for (size_t x = 0; x < dim.x - 1; x++) {
            auto idx = index(x, y);
            vals[0] = util::glm_convert<double>(util::glmcomp(data[idx], channel));
            vals[1] = util::glm_convert<double>(util::glmcomp(data[idx + 1], channel));
            vals[2] = util::glm_convert<double>(util::glmcomp(data[idx + 1 + dim.x], channel));
            vals[3] = util::glm_convert<double>(util::glmcomp(data[idx + dim.x], channel));

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

            outPos[0] = vec3(x, y, 0) * outPosScale;
            outPos[1] = vec3(x + 1, y, 0) * outPosScale;
            outPos[2] = vec3(x + 1, y + 1, 0) * outPosScale;
            outPos[3] = vec3(x, y + 1, 0) * outPosScale;

            auto& edges = caseTable[theCase];
            for (size_t i = 0; i < edges.size(); i += 2) {
                auto t = (isoValue - vals[edges[i]]) / (vals[edges[i + 1]] - vals[edges[i]]);
                auto p = Interpolation<vec3, float>::linear(outPos[edges[i]], outPos[edges[i + 1]],
                                                            static_cast<float>(t));
                indices->add(mesh->addVertex(p, p, p, color));
            }
        }
    }

    return mesh;
};

}  // namespace

std::shared_ptr<Mesh> ImageContour::apply(const LayerRepresentation* in, size_t channel,
                                          double isoValue, vec4 color) {
    auto df = in->getDataFormat();
    if (df->getNumericType() != NumericType::Float) {
        isoValue = df->getMin() + isoValue * (df->getMax() - df->getMin());
    }
    return dispatching::singleDispatch<std::shared_ptr<Mesh>, dispatching::filter::All>(
        df->getId(), dispatcher, in, channel, isoValue, color);
}

}  // namespace inviwo
