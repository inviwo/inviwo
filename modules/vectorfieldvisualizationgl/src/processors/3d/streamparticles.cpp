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

#include <modules/vectorfieldvisualizationgl/processors/3d/streamparticles.h>

#include <inviwo/core/common/inviwoapplication.h>                       // for InviwoApplication
#include <inviwo/core/datastructures/buffer/buffer.h>                   // for Buffer
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>       // for BufferRAMPrecision
#include <inviwo/core/datastructures/coordinatetransformer.h>           // for StructuredCoordin...
#include <inviwo/core/datastructures/geometry/geometrytype.h>           // for BufferUsage, Buff...
#include <inviwo/core/datastructures/geometry/mesh.h>                   // for Mesh
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/datastructures/transferfunction.h>                // for TransferFunction
#include <inviwo/core/ports/meshport.h>                                 // for MeshOutport
#include <inviwo/core/ports/volumeport.h>                               // for VolumeInport
#include <inviwo/core/processors/processor.h>                           // for Processor
#include <inviwo/core/processors/processorinfo.h>                       // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                      // for CodeState, CodeSt...
#include <inviwo/core/processors/processortags.h>                       // for Tags, Tags::GL
#include <inviwo/core/properties/invalidationlevel.h>                   // for InvalidationLevel
#include <inviwo/core/properties/minmaxproperty.h>                      // for FloatMinMaxProperty
#include <inviwo/core/properties/optionproperty.h>                      // for OptionPropertyOption
#include <inviwo/core/properties/ordinalproperty.h>                     // for FloatProperty
#include <inviwo/core/properties/propertysemantics.h>                   // for PropertySemantics
#include <inviwo/core/properties/transferfunctionproperty.h>            // for TransferFunctionP...
#include <inviwo/core/util/clock.h>                                     // for Clock
#include <inviwo/core/util/exception.h>                                 // for Exception
#include <inviwo/core/util/glmutils.h>                                  // for Matrix
#include <inviwo/core/util/glmvec.h>                                    // for vec4, vec3
#include <inviwo/core/util/logcentral.h>                                // for log, LogLevel
#include <inviwo/core/util/pathtype.h>                                  // for PathType, PathTyp...
#include <inviwo/core/util/rendercontext.h>                             // for RenderContext
#include <inviwo/core/util/staticstring.h>                              // for operator+
#include <inviwo/core/util/stringconversion.h>                          // for toString
#include <inviwo/core/util/timer.h>                                     // for Timer, Timer::Mil...
#include <inviwo/core/util/zip.h>                                       // for get, zip, zipIter...
#include <modules/opengl/buffer/buffergl.h>                             // for BufferGL
#include <modules/opengl/inviwoopengl.h>                                // for GL_SHADER_STORAGE...
#include <modules/opengl/openglcapabilities.h>
#include <modules/opengl/shader/shader.h>                           // for Shader, Shader::B...
#include <modules/opengl/shader/shaderobject.h>                     // for ShaderObject
#include <modules/opengl/shader/shadertype.h>                       // for ShaderType, Shade...
#include <modules/opengl/shader/shaderutils.h>                      // for setUniforms
#include <modules/opengl/texture/textureunit.h>                     // for TextureUnitContainer
#include <modules/opengl/texture/textureutils.h>                    // for bindAndSetUniforms
#include <modules/opengl/volume/volumeutils.h>                      // for bindAndSetUniforms
#include <modules/vectorfieldvisualization/ports/seedpointsport.h>  // for SeedPoints3DInport

#include <algorithm>      // for transform, fill, min
#include <cstddef>        // for size_t
#include <limits>         // for numeric_limits
#include <random>         // for mt19937, uniform_...
#include <type_traits>    // for remove_extent_t
#include <unordered_map>  // for unordered_map
#include <unordered_set>  // for unordered_set
#include <utility>        // for pair

#include <glm/detail/qualifier.hpp>  // for tvec2
#include <glm/mat4x4.hpp>            // for operator*, mat
#include <glm/vec2.hpp>              // for vec<>::(anonymous)
#include <glm/vec4.hpp>              // for operator*, operator+

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo StreamParticles::processorInfo_{
    "org.inviwo.StreamParticles",  // Class identifier
    "Stream Particles",            // Display name
    "Particle",                    // Category
    CodeState::Experimental,       // Code state
    Tags::GL,                      // Tags
    R"(Processor that simulate particles moving through a velocity filed. Particles are initialized
    based on the __seeds__ inport and are advected through the field using the Velocity Volume on
    the __volume__ inport. If particles velocity has been zero for 0.5 seconds their position will
    be reset to a random element in the seedpoint input vector. Reseeding is done on a less frequent
    intervall than the advection based on the __Reseed interval__ property. Using GLSL Compute
    Shaders, requires OpenGL 4.3)"_unindentHelp};

