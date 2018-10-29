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
#include <inviwo/core/interaction/cameratrackball.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/io/serialization/versionconverter.h>
#include <modules/opengl/buffer/buffergl.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>

namespace inviwo {

const ProcessorInfo MPREntryExitPoints::processorInfo_{
    "org.inviwo.MPREntryExitPoints",  // Class identifier
    "MPR Entry Exit Points",           // Display name
    "Mesh Rendering",          // Category
    CodeState::Experimental,             // Code state
    Tags::GL,                      // Tags
};
const ProcessorInfo MPREntryExitPoints::getProcessorInfo() const { return processorInfo_; }

MPREntryExitPoints::MPREntryExitPoints()
    : Processor()
    , entryPort_("entry", DataVec4UInt16::get())
    , exitPort_("exit", DataVec4UInt16::get())
    , camera_("camera", "Camera", vec3(0.0f, 0.0f, -2.0f), vec3(0.0f, 0.0f, 0.0f),
              vec3(0.0f, 1.0f, 0.0f))
    , capNearClipping_("capNearClipping", "Cap near plane clipping", true)
    , trackball_(&camera_)
	, planePosition_("planePosition", "Position")
	, planeNormal_("planeNormal", "Normal")
	, planeUp_("planeUp", "Up")
	, offset0_("offset0", "Offset 0", -0.01f, -1.0f, 0.0f, 0.001f)
	, offset1_("offset1", "Offset 1", 0.01f, 0.0f, 1.0f, 0.001f)
	, shader_("uv_pass_through.vert", "mpr_entry_exit_points.frag") {
    addPort(entryPort_, "ImagePortGroup1");
    addPort(exitPort_, "ImagePortGroup1");
    addProperty(capNearClipping_);
    addProperty(camera_);
    addProperty(trackball_);
	addProperty(planePosition_);
	addProperty(planeNormal_);
	addProperty(offset0_);
	addProperty(offset1_);
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
	shader_.setUniform("middle", planePosition_.get());
	shader_.setUniform("normal", planeNormal_.get());
	shader_.setUniform("offset", offset0_.get()); // note that setUniform does not work when passing a literal 0
	auto quadGL = quad->getRepresentation<BufferGL>();
	quadGL->enable();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	// generate exit points
	utilgl::activateAndClearTarget(*exitPort_.getEditableData().get(), ImageType::ColorOnly);
	shader_.setUniform("offset", offset1_.get());
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	quadGL->disable();
}

void MPREntryExitPoints::deserialize(Deserializer& d) {
    util::renamePort(d, {{&entryPort_, "entry-points"}, {&exitPort_, "exit-points"}});
    Processor::deserialize(d);
}

}  // namespace
