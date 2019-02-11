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

#ifndef IVW_MESHAXISALIGNEDBOUNDINGBOX_H
#define IVW_MESHAXISALIGNEDBOUNDINGBOX_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/datastructures/geometry/mesh.h>

namespace inviwo {

namespace meshutil {
/**
 * \brief Compute bounding box of meshes aligned to principal axes of world space.
 *
 * @param meshes Meshes to consider
 * @return Minimum and maximum vertex points of meshes in (x,y,z) world space, or zero if vector is
 * empty.
 */
IVW_MODULE_BASE_API std::pair<vec3, vec3> axisAlignedBoundingBox(
    const std::vector<std::shared_ptr<const Mesh>>& meshes);

/**
 * \brief Compute bounding box of mesh aligned to principal axes of world space.
 * Returns mesh offset in both min/max if the mesh does not have any vertices.
 * @param mesh Mesh to consider
 * @return Minimum and maximum vertex points of mesh in (x,y,z), or mesh offset if no vertices
 * exist.
 */
IVW_MODULE_BASE_API std::pair<vec3, vec3> axisAlignedBoundingBox(const Mesh& mesh);

}  // namespace meshutil

}  // namespace inviwo

#endif  // IVW_MESHAXISALIGNEDBOUNDINGBOX_H
