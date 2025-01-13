/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2025 Inviwo Foundation
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
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/ports/volumeport.h>
#include <modules/opengl/shader/shader.h>
#include <inviwo/core/properties/isotfproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <modules/oit/ports/rasterizationport.h>
#include <modules/oit/processors/rasterizer.h>
#include <modules/oit/rendering/volumefragmentlistrenderer.h>

namespace inviwo {

class IVW_MODULE_OIT_API VolumeRasterizer : public Rasterizer {
public:
    VolumeRasterizer();
    virtual ~VolumeRasterizer() override = default;

    virtual void initializeResources() override;

    virtual void rasterize(const ivec2& imageSize, const mat4& worldMatrixTransform) override;

    virtual UseFragmentList usesFragmentLists() const override {
        return VolumeFragmentListRenderer::supportsFragmentLists() ? UseFragmentList::Yes
                                                                   : UseFragmentList::No;
    }

    virtual std::optional<mat4> boundingBox() const override;

    virtual std::optional<RaycastingState> getRaycastingState() const override;

    virtual Document getInfo() const override;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void setUniforms(Shader& shader) override;

    VolumeInport volumeInport_;
    MeshInport meshInport_;

    Shader shader_;
    IsoTFProperty tf_;
    OptionPropertyInt channel_;
    FloatProperty opacityScaling_;

    std::shared_ptr<TFLookupTable> tfLookup_;
};

}  // namespace inviwo
