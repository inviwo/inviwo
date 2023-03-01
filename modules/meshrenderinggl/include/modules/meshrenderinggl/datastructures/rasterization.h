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

#include <modules/meshrenderinggl/meshrenderingglmoduledefine.h>  // for IVW_MODULE_MESHRENDERIN...

#include <inviwo/core/processors/processor.h>
#include <modules/meshrenderinggl/ports/rasterizationport.h>

#include <inviwo/core/datastructures/datatraits.h>  // for DataTraits
#include <inviwo/core/util/document.h>              // for Document
#include <inviwo/core/util/glmmat.h>                // for mat4
#include <inviwo/core/util/glmvec.h>                // for uvec3, ivec2

#include <functional>  // for function
#include <string>      // for string
#include <optional>

namespace inviwo {
class Shader;
class RasterizationProcessor;

/**
 * \brief A functor class for rendering geometry into a fragment list
 * Will be applied by a renderer containing an A-buffer.
 */
class IVW_MODULE_MESHRENDERINGGL_API Rasterization {
public:
    Rasterization() = default;
    Rasterization(std::shared_ptr<RasterizationProcessor> processor);
    Rasterization(const Rasterization&) = delete;
    Rasterization(Rasterization&&) = delete;
    Rasterization& operator=(const Rasterization&) = delete;
    Rasterization& operator=(Rasterization&&) = delete;

    std::shared_ptr<RasterizationProcessor> getProcessor() const;

    /**
     * \brief Query whether fragments will be emitted.
     * @return True for order-independent rendering, false for opaque rasterization.
     */
    bool usesFragmentLists() const;

    /**
     * \brief Get data description for the network interface.
     * @return Specific information about this rasterization type/instance.
     */
    Document getInfo() const;

    /**
     * \brief Return the world space bounding box of the rendered geometry.
     */
    std::optional<mat4> boundingBox() const;

private:
    std::weak_ptr<RasterizationProcessor> processor_;
};

class IVW_MODULE_MESHRENDERINGGL_API RasterizationProcessor : public Processor {
public:
    RasterizationProcessor(std::string_view identifier = "", std::string_view displayName = "");

    virtual void initializeResources() override;

    virtual void process() final;
 



    /**
     * \brief Render the fragments, with all setup and evaluation taken care of.
     * If opaque is set, a standard render call instead.
     * @param imageSize Size in pixels.
     * @param worldMatrixTransform Additional transform to be applied before rendering.
     * @param setUniforms Binds the fragment list buffer and sets required uniforms.
     */
    virtual void rasterize(const ivec2& imageSize, const mat4& worldMatrixTransform,
                           std::function<void(Shader&)> setUniforms,
                           std::function<void(Shader&)> initializeShader) = 0;

    /**
     * \brief Query whether fragments will be emitted.
     * @return True for order-independent rendering, false for opaque rasterization.
     */
    virtual bool usesFragmentLists() const = 0;

    /**
     * \brief Return the world space bounding box of the rendered geometry.
     */
    virtual std::optional<mat4> boundingBox() const { return std::nullopt; }

    virtual Document getInfo() const { return Document{}; }

    RasterizationOutport outport_;
    
    bool needsInitialize_;
};

template <>
struct DataTraits<Rasterization> {
    static std::string classIdentifier() { return "org.inviwo.Rasterization"; }
    static std::string dataName() { return "Rasterization"; }
    static uvec3 colorCode() { return uvec3(80, 160, 160); }
    static Document info(const Rasterization& data) { return data.getInfo(); }
};
}  // namespace inviwo
