/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2018 Inviwo Foundation
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

#include <modules/basegl/processors/mprentryexitpointsprocessor.h>
#include <inviwo/core/io/serialization/versionconverter.h>
#include <modules/opengl/buffer/buffergl.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>

namespace inviwo {

const ProcessorInfo MPREntryExitPoints::processorInfo_{
    "org.inviwo.MPREntryExitPoints",  // Class identifier
    "MPR Entry Exit Points",          // Display name
    "Mesh Rendering",                 // Category
    CodeState::Experimental,          // Code state
    Tags::GL,                         // Tags
};
const ProcessorInfo MPREntryExitPoints::getProcessorInfo() const { return processorInfo_; }

MPREntryExitPoints::MPREntryExitPoints()
    : Processor()
    , volumeInport_("volume")
    , entryPort_("entry", DataVec4UInt16::get())
    , exitPort_("exit", DataVec4UInt16::get())
    , p_("planePosition", "p", vec3(0.5f))
    , n_("planeNormal", "n", vec3(0.0f, 0.0f, 1.0f), vec3(-1.0f), vec3(1.0f))
    , u_("planeUp", "u", vec3(0.0f, 1.0f, 0.0f), vec3(-1.0f), vec3(1.0f))
    , offset0_("offset0", "Offset 0", -0.01f, -1.0f, 1.0f, 0.001f)
    , offset1_("offset1", "Offset 1", 0.01f, -1.0f, 1.0f, 0.001f)
    , R_("rotationMatrix", "R", mat4(1.0f))
    , n_prime_("nPrime", "n'", n_.get(), vec3(-1.0f), vec3(1.0f))
    , u_prime_("uPrime", "u'", u_.get(), vec3(-1.0f), vec3(1.0f))
    , cursorScreenPos_("cursorScreenPos", "Cursor Screen Pos", vec2(0.5f), vec2(0.0f), vec2(1.0f))
    , cursorScreenPosOld_("cursorScreenPosOld", "Cursor Screen Pos Old", cursorScreenPos_.get(), vec2(0.0f), vec2(1.0f))
    , shader_("uv_pass_through.vert", "mpr_entry_exit_points.frag") {

    addPort(volumeInport_);
    addPort(entryPort_, "ImagePortGroup1");
    addPort(exitPort_, "ImagePortGroup1");

    addProperty(p_);
    n_.onChange([this]() {
        n_prime_ = R_.get() * vec4(n_.get(), 0.0f);
    });
    addProperty(n_);
    u_.onChange([this]() {
        u_prime_ = R_.get() * vec4(u_.get(), 0.0f);
    });
    addProperty(u_);
    addProperty(offset0_);
    addProperty(offset1_);

    R_.onChange([this]() {
        n_prime_ = R_.get() * vec4(n_.get(), 0.0f);
        u_prime_ = R_.get() * vec4(u_.get(), 0.0f);
    });
    addProperty(R_);
    addProperty(n_prime_); n_prime_.setReadOnly(true);
    addProperty(u_prime_); u_prime_.setReadOnly(true);
    
    cursorScreenPos_.onChange([this]() {
        const auto offset = cursorScreenPos_.get() - cursorScreenPosOld_.get();

        // Construct coordinate cross for this plane
        const auto n_prime_normalized = glm::normalize(n_prime_.get());
        const auto u_prime_normalized = glm::normalize(u_prime_.get());
        const auto r_prime_normalized = glm::normalize(cross(n_prime_normalized, u_prime_normalized));

        // update plane position
        p_ = p_.get() + offset.x * r_prime_normalized + offset.y * u_prime_normalized;

        // save current cursor position
        cursorScreenPosOld_ = cursorScreenPos_;
    });
    addProperty(cursorScreenPos_);
    addProperty(cursorScreenPosOld_);

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
}

MPREntryExitPoints::~MPREntryExitPoints() {}

void MPREntryExitPoints::process() {
    auto const quad = util::makeBuffer<vec2>({
        { -1.0f, -1.0f },{ 1.0f, -1.0f },{ -1.0f, 1.0f },{ 1.0f, 1.0f } 
    });

    // Construct coordinate cross for this plane
    const auto n_prime_normalized = glm::normalize(n_prime_.get());
    const auto u_prime_normalized = glm::normalize(u_prime_.get());
    const auto r_prime_normalized = glm::normalize(cross(n_prime_normalized, u_prime_normalized));

    // generate entry points
    utilgl::activateAndClearTarget(*entryPort_.getEditableData().get(), ImageType::ColorOnly);
    shader_.activate();

    shader_.setUniform("p", p_.get()); // plane pos. in volume space
    shader_.setUniform("p_screen", cursorScreenPos_.get()); // plane pos. in screen space
    shader_.setUniform("n", n_prime_normalized); // plane's normal
    shader_.setUniform("u", u_prime_normalized); // plane's up
    shader_.setUniform("r", r_prime_normalized); // plane's right
    shader_.setUniform("thickness_offset", offset0_.get()); // plane's offset along normal, // note that setUniform does not work when passing a literal 0

    auto quadGL = quad->getRepresentation<BufferGL>();
    quadGL->enable();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // generate exit points
    utilgl::activateAndClearTarget(*exitPort_.getEditableData().get(), ImageType::ColorOnly);
    shader_.setUniform("thickness_offset", offset1_.get());
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    quadGL->disable();
}

void MPREntryExitPoints::deserialize(Deserializer& d) {
    util::renamePort(d, {{&entryPort_, "entry-points"}, {&exitPort_, "exit-points"}});
    Processor::deserialize(d);
}

}  // namespace
