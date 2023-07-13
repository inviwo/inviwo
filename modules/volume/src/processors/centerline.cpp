/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <inviwo/volume/processors/centerline.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/util/glm.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo CenterLine::processorInfo_{
    "org.inviwo.CenterLine",  // Class identifier
    "Center Line",            // Display name
    "Undefined",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};
const ProcessorInfo CenterLine::getProcessorInfo() const { return processorInfo_; }

CenterLine::CenterLine()
    : Processor()
    , volume_("volume")
    , distanceField_("distanceField")
    , lines_("centerLines")
    , points_("extrumumPoints")
    , threshold_("threshold", "Threshold")
    , method_("method", "Method",
              {
                  {"DistanceField", "DistanceField", Method::DistanceField},
                  {"Topology", "Topology", Method::Topology},
              },
              0, InvalidationLevel::InvalidResources) {

    addPorts(volume_, distanceField_, lines_, points_);
    addProperties(threshold_, method_);
}

void CenterLine::process() {
    auto lineMesh = std::make_shared<ColoredMesh>();
    lineMesh->setModelMatrix(volume_.getData()->getModelMatrix());
    lineMesh->setWorldMatrix(volume_.getData()->getWorldMatrix());
    auto lineIdcs = lineMesh->addIndexBuffer(DrawType::Lines, ConnectivityType::None);
    std::vector<ColoredMesh::Vertex> lineVerts;

    auto pointMesh = std::make_shared<ColoredMesh>(DrawType::Points, ConnectivityType::None);
    pointMesh->setModelMatrix(volume_.getData()->getModelMatrix());
    pointMesh->setWorldMatrix(volume_.getData()->getWorldMatrix());
    std::vector<ColoredMesh::Vertex> pointVerts;

    // indexBuffer->add(static_cast<std::uint32_t>(vertices.size()));

    auto volumeRam =
        volume_.getData()->getRepresentation<VolumeRAM>()->dispatch<std::shared_ptr<VolumeRAM>>(
            [&, threshold = threshold_.get(), method = method_.get()](auto vr) {
                using ValueType = util::PrecisionValueType<decltype(vr)>;

                const auto inputField = vr->getDataTyped();
                const auto dim = vr->getDimensions();
                const int size = glm::compMul(dim);

                auto distanceRam = std::make_shared<VolumeRAMPrecision<float>>(
                    dim, vr->getSwizzleMask(), vr->getInterpolation(), vr->getWrapping());
                auto distanceField = distanceRam->getDataTyped();

                for (size_t f = 0; f < size; ++f) {
                    distanceField[f] = util::glm_convert<float>(inputField[f]);
                }

                auto doForXYZ = [&](auto func) {
                    size3_t index;
                    for (index.z = 0; index.z < dim.z; ++index.z)
                        for (index.y = 0; index.y < dim.y; ++index.y)
                            for (index.x = 0; index.x < dim.x; ++index.x) {
                                func(index);
                            }
                };

                auto getNeighborsXY = [&](std::vector<size_t>& result, const size3_t& index,
                                          size_t linIdx) {
                    result.clear();
                    if (index.x >= 1) result.push_back(linIdx - 1);
                    if (index.x + 1 < dim.x) result.push_back(linIdx + 1);
                    if (index.y >= 1) result.push_back(linIdx - dim.x);
                    if (index.y + 1 < dim.y) result.push_back(linIdx + dim.x);
                };

                auto getLinIdx = [&](const size3_t& index) -> size_t {
                    return index.x + index.y * dim.x + index.z * dim.x * dim.y;
                };

                std::vector<size_t> neighbors;

                // Fill volume depending on method selected.
                switch (method) {
                    // ============== Distance Field ==============
                    case Method::DistanceField: {
                        std::cout << "Distance Field" << std::endl;

                        for (size_t f = 0; f < size; ++f) {
                            if (distanceField[f] < threshold)
                                distanceField[f] = std::min(-1.0f, threshold);
                        }

                        int step = 0;
                        bool changedField = true;
                        while (changedField) {
                            changedField = false;
                            doForXYZ([&](const size3_t& index) {
                                size_t linIdx = getLinIdx(index);
                                if (distanceField[linIdx] != -1) return;

                                getNeighborsXY(neighbors, index, linIdx);
                                for (size_t neigh : neighbors) {
                                    if (distanceField[neigh] == step) {
                                        distanceField[linIdx] = step + 1;
                                        changedField = true;
                                        break;
                                    }
                                }
                            });
                            step++;
                        }
                        std::cout << "step: " << step << std::endl;
                    } break;

                    case Method::Topology:
                        // ============== Topology ==============
                        {
                            std::cout << "Topology" << std::endl;
                            for (size_t f = 0; f < size; ++f) {
                                if (distanceField[f] < threshold)
                                    distanceField[f] = std::min(-1.0f, threshold);
                            }

                            doForXYZ([&](const size3_t& index) {
                                size_t linIdx = getLinIdx(index);
                                if (distanceField[linIdx] < threshold) return;

                                getNeighborsXY(neighbors, index, linIdx);
                                bool isMax = true;
                                for (size_t neigh : neighbors) {
                                    if (distanceField[neigh] > distanceField[linIdx]) {
                                        isMax = false;
                                        break;
                                    }
                                }

                                if (isMax) {
                                    pointVerts.push_back(
                                        {{(0.5 + index.x) / dim.x, (0.5 + index.y) / dim.y,
                                          (0.5 + index.z) / dim.z},
                                         {1, 0, 0, 1}});
                                }
                            });
                        }
                        break;

                    case Method::AStar:
                    default:
                        break;
                }

                return distanceRam;
            });

    auto vol = std::make_shared<Volume>(volumeRam);
    vol->copyMetaDataFrom(*(volume_.getData()));
    vol->dataMap_ = volume_.getData()->dataMap_;

    vol->setModelMatrix(volume_.getData()->getModelMatrix());
    vol->setWorldMatrix(volume_.getData()->getWorldMatrix());

    distanceField_.setData(vol);
    lineMesh->addVertices(lineVerts);
    pointMesh->addVertices(pointVerts);
    lines_.setData(lineMesh);
    points_.setData(pointMesh);
}

}  // namespace inviwo
