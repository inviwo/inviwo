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

vec2 rotate2D(vec2 pt, float angle)
{
    return mat2(cos(angle), -sin(angle), sin(angle), cos(angle)) * pt;
}

vec3 MPREntryExitPoints::screenPosToVolumePos(const vec2& screen_pos) const
{
    // use cursorScreenPos_old_ instead of cursorScreenPos_.get() to prevent 0 offset in case the cursor has changes in the same frame
    const auto screen_offset = screen_pos - cursorScreenPos_old_;
    const auto volume_offset = screenOffsetToVolumeOffset(screen_offset);
    const auto volume_pos = mprP_.get() + volume_offset;

    return volume_pos;
}

vec3 MPREntryExitPoints::screenOffsetToVolumeOffset(const vec2& screen_offset) const
{
    const auto canvas_size = entryPort_.getDimensions();
    const auto aspect_ratio = static_cast<float>(canvas_size.x) / static_cast<float>(canvas_size.y);
    const auto vd = vec3(volumeDimensions_.get()) / glm::compMax(vec3(volumeDimensions_.get()));
    const auto vs = volumeSpacing_.get() / glm::compMin(volumeSpacing_.get());

    const auto uv_offset = screen_offset * vec2(aspect_ratio, 1.0);
    const auto uv_rotated = rotate2D(uv_offset, -correctionAngle_.get()); // watch out for the correction angle if this is actually desired

    // offset along r and u, not n
    const auto volume_offset = (1.0f / vd) * (1.0f / vs) * zoomFactor_.get() * (uv_rotated.x * mprBasisR_.get() + uv_rotated.y * mprBasisU_.get());

    return volume_offset;
}

vec2 MPREntryExitPoints::volumePosToScreenPos(const vec3& volume_pos) const
{
    const auto volume_offset = volume_pos - mprP_old_;
    const auto screen_offset = volumeOffsetToScreenOffset(volume_offset);
    const auto screen_pos = cursorScreenPos_.get() + screen_offset;

    return screen_pos;
}

vec2 MPREntryExitPoints::volumeOffsetToScreenOffset(const vec3& volume_offset) const
{
    const auto vd = vec3(volumeDimensions_.get()) / glm::compMax(vec3(volumeDimensions_.get()));
    const auto vs = volumeSpacing_.get() / glm::compMin(volumeSpacing_.get());
    const auto canvas_size = vec2(entryPort_.getDimensions());
    const auto aspect_ratio = canvas_size.x / canvas_size.y;

    const auto volume_offset_corrected = volume_offset * vd * vs * zoomFactor_.get();
    const auto offset_along_r = glm::dot(volume_offset_corrected, mprBasisR_.get()); // x-offset is along r
    const auto offset_along_u = glm::dot(volume_offset_corrected, mprBasisU_.get()); // y-offset is along u
    const auto uv_rotated = rotate2D(vec2(offset_along_r, offset_along_u), correctionAngle_.get()); // watch out for the correction angle if this is actually desired, positive correction angle here?
    const auto screen_offset = uv_rotated / vec2(aspect_ratio, 1.0);

    return screen_offset;
}

