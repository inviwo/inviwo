/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <inviwo/meshandvolume/meshandvolumemoduledefine.h>
#include <inviwo/core/processors/processor.h>

#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/ports/volumeport.h>
#include <modules/meshrenderinggl/ports/rasterizationport.h>
#include <inviwo/meshandvolume/rendering/myfragmentlistrenderer.h>
#include <inviwo/core/properties/isotfproperty.h>
#include <inviwo/core/properties/simplelightingproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/cameraproperty.h>

namespace inviwo {

class IVW_MODULE_MESHANDVOLUME_API VolumeRasterizer : public Processor {
    friend class VolumeRasterization;

public:
    VolumeRasterizer();
    virtual ~VolumeRasterizer() override = default;

    virtual void process() override;
    virtual void initializeResources() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    void setUniforms(Shader& shader, std::string_view prefix) const;

protected:
    VolumeInport volumeInport_;
    MeshInport meshInport_;
    RasterizationOutport outport_;

    std::shared_ptr<Shader> shader_;
    IsoTFProperty tf_;
    OptionPropertyInt channel_;
    CameraProperty camera_;
    SimpleLightingProperty lighting_;
};

class IVW_MODULE_MESHANDVOLUME_API VolumeRasterization : public Rasterization {
public:
    VolumeRasterization(const VolumeRasterizer& processor);
    virtual void rasterize(const ivec2& imageSize, const mat4& worldMatrixTransform,
                           std::function<void(Shader&)> setUniforms) const override;
    virtual bool usesFragmentLists() const override {
        return MyFragmentListRenderer::supportsFragmentLists();
    }
    virtual Document getInfo() const override;
    virtual Rasterization* clone() const override;
    const RaycastingState* getRaycastingState() const;

public:
    RaycastingState raycastState_;
    std::shared_ptr<Shader> shader_;
    std::shared_ptr<const Volume> volume_;
    std::shared_ptr<const Mesh> boundingMesh_;
};

}  // namespace inviwo
