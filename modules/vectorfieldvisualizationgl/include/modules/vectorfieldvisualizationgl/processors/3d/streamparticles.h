/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/ports/meshport.h>
#include <modules/opengl/shader/shader.h>

#include <inviwo/core/util/clock.h>
#include <inviwo/core/util/timer.h>

#include <modules/vectorfieldvisualization/ports/seedpointsport.h>

namespace inviwo {

/** \docpage{org.inviwo.StreamParticles, Stream Particles}
 * ![](org.inviwo.StreamParticles.png?classIdentifier=org.inviwo.StreamParticles)
 * Explanation of how to use the processor.
 *
 * ### Inports
 *   * __<Inport1>__ <description>.
 *
 * ### Outports
 *   * __<Outport1>__ <description>.
 *
 * ### Properties
 *   * __<Prop1>__ <description>.
 *   * __<Prop2>__ <description>
 */

/**
 * \brief VERY_BRIEFLY_DESCRIBE_THE_PROCESSOR
 * DESCRIBE_THE_PROCESSOR_FROM_A_DEVELOPER_PERSPECTIVE
 */
class IVW_MODULE_VECTORFIELDVISUALIZATIONGL_API StreamParticles : public Processor {
public:
    StreamParticles(InviwoApplication *app);
    virtual ~StreamParticles() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    enum class SeedingSpace { Data, World };
    VolumeInport volume_{"volume"};
    SeedPoints3DInport seeds_{"seeds"};
    MeshOutport meshPort_{"particles"};

    TemplateOptionProperty<SeedingSpace> seedingSpace_{
        "seedingSpace",
        "Seeding Space",
        {{"data", "Data", SeedingSpace::Data}, {"world", "World", SeedingSpace::World}}};

    FloatProperty stepLength_{"stepLength", "Step Length", 0.01f, 0.0f, 1.0f};
    IntProperty internalSteps_{"internalSteps", "Internal Steps", 10, 1, 100};

    FloatMinMaxProperty particleSize_{
        "particleSize", "Paricle Size (visual only)", 0.025f, 0.035f, 0.0f, 1.0f};

    FloatProperty minV_{"minV",
                        "Min velocity",
                        0,
                        0,
                        std::numeric_limits<float>::max(),
                        0.1f,
                        InvalidationLevel::Valid,
                        PropertySemantics::Text};
    FloatProperty maxV_{"maxV",
                        "Max velocity",
                        1,
                        0,
                        std::numeric_limits<float>::max(),
                        0.1f,
                        InvalidationLevel::Valid,
                        PropertySemantics::Text};
    TransferFunctionProperty tf_{"tf", "Velocity mapping"};

    FloatProperty reseedInterval_{"reseedsInterval", "Reseed interval", 1.0f, 0.0f, 10.0f};

    Shader shader_{{{ShaderType::Compute, "streamparticles.comp"}}};

    Timer timer_{Timer::Milliseconds(17), [&]() { update(); }};

    double reseedtime;
    double prevT = 0;
    Clock c;
    std::mutex mutex_;
    void update();

    void initBuffers();
    void advect();
    void reseed();

    bool buffersDirty = true;
    std::shared_ptr<Mesh> mesh_{nullptr};
    std::shared_ptr<Buffer<vec4>> bufPos_{nullptr};
    std::shared_ptr<Buffer<float>> bufLife_{nullptr};
    std::shared_ptr<Buffer<float>> bufRad_{nullptr};
    std::shared_ptr<Buffer<vec4>> bufCol_{nullptr};
};

}  // namespace inviwo
