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

#include <modules/basegl/processors/entryexitpointsprocessor.h>
#include <inviwo/core/interaction/cameratrackball.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/io/serialization/versionconverter.h>
#include <modules/opengl/buffer/buffergl.h>
#include <modules/opengl/texture/textureutils.h>

namespace inviwo {

	const ProcessorInfo EntryExitPoints::processorInfo_{
		"org.inviwo.EntryExitPoints",  // Class identifier
		"Entry Exit Points",           // Display name
		"Mesh Rendering",          // Category
		CodeState::Stable,             // Code state
		Tags::GL,                      // Tags
	};
	const ProcessorInfo EntryExitPoints::getProcessorInfo() const { return processorInfo_; }

	EntryExitPoints::EntryExitPoints()
		: Processor()
		, inport_("geometry")
		, polyline_("polyline")
		, entryPort_("entry", DataVec4UInt16::get())
		, exitPort_("exit", DataVec4UInt16::get())
		, camera_("camera", "Camera", vec3(0.0f, 0.0f, -2.0f), vec3(0.0f, 0.0f, 0.0f),
			vec3(0.0f, 1.0f, 0.0f), &inport_)
		, capNearClipping_("capNearClipping", "Cap near plane clipping", true)
		, trackball_(&camera_)
		, entryShader_("uvPassThrough.vert", "entryPoints.frag")
		, exitShader_("uvPassThrough.vert", "exitPoints.frag")
		, enableVolumeReformation_("reformation", "Volume Reformation", false)
		, quad_{ nullptr }
	{
		addPort(inport_);
		polyline_.setOptional(true);
		addPort(polyline_);
		addPort(entryPort_, "ImagePortGroup1");
		addPort(exitPort_, "ImagePortGroup1");
		addProperty(capNearClipping_);
		addProperty(camera_);
		addProperty(trackball_);
		addProperty(enableVolumeReformation_);
		entryPort_.addResizeEventListener(&camera_);

		entryExitHelper_.getEntryExitShader().onReload(
			[this]() { invalidate(InvalidationLevel::InvalidResources); });
		entryExitHelper_.getNearClipShader().onReload(
			[this]() { invalidate(InvalidationLevel::InvalidResources); });

		entryShader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
		exitShader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });

		quad_ = util::makeBuffer<vec2>({ { -1.0f, -1.0f },{ 1.0f, -1.0f },{ -1.0f, 1.0f },{ 1.0f, 1.0f } });
	}

	EntryExitPoints::~EntryExitPoints() {}

	void EntryExitPoints::process() {
		if (enableVolumeReformation_.get() && polyline_.hasData() && polyline_.getData()) {
			if (polyline_.getData()->size() >= 2) {
				const auto n0 = accumulatedDistance_.size();
				calcAccumulatedDistance();
				const auto n1 = accumulatedDistance_.size();
				const auto rebuild = n0 != n1;

				// generate entry points
				utilgl::activateAndClearTarget(*entryPort_.getEditableData().get(), ImageType::ColorOnly);
				if (rebuild) {
					entryShader_[ShaderType::Fragment]->removeShaderDefine("NUM_PTS");
					entryShader_[ShaderType::Fragment]->addShaderDefine("NUM_PTS", std::to_string(polyline_.getData()->size()));
					entryShader_.build();
				}
				entryShader_.activate();
				const auto quadGL_ = quad_->getRepresentation<BufferGL>();
				quadGL_->enable();
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

				// generate exit points
				utilgl::activateAndClearTarget(*exitPort_.getEditableData().get(), ImageType::ColorOnly);
				if (rebuild) {
					exitShader_[ShaderType::Fragment]->removeShaderDefine("NUM_PTS");
					exitShader_[ShaderType::Fragment]->addShaderDefine("NUM_PTS", std::to_string(polyline_.getData()->size()));
					exitShader_.build();
				}
				exitShader_.activate();
				exitShader_.setUniform("pts", *(polyline_.getData()));
				exitShader_.setUniform("accumulated_distance", accumulatedDistance_);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
				quadGL_->disable();
			}
		}
		else {
			entryExitHelper_(*entryPort_.getEditableData().get(), *exitPort_.getEditableData().get(), camera_.get(),
				*inport_.getData().get(), capNearClipping_.get());
		}
	}

	void EntryExitPoints::calcAccumulatedDistance() {
		const auto polyline = polyline_.getData();
		assert(polyline);
		assert(!polyline->empty());
		accumulatedDistance_.resize(polyline->size());
		accumulatedDistance_[0] = 0.0f;
		for (int idx = 1; idx < polyline->size(); ++idx) {
			const vec3 p1 = (*polyline)[idx - 1];
			const vec3 p2 = (*polyline)[idx];
			accumulatedDistance_[idx] = accumulatedDistance_[idx - 1] + glm::distance(p1, p2);
		}

		for (int idx = 1; idx < polyline->size(); ++idx) {
			accumulatedDistance_[idx] /= accumulatedDistance_[polyline->size() - 1];
		}
	}

	void EntryExitPoints::deserialize(Deserializer& d) {
		util::renamePort(d, { { &entryPort_, "entry-points" },{ &exitPort_, "exit-points" } });
		Processor::deserialize(d);
	}

}  // namespace
