/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2025 Inviwo Foundation
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

#include <modules/vectorfieldvisualizationgl/vectorfieldvisualizationglmoduledefine.h>

#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorinfo.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/util/clock.h>
#include <inviwo/core/util/glmvec.h>  // for vec4
#include <inviwo/core/util/staticstring.h>
#include <inviwo/core/util/timer.h>
#include <modules/opengl/shader/shader.h>
#include <modules/vectorfieldvisualization/ports/seedpointsport.h>

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace inviwo {
class InviwoApplication;
class Mesh;

class IVW_MODULE_VECTORFIELDVISUALIZATIONGL_API StreamParticles : public Processor {
public:
    StreamParticles(InviwoApplication* app);
    virtual ~StreamParticles() = default;

    virtual void initializeResources() override;
    virtual void process() override;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    void update();
    void initBuffers();
    void reseed();
    void advect();

    enum class SeedingSpace { Data, World };
    VolumeInport volume_;
    SeedPoints3DInport seeds_;
    MeshOutport meshPort_;
    OptionProperty<SeedingSpace> seedingSpace_;

    FloatProperty advectionSpeed_;
    IntProperty internalSteps_;

    FloatMinMaxProperty particleSize_;

    FloatProperty minV_;
    FloatProperty maxV_;
    TransferFunctionProperty tf_;

    FloatProperty reseedInterval_;

    Shader shader_;
    Timer timer_;
    double reseedtime_;
    double prevT_;
    Clock clock_;
    bool ready_;
    bool buffersDirty_;

    std::shared_ptr<Mesh> mesh_;
    std::shared_ptr<Buffer<vec4>> bufPos_;
    std::shared_ptr<Buffer<float>> bufLife_;
    std::shared_ptr<Buffer<float>> bufRad_;
    std::shared_ptr<Buffer<vec4>> bufCol_;
};

}  // namespace inviwo