const ProcessorInfo& StreamParticles::getProcessorInfo() const { return processorInfo_; }

StreamParticles::StreamParticles(InviwoApplication* app)
    : Processor()
    , volume_{"volume", "Velocity field"_help}
    , seeds_{"seeds", "Starting position for particles"_help}
    , meshPort_{"particles",
                "Mesh containing position, color and radii buffers. Can be "
                "connected to (for example) the Sphere Renderer Processor to render them"_help}
    , seedingSpace_{"seedingSpace",
                    "Seeding Space",
                    "Tells the processor which space the seeds are defined in"_help,
                    {{"data", "Data", SeedingSpace::Data}, {"world", "World", SeedingSpace::World}}}
    , advectionSpeed_{"advectionSpeed", "Advection Speed",
                      util::ordinalLength(0.01f, 1.0f)
                          .set("How fast the particles will move each advection."_help)}
    , internalSteps_{"advectionsPerFrame", "Advections per Frame",
                     util::ordinalCount<int>(10, 100)
                         .setMin(1)
                         .set(InvalidationLevel::InvalidResources)
                         .set("Can be used to increase advection "
                              "speed without loss of precision"_help)}
    , particleSize_{"particleSize",
                    "Particle radius",
                    "Maps velocity to radius of particle (For rendering that supports "
                    "the Radii buffer, e.g. Sphere Renderer)."_help,
                    0.025f,
                    0.035f,
                    0.0f,
                    1.0f}
    , minV_{"minV", "Min velocity",
            util::ordinalLength(0.0f)
                .set("Used together with __Max velocity__ to map velocity magnitude "
                     "from [Min velocity, Max velocity] -> [0 1], used for mapping "
                     "velocity to radius and color"_help)
                .set(InvalidationLevel::Valid)
                .set(PropertySemantics::Text)}
    , maxV_{"maxV", "Max velocity",
            util::ordinalLength(1.0f)
                .set("Used together with __Min velocity__ to map velocity magnitude "
                     "from [Min velocity, Max velocity] -> [0 1], used for mapping "
                     "velocity to radius and color"_help)
                .set(InvalidationLevel::Valid)
                .set(PropertySemantics::Text)}
    , tf_{"tf", "Velocity mapping", "Transferfunction to map a velocity to color"_help,
          TransferFunction::load(
              filesystem::getPath(PathType::TransferFunctions, "/matplotlib/plasma.itf"))}
    , reseedInterval_{"reseedsInterval", "Reseed interval",
                      util::ordinalLength(1.0f, 10.f)
                          .set("Seconds between reseeding. When reseeding particles whose "
                               "life is zero will get new position by selecting (randomly) "
                               "from the input seed vector"_help)}
    , shader_{{{ShaderType::Compute, std::string{"streamparticles.comp"}}}, Shader::Build::No}
    , timer_{Timer::Milliseconds(17), [&]() { update(); }}
    , reseedtime_{0.0}
    , prevT_{0}
    , clock_{}
    , ready_{false}
    , buffersDirty_{true} {

    addPort(volume_);
    addPort(seeds_);
    addPort(meshPort_);

    addProperties(seedingSpace_, advectionSpeed_, internalSteps_, particleSize_, minV_, maxV_, tf_,
                  reseedInterval_);

    shader_.onReload([this]() {
        invalidate(InvalidationLevel::InvalidOutput);
        buffersDirty_ = true;
    });

    seedingSpace_.onChange([this]() { buffersDirty_ = true; });
    seeds_.onChange([this]() { buffersDirty_ = true; });

    timer_.start();
}

void StreamParticles::initializeResources() {
    if (OpenGLCapabilities::getOpenGLVersion() < 430) {
        isReady_.setUpdate([]() -> ProcessorStatus {
            static constexpr std::string_view error{
                "The StreamParticles processor requires OpenGL 4.3"};
            return {ProcessorStatus::Error, error};
        });
        throw Exception(IVW_CONTEXT, "The StreamParticles processor requires OpenGL 4.3");
    }

    shader_.getShaderObject(ShaderType::Compute)
        ->setShaderDefine("NUM_ADVECTIONS", true, toString(internalSteps_.get()));
    shader_.build();
}

void StreamParticles::process() {
    if (buffersDirty_) initBuffers();
    reseed();
    advect();
    meshPort_.setData(mesh_);
    ready_ = true;
}

void StreamParticles::update() {
    try {
        RenderContext::getPtr()->activateDefaultRenderContext();
        if (ready_) {
            ready_ = false;
            invalidate(InvalidationLevel::InvalidOutput);
        }
    } catch (const Exception& e) {
        log::exception(e);
    }
}

