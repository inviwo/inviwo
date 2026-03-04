/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2026 Inviwo Foundation
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

#include <modules/oit/oitmoduledefine.h>

#include <inviwo/core/datastructures/spatialdata.h>
#include <inviwo/core/util/document.h>
#include <inviwo/core/util/glmmat.h>
#include <inviwo/core/util/glmvec.h>
#include <modules/oit/datastructures/rasterization.h>

#include <functional>
#include <memory>
#include <type_traits>

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

namespace inviwo {

/**
 * @brief A very simple SpatialEntity to handle a world and model transform
 * Used by rasterizations for adding transforms before rendering, without copying the mesh data.
 */
struct IVW_MODULE_OIT_API CompositeTransform : public SpatialEntity {
    CompositeTransform() = default;
    CompositeTransform(const SpatialEntity& rhs) : SpatialEntity(rhs) {}
    CompositeTransform(const mat4& modelMatrix, const mat4& worldMatrix)
        : SpatialEntity(modelMatrix, worldMatrix) {}
    virtual SpatialEntity* clone() const override { return new CompositeTransform(*this); }

    virtual const Axis* getAxis(size_t) const override { return nullptr; }
};

}  // namespace inviwo