MPREntryExitPoints::MPREntryExitPoints()
    : Processor()
    , volumeInport_("volume")
    , entryPort_("entry", DataVec4UInt16::get())
    , exitPort_("exit", DataVec4UInt16::get())
    , offset0_("offset0", "Slab Offset 0", -0.01f, -1.0f, 1.0f, 0.001f)
    , offset1_("offset1", "Slab Offset 1", 0.01f, -1.0f, 1.0f, 0.001f)
    , zoomFactor_("zoomFactor", "Zoom Factor", 1.0f, 0.01f, 100.0f, 0.01f)
    , correctionAngle_("correctionAngle", "Correction Angle", 0.0f, -1e9f, 1e9f, 0.001f)
    , volumeDimensions_("volumeDimensions", "Volume Dimensions", size3_t(0), size3_t(0), size3_t(std::numeric_limits<size_t>::max()), size3_t(1))
    , volumeSpacing_("volumeSpacing", "Volume Spacing", vec3(0.0f), vec3(0.0f), vec3(1e5f), vec3(1e-3f))
    , cursorScreenPos_("cursorScreenPos", "Cursor Screen Pos", vec2(0.5f), vec2(0.0f), vec2(1.0f))
    , cursorScreenPos_old_(cursorScreenPos_.get())
    , mprP_("mprP_", "mprP_", vec3(0.5f), vec3(-10.0f), vec3(10.0f))
    , mprP_old_(mprP_.get())
    , mprBasisR_("mprBasisR_", "mprBasisR_", vec3(0.0f), vec3(-1.0f), vec3(1.0f))
    , mprBasisU_("mprBasisU_", "mprBasisU_", vec3(0.0f), vec3(-1.0f), vec3(1.0f))
    , mprBasisN_("mprBasisN_", "mprBasisN_", vec3(0.0f), vec3(-1.0f), vec3(1.0f))
    , shader_("uv_pass_through.vert", "mpr_entry_exit_points.frag")
    , dirty_(false)
{
    addPort(volumeInport_);
    addPort(entryPort_, "ImagePortGroup1");
    addPort(exitPort_, "ImagePortGroup1");

    addProperty(offset0_);
    addProperty(offset1_);
    addProperty(zoomFactor_);
    addProperty(correctionAngle_);
    addProperty(volumeDimensions_);
    addProperty(volumeSpacing_);

    cursorScreenPos_.onChange([this]() {
        if (!dirty_) {
            dirty_ = true;

            mprP_ = screenPosToVolumePos(cursorScreenPos_);

            mprP_old_ = mprP_;
            cursorScreenPos_old_ = cursorScreenPos_;

            dirty_ = false;
        }
    });
    addProperty(cursorScreenPos_);

    mprP_.onChange([this]() {
        if (!dirty_) {
            dirty_ = true;

            cursorScreenPos_ = volumePosToScreenPos(mprP_);

            mprP_old_ = mprP_;
            cursorScreenPos_old_ = cursorScreenPos_;

            dirty_ = false;
        }
    });
    addProperty(mprP_);

    addProperty(mprBasisR_);
    addProperty(mprBasisU_);
    addProperty(mprBasisN_);

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
}

MPREntryExitPoints::~MPREntryExitPoints() {}

void MPREntryExitPoints::process() {
    auto const quad = util::makeBuffer<vec2>({
        { -1.0f, -1.0f },{ 1.0f, -1.0f },{ -1.0f, 1.0f },{ 1.0f, 1.0f } 
    });

    // generate entry points
    utilgl::activateAndClearTarget(*entryPort_.getEditableData().get(), ImageType::ColorOnly);
    shader_.activate();

    const auto canvas_size = vec2(entryPort_.getDimensions());
    const auto aspect_ratio = canvas_size.x / canvas_size.y;

    shader_.setUniform("p_screen", cursorScreenPos_.get()); // plane pos. in screen space
    shader_.setUniform("p", mprP_.get()); // volume position
    shader_.setUniform("n", mprBasisN_.get()); // plane's normal
    shader_.setUniform("u", mprBasisU_.get()); // plane's up
    shader_.setUniform("r", mprBasisR_.get()); // plane's right
    shader_.setUniform("thickness_offset", offset0_.get()); // plane's offset along normal, // note that setUniform does not work when passing a literal 0
    shader_.setUniform("thickness_offset_other", offset1_.get()); // plane's offset along normal, // note that setUniform does not work when passing a literal 0
    shader_.setUniform("zoom_factor", zoomFactor_.get());
    shader_.setUniform("correction_angle", -correctionAngle_.get());
    shader_.setUniform("aspect_ratio", aspect_ratio);
    shader_.setUniform("volume_dimensions", vec3(volumeDimensions_.get()));
    shader_.setUniform("volume_spacing", volumeSpacing_.get());

    auto quadGL = quad->getRepresentation<BufferGL>();
    quadGL->enable();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // generate exit points
    utilgl::activateAndClearTarget(*exitPort_.getEditableData().get(), ImageType::ColorOnly);
    shader_.setUniform("thickness_offset", offset1_.get());
    shader_.setUniform("thickness_offset_other", offset0_.get());
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    quadGL->disable();
}

void MPREntryExitPoints::deserialize(Deserializer& d) {
    util::renamePort(d, {{&entryPort_, "entry-points"}, {&exitPort_, "exit-points"}});
    Processor::deserialize(d);
}

}  // namespace
