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

#ifndef IVW_MESHCAMERAALGORITHMS_H
#define IVW_MESHCAMERAALGORITHMS_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/properties/cameraproperty.h>

namespace inviwo {

namespace meshutil {
/**
 * \brief Set lookAt position of camera to the center point of the meshes and adjust near/far plane
 * distances. Adjusts min/max parameters of lookTo and near/far plane according to the supplied
 * minMaxRatio.
 *
 * @param meshes Meshes to consider
 * @param camera Camera to adjust
 * @param minMaxRatio Adjust min/max values to (value - minMaxRatio*|value|, value +
 * minMaxRatio*|value|)
 */
IVW_MODULE_BASE_API void centerViewOnMeshes(const std::vector<std::shared_ptr<const Mesh>>& meshes,
                                            CameraProperty& camera, float minMaxRatio = 10.f);
/**
 * \brief Compute near and far plane parameters covering the bounding box when maximally zoomed out.
 * Projects the bounding box onto the view direction and selects the distance furthest away as far
 * plane (increased by 1% to make sure that mesh is not clipped). The view directions considered are
 * lookFrom min/max -> lookTo. Near plane is computed as max(1e^-6, farPlaneDistance * farNearRatio)
 *
 * @param worldSpaceBoundingBox Min and max points of geometry
 * @param camera Camera used as basis for computation
 * @param farNearRatio Ratio between near and far plane. 1:10000 is commonly used by game engines.
 * @return Near and far plane distances.
 */
IVW_MODULE_BASE_API std::pair<float, float> computeNearFarPlanes(
    std::pair<vec3, vec3> worldSpaceBoundingBox, const CameraProperty& camera,
    float nearFarRatio = 1.f / 10000.f);

}  // namespace meshutil

}  // namespace inviwo

#endif  // IVW_MESHCAMERAALGORITHMS_H
