/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2024 Inviwo Foundation
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

#include <modules/meshrenderinggl/meshrenderingglmoduledefine.h>  // for IVW_MODULE_MESHRENDERIN...

#include <inviwo/core/datastructures/datatraits.h>  // for DataTraits
#include <inviwo/core/util/document.h>              // for Document
#include <inviwo/core/util/glmmat.h>                // for mat4
#include <inviwo/core/util/glmvec.h>                // for uvec3, ivec2

#include <modules/meshrenderinggl/rasterizeevent.h>

#include <functional>  // for function
#include <string>      // for string
#include <optional>

namespace inviwo {
class Shader;
class Rasterizer;

/**
 * @brief A functor class for rendering geometry into a fragment list
 * Will be applied by a renderer containing an A-buffer.
 */
class IVW_MODULE_MESHRENDERINGGL_API Rasterization {
public:
    Rasterization() = default;
    Rasterization(std::shared_ptr<Rasterizer> processor);
    Rasterization(const Rasterization&) = delete;
    Rasterization(Rasterization&&) = delete;
    Rasterization& operator=(const Rasterization&) = delete;
    Rasterization& operator=(Rasterization&&) = delete;

    std::shared_ptr<Rasterizer> getProcessor() const;

    void rasterize(const ivec2& imageSize, const mat4& worldMatrixTransform) const;

    /**
     * @brief Query whether fragments will be emitted.
     * @return True for order-independent rendering, false for opaque rasterization.
     */
    UseFragmentList usesFragmentLists() const;

    /**
     * @brief Get data description for the network interface.
     * @return Specific information about this rasterization type/instance.
     */
    Document getInfo() const;

    /**
     * @brief Return the world space bounding box of the rendered geometry.
     */
    std::optional<mat4> boundingBox() const;

private:
    std::weak_ptr<Rasterizer> processor_;
};

template <>
struct DataTraits<Rasterization> {
    static constexpr std::string_view classIdentifier() { return "org.inviwo.Rasterization"; }
    static constexpr std::string_view dataName() { return "Rasterization"; }
    static constexpr uvec3 colorCode() { return uvec3(80, 160, 160); }
    static Document info(const Rasterization& data) { return data.getInfo(); }
};
}  // namespace inviwo
