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

#include "modules/basegl/processors/uicrosshairoverlay.h"

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/rendering/meshdrawer.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/image/imagegl.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <inviwo/core/util/assertion.h>
#include <inviwo/core/util/rendercontext.h>
#include <inviwo/core/interaction/events/mousebuttons.h>
#include <inviwo/core/interaction/events/mouseevent.h>

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
    , outport_("outport")
	, planePosition_("planePosition", "Plane Position", vec3(0.5f), vec3(0.0f), vec3(1.0f))
	, planeNormal_("planeNormal", "Plane Normal", vec3(1.f, 0.f, 0.f), vec3(-1.f, -1.f, -1.f),
		vec3(1.f, 1.f, 1.f), vec3(0.01f, 0.01f, 0.01f))
	, planeUp_("planeUp", "Plane Up", vec3(0.f, 0.f, 1.f), vec3(-1.f, -1.f, -1.f),
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
	, meshDirty_(true) {
    inport_.setOptional(true);
    addPort(inport_);
    addPort(outport_);
	addProperty(planePosition_);
	planePosition_.onChange(this, &UICrosshairOverlay::invalidateMesh);
	addProperty(planeNormal_);
	planeNormal_.setReadOnly(true);
	addProperty(planeUp_);
	planeUp_.setReadOnly(true);
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
	normalRotationMatrix_.onChange(this, &UICrosshairOverlay::rotateNormal);
	uiShader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
}

void UICrosshairOverlay::process() {
	render();
}

void UICrosshairOverlay::rotateNormal() {
	const auto planeNormalNormalized = glm::normalize(planeNormal_.get());
	const auto rotated = normalRotationMatrix_.get() * vec4 { planeNormalNormalized, 0.0f };
	planeNormal_.set(glm::normalize(vec3{ rotated }));
	const auto upRotated = normalRotationMatrix_.get() * vec4(glm::normalize(planeUp_.get()), 0.0f);
	planeUp_.set(glm::normalize(vec3(upRotated)));
}


////////////////////////////////////////////////////////////////////////////////////////////////////////
// Interaction handlers:
////////////////////////////////////////////////////////////////////////////////////////////////////////

void UICrosshairOverlay::eventSetRotateMarker(Event* event) {
	auto mouseEvent = static_cast<MouseEvent*>(event);
	const auto newMousePos = vec2(mouseEvent->posNormalized());
	const auto markerPos = getScreenPosFromVolPos();
	// Rotate if mouse drag started far from crosshair center
	const auto d = glm::distance(lastMousePos_, markerPos);
	if (d > 0.1f) {
		// Get direction change
		const vec2 newDir = glm::normalize(newMousePos - markerPos);
		const vec2 oldDir = glm::normalize(lastMousePos_ - markerPos);
		// Get angle
		// need clamp because of float inaccuracies
		const float cosAlpha = glm::clamp(glm::dot(newDir, oldDir), 0.0f, 1.0f);
		const float alpha = glm::acos(cosAlpha);
		assert(!std::isnan(alpha));
		const float sign = (glm::cross(vec3(oldDir, 0), vec3(newDir, 0)).z > 0) ? 1.0f : -1.0f;
		normalRotationMatrix_ = glm::rotate(sign * alpha, planeNormal_.get());
		markerRotationMatrix_ *= glm::rotate(sign * alpha, vec3(0.0f, 0.0f, 1.0f));
		interactionState_ = ROTATE;
		//TODO keep position fixed when rotating
	}
	// Move if crosshair center is dragged
	else if (interactionState_ == NONE || interactionState_ == MOVE) {
		setVolPosFromScreenPos(newMousePos);
		interactionState_ = MOVE;
	}
	invalidateMesh();
	event->markAsUsed();
	lastMousePos_ = newMousePos;
}

void UICrosshairOverlay::eventUpdateMousePos(Event* event) {
	interactionState_ = NONE;
	lastMousePos_ = vec2(static_cast<MouseEvent*>(event)->posNormalized());
	event->markAsUsed();
}



////////////////////////////////////////////////////////////////////////////////////////////////////////
// Rendering:
////////////////////////////////////////////////////////////////////////////////////////////////////////

void UICrosshairOverlay::render() {
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




////////////////////////////////////////////////////////////////////////////////////////////////////////
// Helper functions:
////////////////////////////////////////////////////////////////////////////////////////////////////////

vec2 UICrosshairOverlay::getScreenPosFromVolPos() {
	return convertVolPosToScreen(clamp(planePosition_.get(), vec3(0,0,0), vec3(1,1,1)));
}

void UICrosshairOverlay::setVolPosFromScreenPos(vec2 pos) {
	pos = clamp(pos, vec2(0, 0), vec2(1, 1));
	planePosition_.set(vec3(convertScreenPosToVolume(pos)));
}


//TODO implement proper plane volume projections (with a reasonable plane origin)

vec2 UICrosshairOverlay::convertVolPosToScreen(const vec3& volPos) const {
	assert(volPos.x >= 0 && volPos.x <= 1);
	assert(volPos.y >= 0 && volPos.y <= 1);
	assert(volPos.z >= 0 && volPos.z <= 1);
	return vec2(volPos);

	// screen params
	/*const float width = 2.0f, height = 2.0f;
	const vec3 n = planeNormal_.get();
	const vec3 up = planeUp_.get();
	const vec3 right = -cross(n, up);*/

	// use quick arbitrary origin
	//const vec3 o = vec3(0.5f, 0.5f, 0.5f) + n * 2.0f;

	// shortcut for matrix mult
	//const float x = volPos.x * right.x + volPos.y * up.x + volPos.z * n.x;
	//const float y = volPos.x * right.y + volPos.y * up.y + volPos.z * n.y;
	//return vec2(x / width / 0.5f + 0.5f, y / height / 0.5f + 0.5f);

	// conventional OpenGL-style transform with orthogonal projection without near/far
	/*const mat4 view = mat4(
		vec4(right, 1),
		vec4(up, 1),
		vec4(-n, 1),
		vec4(0, 0, 0, 1));
	const mat4 proj = mat4(
		vec4(1.0f / width, 0, 0, 0),
		vec4(0, 1.0f / height, 0, 0),
		vec4(0, 0, 1, 0),
		vec4(0, 0, 0, 1));*/

	// project point to plane
	//const vec3 tmp = volPos - o;
	//const float d = tmp.x*n.x + tmp.y*n.y + tmp.z*n.z;
	//return vec2(volPos - d * n);
}

vec3 UICrosshairOverlay::convertScreenPosToVolume(const vec2& screenPos) const {
	/*assert(screenPos.x >= 0 && screenPos.x <= 1);
	assert(screenPos.y >= 0 && screenPos.y <= 1);

	LogInfo(planeNormal_.get());
	LogInfo(planeUp_.get());

	const float width = 2.0f, height = 2.0f;
	const vec3 n = planeNormal_.get();
	const vec3 up = planeUp_.get();
	const vec3 right = -cross(n, up);

	const mat4 view = mat4(
		vec4(right, 1),
		vec4(up, 1),
		vec4(-n, 1),
		vec4(0, 0, 0, 1));
	const mat4 proj = mat4(
		vec4(1.0f / width, 0, 0, 0),
		vec4(0, 1.0f / height, 0, 0),
		vec4(0, 0, 1, 0),
		vec4(0, 0, 0, 1));

	const mat4 m = inverse(proj * view);
	return vec3(m * vec4(screenPos, 0, 1));

	const float x = screenPos.x;
	const float y = screenPos.y;
	return x * right + y * up;*/

	vec2 pos = vec2(glm::translate(vec3(0.5f, 0.5f, 0.0f)) *
		glm::translate(vec3(-0.5f, -0.5f, 0.0f)) * vec4(screenPos, 0.0f, 1.0f));

	vec4 newpos(inverse(normalRotationMatrix_.get()) * vec4(planePosition_.get(), 1.0f));
	newpos.x = pos.x;
	newpos.y = pos.y;
	newpos = normalRotationMatrix_.get() * newpos;

	return vec3(newpos);
}

}  // namespace inviwo
