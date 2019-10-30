/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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

#ifndef IVW_CALCNORMALS_H
#define IVW_CALCNORMALS_H

#include <modules/fancymeshrenderer/fancymeshrenderermoduledefine.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>
#include <memory>

namespace inviwo {
class Mesh;

/**
 * \class CalcNormals
 * \brief VERY_BRIEFLY_DESCRIBE_THE_CLASS
 * DESCRIBE_THE_CLASS_FROM_A_DEVELOPER_PERSPECTIVE
 */
class IVW_MODULE_FANCYMESHRENDERER_API CalcNormals {
public:
    CalcNormals() {}
    virtual ~CalcNormals() = default;

    /**
     * \brief The weighting modes
     */
    enum class Mode {
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

    /**
     * \brief Computes vertex normals for the specified mesh.
     * The mesh is always cloned, even if Mode::PassThrough is chosen.
     * TODO: only supports triangle meshes with triangle lists (connectivity=None)
     * \param input the mesh to process
     * \param mode the computation mode
     * \return the new mesh with new normals
     */
    Mesh* processMesh(const Mesh* const input, Mode mode);

    /**
     * \brief The "best" computation algorithm, as shown in some examples.
     * Might be used as an initial value to the algorithm selection
     * \return the preferred mode
     */
    static Mode preferredMode() { return Mode::WeightNMax; }
};

}  // namespace inviwo

#endif  // IVW_CALCNORMALS_H
