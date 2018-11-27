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
	, volumeInport_("volume")
    , entryPort_("entry", DataVec4UInt16::get())
    , exitPort_("exit", DataVec4UInt16::get())
    , capNearClipping_("capNearClipping", "Cap near plane clipping", true)
	, planePosition_("planePosition", "Position")
	, planeNormal_("planeNormal", "Normal", vec3(0))
	, planeUp_("planeUp", "Up")
	, offset0_("offset0", "Offset 0", -0.01f, -1.0f, 0.0f, 0.001f)
	, offset1_("offset1", "Offset 1", 0.01f, 0.0f, 1.0f, 0.001f)
	, planeSize_(0)
	, shader_("uv_pass_through.vert", "mpr_entry_exit_points.frag") {
	addPort(volumeInport_);
    addPort(entryPort_, "ImagePortGroup1");
    addPort(exitPort_, "ImagePortGroup1");
    addProperty(capNearClipping_);
	addProperty(planePosition_);
	addProperty(planeNormal_);
	addProperty(planeUp_);
	planeUp_.setReadOnly(true);
	addProperty(offset0_);
	addProperty(offset1_);
	shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
}

MPREntryExitPoints::~MPREntryExitPoints() {}

void MPREntryExitPoints::process() {
	auto const quad = util::makeBuffer<vec2>({ 
		{ -1.0f, -1.0f },{ 1.0f, -1.0f },{ -1.0f, 1.0f },{ 1.0f, 1.0f } 
	});

	// The problem with seemingly non-orthogonal indicator planes in the volume raycaster
	// appeared after applying spacings to the model matrix
	// This could mean that the planes are wrongly calculated in model space

	// Construct coordinate cross on plane
	const vec3 normal = normalize(planeNormal_.get());
	const vec3 up = normalize(planeUp_.get());
	const vec3 right = cross(normal, up);

	vec3 middle = planePosition_.get();

	// Calc length of up- and right-segments inside the volume to get the plane's width and height
	const auto volume = volumeInport_.getData();
	const vec3 voxels = static_cast<vec3>(volume->getDimensions());
	{
		// Let r be right vector scaled with voxel ratio
		const vec3 r = right * normalize(voxels);

		// Solve middle + t * r = 0
		float t0 = std::numeric_limits<float>::infinity();
		for (int i = 0; i < 3; i++) if (r[i] != 0.0f) t0 = glm::min(t0, fabs(-middle[i] / r[i]));

		// Solve middle + t * r = 1
		float t1 = std::numeric_limits<float>::infinity();
		for (int i = 0; i < 3; i++) if (r[i] != 0.0f) t1 = glm::min(t1, fabs((1.0f - middle[i]) / r[i]));

		planeSize_.x = t0 + t1;

		// Move to middle
		middle += ((planeSize_.x/2.0f - t0) * r);
	}
	{
		// Let u be up vector scaled with voxel ratio
		const vec3 u = up * normalize(voxels);

		// Solve middle + t * u = 0
		float t0 = std::numeric_limits<float>::infinity();
		for (int i = 0; i < 3; i++) if (u[i] != 0.0f) t0 = glm::min(t0, fabs(-middle[i] / u[i]));

		// Solve middle + t * u = 1
		float t1 = std::numeric_limits<float>::infinity();
		for (int i = 0; i < 3; i++) if (u[i] != 0.0f) t1 = glm::min(t1, fabs((1.0f - middle[i]) / u[i]));

		planeSize_.y = t0 + t1;

		// Move to middle
		middle += ((planeSize_.y/2.0f - t0) * u);
	}

	// generate entry points
	utilgl::activateAndClearTarget(*entryPort_.getEditableData().get(), ImageType::ColorOnly);
	shader_.activate();

	const auto viewportSize = entryPort_.getDimensions();
	shader_.setUniform("viewportAspect", static_cast<float>(viewportSize.x) / static_cast<float>(viewportSize.y));
	shader_.setUniform("sliceAspect", planeSize_.x / planeSize_.y);
	shader_.setUniform("middle", middle);
	shader_.setUniform("normal", normal);
	shader_.setUniform("up", up);
	shader_.setUniform("right", right);
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
