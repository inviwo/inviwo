/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2023 Inviwo Foundation
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

#include <modules/meshrenderinggl/meshrenderingglmoduledefine.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <memory>

namespace inviwo {

namespace meshutil {
/**
 * \brief The weighting modes for calculating normals
 */
enum class CalculateMeshNormalsMode {
    /**
     * \brief Pass through, mesh is not changed
     */
    PassThrough,
    /**
     * \brief no weighting of the normals, simple average
     */
    NoWeighting,
    /**
     * \brief Weight = area of the triangle
     */
    WeightArea,
    /**
     * \brief Weight based on the angle.
     * As defined in "Computing vertex normals from polygonal facets" by Grit Thürmer and
     * Charles A. Wüthrich 1998.
     */
    WeightAngle,
    /**
     * \brief Based on "Weights for Computing Vertex Normals from Facet Normals", N. Max, 1999.
     * This gives the best results in most cases.
     */
    WeightNMax
};

IVW_MODULE_MESHRENDERINGGL_API void calculateMeshNormals(
    Mesh& mesh, CalculateMeshNormalsMode mode = CalculateMeshNormalsMode::WeightNMax);

inline std::unique_ptr<Mesh> calculateMeshNormals(
    const Mesh& mesh, CalculateMeshNormalsMode mode = CalculateMeshNormalsMode::WeightNMax) {
    auto cloned = std::unique_ptr<Mesh>(mesh.clone());
    calculateMeshNormals(*cloned, mode);
    return cloned;
}

}  // namespace meshutil

}  // namespace inviwo
