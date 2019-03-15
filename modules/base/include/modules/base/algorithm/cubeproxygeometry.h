/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#ifndef IVW_CUBEPROXYGEOMETRY_H
#define IVW_CUBEPROXYGEOMETRY_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <tuple>

namespace inviwo {

class Volume;
class SimpleMesh;

namespace algorithm {

std::shared_ptr<SimpleMesh> IVW_MODULE_BASE_API
createCubeProxyGeometry(const std::shared_ptr<const Volume> &volume);

/**
 * \brief create a clipped cube proxy geometry for the given volume
 *
 * @param volume
 * @param clipOrigin  clip origin (bottom left corner), normalized texture coordinates [0,1]
 * @param clipExtent  extent of the clipped volume, normalized texture coordinates [0,1]
 * @return cube proxy geometry
 */
std::shared_ptr<SimpleMesh> IVW_MODULE_BASE_API createCubeProxyGeometry(
    const std::shared_ptr<const Volume> &volume, const vec3 &clipOrigin, const vec3 &clipExtent);

/**
 * \brief create a clipped cube proxy geometry for the given volume
 *
 * @param volume
 * @param clipMin  bottom left clip position in voxel coordinates [0,volDim - 1]
 * @param clipMax  top right clip position in voxel coordinates [0,volDim - 1]
 * @return cube proxy geometry
 */
std::shared_ptr<SimpleMesh> IVW_MODULE_BASE_API createCubeProxyGeometry(
    const std::shared_ptr<const Volume> &volume, const size3_t &clipMin, const size3_t &clipMax);

}  // namespace algorithm

}  // namespace inviwo

#endif  // IVW_CUBEPROXYGEOMETRY_H