void StreamParticles::initBuffers() {
    auto seeds = seeds_.getData();
    auto numPoints = seeds->size();

    bufPos_ = std::make_shared<Buffer<vec4>>(numPoints, BufferUsage::Dynamic);
    bufLife_ = std::make_shared<Buffer<float>>(numPoints, BufferUsage::Dynamic);
    bufRad_ = std::make_shared<Buffer<float>>(numPoints, BufferUsage::Dynamic);
    bufCol_ = std::make_shared<Buffer<vec4>>(numPoints, BufferUsage::Dynamic);

    std::mt19937 rand;
    std::uniform_real_distribution<float> dist(0, 1);

    auto& positions = bufPos_->getEditableRAMRepresentation()->getDataContainer();
    auto& lifes = bufLife_->getEditableRAMRepresentation()->getDataContainer();

    if (seedingSpace_.get() == SeedingSpace::World) {
        std::transform(seeds->begin(), seeds->end(), positions.begin(),
                       [](vec3 seed) { return vec4(seed, 1.0f); });
    } else {
        std::transform(
            seeds->begin(), seeds->end(), positions.begin(),
            [toWorld =
                 volume_.getData()->getCoordinateTransformer().getDataToWorldMatrix()](vec3 seed) {
                vec4 p = toWorld * vec4(seed, 1.0);
                return p / p.w;
            });
    }

    std::fill(lifes.begin(), lifes.end(), 1.0f);

    bufPos_->getEditableRepresentation<BufferGL>();
    bufLife_->getEditableRepresentation<BufferGL>();
    bufRad_->getEditableRepresentation<BufferGL>();
    bufCol_->getEditableRepresentation<BufferGL>();

    mesh_ = std::make_shared<Mesh>();
    mesh_->addBuffer(BufferType::PositionAttrib, bufPos_);
    mesh_->addBuffer(BufferType::RadiiAttrib, bufRad_);
    mesh_->addBuffer(BufferType::ColorAttrib, bufCol_);

    prevT_ = reseedtime_ = clock_.getElapsedSeconds();
    buffersDirty_ = false;
}

void StreamParticles::reseed() {
    auto curT = clock_.getElapsedSeconds();
    if (curT >= reseedtime_ + reseedInterval_.get()) {
        auto seeds = seeds_.getData();

        auto& positions = bufPos_->getEditableRAMRepresentation()->getDataContainer();
        auto& lifes = bufLife_->getEditableRAMRepresentation()->getDataContainer();

        std::mt19937 rand;
        std::uniform_int_distribution<size_t> dist(0, seeds->size());

        auto seedTransform =
            [space = seedingSpace_.get(),
             toWorld = volume_.getData()->getCoordinateTransformer().getDataToWorldMatrix()](
                vec3 seed) -> vec4 {
            if (space == SeedingSpace::World) {
                return vec4(seed, 1.0f);
            } else {
                vec4 p = toWorld * vec4(seed, 1.0f);
                return p / p.w;
            }
        };

        for (auto z : util::zip(positions, lifes)) {
            if (get<1>(z) <= 0) {
                get<0>(z) = seedTransform((*seeds)[dist(rand)]);
                get<1>(z) = 1.0f;
            }
        }

        reseedtime_ = curT;
    }
}

void StreamParticles::advect() {
    TextureUnitContainer cont;

    const auto t = clock_.getElapsedSeconds();
    const auto dt = t - prevT_;
    prevT_ = t;

    auto volume = volume_.getData();

    auto bufPosGL = bufPos_->getEditableRepresentation<BufferGL>();
    auto bufLifeGL = bufLife_->getEditableRepresentation<BufferGL>();
    auto bufRadGL = bufRad_->getEditableRepresentation<BufferGL>();
    auto bufColGL = bufCol_->getEditableRepresentation<BufferGL>();

    shader_.activate();

    // using min(dt,0.1) prevent big jumps when execution is slow / have been paused
    shader_.setUniform("dt", std::min(static_cast<float>(dt), 0.1f));
    utilgl::setUniforms(shader_, advectionSpeed_, internalSteps_, minV_, maxV_);
    utilgl::bindAndSetUniforms(shader_, cont, tf_);

    shader_.setUniform("minR", particleSize_.get().x);
    shader_.setUniform("maxR", particleSize_.get().y);
    shader_.setUniform("toTextureMatrix",
                       volume->getCoordinateTransformer().getWorldToTextureMatrix());

    utilgl::bindAndSetUniforms(shader_, cont, *volume, "velocityField");

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufPosGL->getId());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufLifeGL->getId());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, bufRadGL->getId());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, bufColGL->getId());

    glDispatchCompute(static_cast<GLuint>(seeds_.getData()->size()), 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

    shader_.deactivate();

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, 0);
}

}  // namespace inviwo
