/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023-2025 Inviwo Foundation
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

#include <inviwo/tetramesh/tetrameshmoduledefine.h>
#include <inviwo/tetramesh/datastructures/tetramesh.h>

namespace inviwo {

class Volume;

/**
 * \ingroup datastructures
 * \brief Data required for rendering an Inviwo Volume as tetrahedral mesh
 *
 * Provides an interface between a Volume and the data structures required for rendering a
 * tetrahedral mesh. Six tetrahedra are created in between each four voxels of the volume to convert
 * the cell-centered data of the uniform grid of the Volume to node-centered values.
 *
 * The extent of the TetraMesh will be smaller than the extent of the Volume by half a voxel in each
 * dimension.
 */
class IVW_MODULE_TETRAMESH_API VolumeTetraMesh : public TetraMesh {
public:
    VolumeTetraMesh(const std::shared_ptr<const Volume>& volume, int channel = 0);
    VolumeTetraMesh* clone() const override;
    virtual ~VolumeTetraMesh() = default;

    /**
     * Use \p volume as source for the tetrahedralization into a TetraMesh.
     *
     * @param volume    input volume to be converted
     * @param channel   volume channel used as scalar values
     * @throws Exception if one of the \p volume dimensions is less than 2
     */
    void setData(const std::shared_ptr<const Volume>& volume, int channel = 0);

    virtual int getNumberOfCells() const override;
    virtual int getNumberOfPoints() const override;

    /**
     * @copydoc TetraMesh::get
     */
    virtual void get(std::vector<vec4>& nodes, std::vector<ivec4>& nodeIds) const override;

    /**
     * @copydoc TetraMesh::getBoundingBox
     */
    virtual mat4 getBoundingBox() const override;

    /**
     * @copydoc TetraMesh::getDataRange
     */
    virtual dvec2 getDataRange() const override;

    /**
     * @copydoc SpatialEntity::getAxis
     */
    virtual const Axis* getAxis(size_t index) const override;

private:
    std::shared_ptr<const Volume> volume_;
    int channel_;
};

}  // namespace inviwo
