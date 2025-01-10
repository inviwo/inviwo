/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024-2025 Inviwo Foundation
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
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>

#include <modules/oit/ports/rasterizationport.h>
#include <modules/oit/rasterizeevent.h>

namespace inviwo {

class Shader;
struct RaycastingState;

class IVW_MODULE_OIT_API Rasterizer : public Processor {
public:
    Rasterizer(std::string_view identifier = "", std::string_view displayName = "");

    virtual void initializeResources() override;

    virtual void configureShader(Shader& shader);
    virtual void setUniforms(Shader& shader);

    virtual void process() override;

    /**
     * Override to maintain isModified() state of properties and isChanged() for inports
     * as these might be accessed later during rasterization in a subsequent rasterization renderer.
     * Note: need to set this processor to valid _after_ rasterization using setValidDelayed.
     *
     * @see setValidDelayed
     */
    virtual void setValid() override;

    /**
     * Set this processor's state to valid. To be used after the rasterization.
     *
     * @see setValid
     */
    void setValidDelayed();

    /**
     * @brief Render the fragments, with all setup and evaluation taken care of.
     * If opaque is set, a standard render call instead.
     * @param imageSize Size in pixels.
     * @param worldMatrixTransform Additional transform to be applied before rendering.
     * @param setUniforms Binds the fragment list buffer and sets required uniforms.
     */
    virtual void rasterize(const ivec2& imageSize, const mat4& worldMatrixTransform) = 0;

    /**
     * @brief Query whether fragments will be emitted.
     * @return True for order-independent rendering, false for opaque rasterization.
     */
    virtual UseFragmentList usesFragmentLists() const = 0;

    /**
     * @brief Return the world space bounding box of the rendered geometry.
     */
    virtual std::optional<mat4> boundingBox() const { return std::nullopt; }

    /**
     * @brief Query raycasting state used for volume rendering.
     * @return raycasting-specific parameters for volume rendering between volume fragments
     */
    virtual std::optional<RaycastingState> getRaycastingState() const { return std::nullopt; }

    virtual Document getInfo() const { return Document{}; }

    virtual void propagateEvent(Event* event, Outport* source) override;

    RasterizationOutport outport_;

private:
    RasterizeHandle handle_;
    std::shared_ptr<Rasterization> rasterization_;
};

}  // namespace inviwo
