/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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

#include <modules/plottinggl/plottingglmoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/ports/layerport.h>
#include <modules/base/properties/datarangeproperty.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/buffer/framebufferobject.h>
#include <modules/basegl/datastructures/meshshadercache.h>

namespace inviwo {

class Mesh;
class Layer;

class IVW_MODULE_PLOTTINGGL_API ContinuousHistogram : public Processor {
public:
    ContinuousHistogram();

    virtual void initializeResources();
    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    void configureShader(Shader& shader);
    std::shared_ptr<Mesh> createDensityMesh() const;

    enum class Scaling { Linear, Log };

    VolumeInport inport1_;
    VolumeInport inport2_;
    LayerOutport outport_;

    IntProperty histogramResolution_;
    FloatProperty scalingFactor_;
    FloatProperty errorThreshold_;
    OptionPropertyInt channel1_;
    OptionPropertyInt channel2_;
    OptionProperty<Scaling> scaling_;
    DataRangeProperty dataRange_;

    MeshShaderCache shaders_;
    LayerConfig config_;
    std::vector<std::pair<FrameBufferObject, std::shared_ptr<Layer>>> cache_;
    std::shared_ptr<Mesh> mesh_;
};

}  // namespace inviwo
