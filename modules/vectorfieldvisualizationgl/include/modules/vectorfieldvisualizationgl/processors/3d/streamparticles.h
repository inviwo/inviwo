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
#include <inviwo/core/properties/minmaxproperty.h>
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
 *
 * Processor that simulate particles moving through a velocity filed. Particles are initialized
 * based on the __seeds__ inport and are advected through the field using the Velocity Volume on the
 * __volume__ inport. If particles velocity has been zero for 0.5 seconds their position will be
 * reset to a random element in the seedpoint input vector. Reseeding is done on a less frequent
 * intervall than the advection based on the __Reseed interval__ property. Using GLSL Compute
 * Shaders, requires OpenGL 4.3
 *
 *
 * ### Inports
 *   * __volume__ Velocity field.
 *   * __seeds__ Starting position for particles.
 *
 * ### Outports
 *   * __particles__ Mesh cotaining position, color and radii buffers. Can be conntected to (for
 * example) the Sphere Renderer Processor to render them.
 *
 * ### Properties
 *   * __Seeding Space__ Tells the processor which space the seeds are defined in.
 *   * __Advection Speed__ How fast the particles will move each advection.
 *   * __Advections per Frame__ Can be used to increase advection speed without loss of
 * precision.
 *   * __Paricle radius__ Maps velocity to radius of particle (For rendering that
 * supports the Radii buffer, e.g. Sphere Renderer).
 *   * __Min velocity__ See bellow.
 *   * __Max velocity__ Used together with __Min velocity__ to map velocity magnitude from [Min
 * velocity, Max velocity] -> [0 1], used for mapping velocity to radius and color.
 *   * __Velocity mapping__ Transferfunction to map a velocity to color.
 *   * __Reseed interval__ Seconds between reseeding. When reseeding particles whose life is zero
 * will get new position by selecting (randomly) from the input seed vector.
 *
 */

class IVW_MODULE_VECTORFIELDVISUALIZATIONGL_API StreamParticles : public Processor {
public:
    StreamParticles(InviwoApplication *app);
    virtual ~StreamParticles() = default;

    virtual void initializeResources() override;
    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    void update();
    void initBuffers();
    void reseed();
    void advect();

    enum class SeedingSpace { Data, World };
    VolumeInport volume_{"volume"};
    SeedPoints3DInport seeds_{"seeds"};
    MeshOutport meshPort_{"particles"};

    TemplateOptionProperty<SeedingSpace> seedingSpace_{
        "seedingSpace",
        "Seeding Space",
        {{"data", "Data", SeedingSpace::Data}, {"world", "World", SeedingSpace::World}}};

    FloatProperty advectionSpeed_{"advectionSpeed", "Advection Speed", 0.01f, 0.0f, 1.0f};
    IntProperty internalSteps_{"advectionsPerFrame",
                               "Advections per Frame",
                               10,
                               1,
                               100,
                               1,
                               InvalidationLevel::InvalidResources};

    FloatMinMaxProperty particleSize_{
        "particleSize", "Particle radius", 0.025f, 0.035f, 0.0f, 1.0f};

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

    Shader shader_{{{ShaderType::Compute, std::string{"streamparticles.comp"}}}, Shader::Build::No};

    Timer timer_{Timer::Milliseconds(17), [&]() { update(); }};

    double reseedtime_;
    double prevT_;
    Clock clock_;
    bool ready_;
    bool buffersDirty_ = true;

    std::shared_ptr<Mesh> mesh_{nullptr};
    std::shared_ptr<Buffer<vec4>> bufPos_{nullptr};
    std::shared_ptr<Buffer<float>> bufLife_{nullptr};
    std::shared_ptr<Buffer<float>> bufRad_{nullptr};
    std::shared_ptr<Buffer<vec4>> bufCol_{nullptr};
};

}  // namespace inviwo
