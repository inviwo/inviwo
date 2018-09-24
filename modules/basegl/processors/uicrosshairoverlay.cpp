/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2018 Inviwo Foundation
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

#include "uicrosshairoverlay.h"

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/datastructures/image/image.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/image/imagegl.h>
#include <inviwo/core/util/assertion.h>
#include <inviwo/core/util/rendercontext.h>

#include <cctype>
#include <locale>

namespace inviwo {

const ProcessorInfo UICrosshairOverlay::processorInfo_{
    "org.inviwo.UICrosshairOverlay",  // Class identifier
    "Crosshair Overlay",              // Display name
    "Drawing",                   // Category
    CodeState::Experimental,           // Code state
    "GL",            // Tags
};
const ProcessorInfo UICrosshairOverlay::getProcessorInfo() const { return processorInfo_; }

UICrosshairOverlay::UICrosshairOverlay()
    : Processor()
    , inport_("inport")
	, volumeInport_("volumeInport")
    , outport_("outport")
	, planePosition_("planePosition", "Plane Position", vec3(0.5f), vec3(0.0f), vec3(1.0f))
	, planeNormal_("planeNormal", "Plane Normal", vec3(1.f, 0.f, 0.f), vec3(-1.f, -1.f, -1.f),
		vec3(1.f, 1.f, 1.f), vec3(0.01f, 0.01f, 0.01f))
	, indicatorColor_("indicatorColor", "Indicator Color", vec4(1.0f, 0.8f, 0.1f, 0.8f), vec4(0.0f),
		vec4(1.0f), vec4(0.01f), InvalidationLevel::InvalidOutput,
		PropertySemantics::Color)
	, mouseSetMarker_("mouseSetMarker", "Mouse Set Marker", [this](Event* e) { eventSetRotateMarker(e); },
		MouseButton::Left, MouseState::Press | MouseState::Move)
	, mousePositionTracker_("mousePositionTracker", "Mouse Position Tracker",
		[this](Event* e) { eventUpdateMousePos(e); }, MouseButton::None,
		MouseState::Move)
	, normalRotationMatrix_("Rotationmatrix", "Rotation Matrix", glm::mat4{ 1.0f })
	, markerRotationMatrix_{ 1.0f }
	, lastMousePos_{ 0.0f }
	, interactionState_(NONE)
	, uiShader_("standard.vert", "standard.frag", true)
	, meshCrossHair_()
	, meshDirty_(true)
{
    inport_.setOptional(true);

    addPort(inport_);
	volumeInport_.setOptional(true);
	addPort(volumeInport_);
    addPort(outport_);

	addProperty(planePosition_);
	planePosition_.onChange(this, &UICrosshairOverlay::invalidateMesh);
	addProperty(planeNormal_);
	planeNormal_.onChange(this, &UICrosshairOverlay::invalidateMesh);

	//TODO use the plane of the crosshair UI to generate extry/exit points for a raycaster
	// dont forget to link the normal rotation matrices

	addProperty(indicatorColor_);
	addProperty(mouseSetMarker_);
	mousePositionTracker_.setVisible(false);
	mousePositionTracker_.setCurrentStateAsDefault();
	addProperty(mousePositionTracker_);

	indicatorColor_.onChange(this, &UICrosshairOverlay::invalidateMesh);
	indicatorColor_.setSemantics(PropertySemantics::Color);

	normalRotationMatrix_.setReadOnly(true);
	addProperty(normalRotationMatrix_);

	uiShader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
}

void UICrosshairOverlay::process() {
	renderPositionIndicator();
}

void UICrosshairOverlay::renderPositionIndicator() {
	if (meshDirty_) {
		updateIndicatorMesh();
	}

	utilgl::activateTargetAndCopySource(outport_, inport_, ImageType::ColorDepth);

	MeshDrawerGL drawer(meshCrossHair_.get());

	utilgl::GlBoolState smooth(GL_LINE_SMOOTH, true);
	utilgl::BlendModeState blend(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	utilgl::LineWidthState linewidth(2.5f); // Only width 1 is guaranteed to be supported

	uiShader_.activate();
	const vec2 clipPos = getScreenPosFromVolPos() * 2.0f - 1.0f;
	uiShader_.setUniform("dataToClip", glm::translate(vec3(clipPos, 0.0f)) * markerRotationMatrix_ * glm::translate(vec3(-clipPos, 0.0f)));

	utilgl::DepthFuncState depth(GL_ALWAYS);
	drawer.draw();
	uiShader_.deactivate();
}

void UICrosshairOverlay::updateIndicatorMesh() {
	const vec2 pos = getScreenPosFromVolPos();

	const size2_t canvasSize(outport_.getDimensions());
	const vec2 indicatorSize = vec2(4.0f / canvasSize.x, 4.0f / canvasSize.y);
	const vec4 color(indicatorColor_.get());

	meshCrossHair_ = util::make_unique<Mesh>();
	meshCrossHair_->setModelMatrix(mat4(1.0f));
	// add two vertical and two horizontal lines with a gap around the selected position
	auto posBuf = util::makeBuffer<vec2>(
		{// horizontal
			vec2(-0.5f, pos.y) * 2.0f - 1.0f,
			vec2(pos.x - indicatorSize.x, pos.y) * 2.0f - 1.0f,

			vec2(pos.x + indicatorSize.x, pos.y) * 2.0f - 1.0f,
			vec2(1.5f, pos.y) * 2.0f - 1.0f,

			// vertical
			vec2(pos.x, -0.5f) * 2.0f - 1.0f,
			vec2(pos.x, pos.y - indicatorSize.y) * 2.0f - 1.0f,

			vec2(pos.x, pos.y + indicatorSize.y) * 2.0f - 1.0f,
			vec2(pos.x, 1.5f) * 2.0f - 1.0f,

			// box
			vec2(pos.x - indicatorSize.x, pos.y - indicatorSize.y) * 2.0f - 1.0f,
			vec2(pos.x + indicatorSize.x, pos.y - indicatorSize.y) * 2.0f - 1.0f,
			vec2(pos.x + indicatorSize.x, pos.y + indicatorSize.y) * 2.0f - 1.0f,
			vec2(pos.x - indicatorSize.x, pos.y + indicatorSize.y) * 2.0f - 1.0f });

	// indices for cross lines
	auto indexBuf1 =
		util::makeIndexBuffer(util::table([&](int i) { return static_cast<uint32_t>(i); }, 0, 8));

	// indices for box lines
	auto indexBuf2 =
		util::makeIndexBuffer(util::table([&](int i) { return static_cast<uint32_t>(i); }, 8, 12));

	// clear up existing attribute buffers
	// meshCrossHair_->deinitialize();
	auto colorBuf = util::makeBuffer<vec4>(std::vector<vec4>(12, color));
	meshCrossHair_->addBuffer(BufferType::ColorAttrib, colorBuf);
	meshCrossHair_->addBuffer(BufferType::PositionAttrib, posBuf);
	meshCrossHair_->addIndicies(Mesh::MeshInfo(DrawType::Lines, ConnectivityType::None), indexBuf1);

	meshCrossHair_->addIndicies(Mesh::MeshInfo(DrawType::Lines, ConnectivityType::Loop), indexBuf2);

	meshDirty_ = false;
}

void UICrosshairOverlay::invalidateMesh() { meshDirty_ = true; }

void UICrosshairOverlay::eventSetRotateMarker(Event* event) {
	auto mouseEvent = static_cast<MouseEvent*>(event);
	const auto newMousePos = vec2(mouseEvent->posNormalized());
	const auto markerPos = getScreenPosFromVolPos();
	// if drag started far from current pos, trigger rotate
	if (glm::distance(lastMousePos_, markerPos) > 0.1f) {
		interactionState_ = ROTATE;
		const vec2 newDir = glm::normalize(newMousePos - markerPos);
		const vec2 oldDir = glm::normalize(lastMousePos_ - markerPos);
		// get angle through dot and clamp because of float inaccuracies
		const float cosAlpha = glm::clamp(glm::dot(newDir, oldDir), 0.0f, 1.0f);
		const float alpha = glm::acos(cosAlpha);
		assert(!std::isnan(alpha));
		const float sign = (glm::cross(vec3(oldDir, 0), vec3(newDir, 0)).z > 0) ? 1.0f : -1.0f;
		normalRotationMatrix_ = glm::rotate(sign * alpha, planeNormal_.get());
		markerRotationMatrix_ *= glm::rotate(sign * alpha, vec3(0.0f, 0.0f, 1.0f));
	}
	else if (interactionState_ != ROTATE) { // else trigger move
		interactionState_ = MOVE;
		setVolPosFromScreenPos(newMousePos);
	}

	lastMousePos_ = newMousePos;

	invalidateMesh();
	event->markAsUsed();
}

void UICrosshairOverlay::eventUpdateMousePos(Event* event) {
	interactionState_ = NONE;
	lastMousePos_ = vec2(static_cast<MouseEvent*>(event)->posNormalized());

	/*if (!volumeInport_.hasData()) {
		return;
	}
	auto volume = volumeInport_.getData();
	auto mouseEvent = static_cast<MouseEvent*>(event);

	auto volPos = convertScreenPosToVolume(vec2(mouseEvent->posNormalized()), false);
	// convert normalized volume position to voxel coords
	const mat4 textureToIndex(volume->getCoordinateTransformer().getTextureToIndexMatrix());
	const vec4 texturePos(volPos, 1.0);
	ivec3 indexPos(ivec3(textureToIndex * texturePos));

	const ivec3 volDim(volume->getDimensions());

	bool outOfBounds = glm::any(glm::greaterThanEqual(indexPos, volDim)) ||
		glm::any(glm::lessThan(indexPos, ivec3(0)));
	if (outOfBounds) {
		normalizedSample_.set(vec4(-std::numeric_limits<float>::infinity()));
		volumeSample_.set(vec4(-std::numeric_limits<float>::infinity()));
	}
	else {
		// sample input volume at given index position
		const auto volumeRAM = volume->getRepresentation<VolumeRAM>();
		normalizedSample_.set(volumeRAM->getAsNormalizedDVec4(indexPos));
		volumeSample_.set(volumeRAM->getAsDVec4(indexPos));
	}
	event->markAsUsed();*/
}

vec2 UICrosshairOverlay::getScreenPosFromVolPos() {
	return vec2(glm::inverse(normalRotationMatrix_.get()) * vec4(planePosition_.get(), 1.0f));
}

void UICrosshairOverlay::setVolPosFromScreenPos(vec2 pos) {
	planePosition_.set(vec3(convertScreenPosToVolume(pos)));
}

vec3 UICrosshairOverlay::convertScreenPosToVolume(const vec2& screenPos, bool clamp) const {
	vec2 pos = vec2(glm::translate(vec3(0.5f, 0.5f, 0.0f)) *
		glm::translate(vec3(-0.5f, -0.5f, 0.0f)) * vec4(screenPos, 0.0f, 1.0f));

	if (clamp) {
		if ((pos.x < 0.0f) || (pos.x > 1.0f) || (pos.y < 0.0f) || (pos.y > 1.0f)) {
			pos = glm::clamp(pos, vec2(0.0f), vec2(1.0f));
		}
	}

	vec4 newpos(glm::inverse(normalRotationMatrix_.get()) * vec4(planePosition_.get(), 1.0f));
	newpos.x = pos.x;
	newpos.y = pos.y;
	newpos = normalRotationMatrix_.get() * newpos;

	if (clamp) {
		newpos = glm::clamp(newpos, vec4(0.0f), vec4(1.0f));
	}
	return vec3(newpos);
}

}  // namespace inviwo
