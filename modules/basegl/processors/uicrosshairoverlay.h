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

#ifndef IVW_UICROSSHAIROVERLAY_H
#define IVW_UICROSSHAIROVERLAY_H

#include <modules/basegl/baseglmoduledefine.h>
#include <modules/opengl/shader/shader.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/eventproperty.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/datastructures/geometry/mesh.h>

namespace inviwo {

class Image;
class ImageGL;

/** \docpage{org.inviwo.UICrosshairOverlay, Text Overlay}
 * ![](org.inviwo.UICrosshairOverlay.png?classIdentifier=org.inviwo.UICrosshairOverlay)
 *
 */

class IVW_MODULE_BASEGL_API UICrosshairOverlay : public Processor {
public:
    UICrosshairOverlay();
    virtual ~UICrosshairOverlay() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void process() override;

	// Interact with crosshair at mouse drag
	void eventSetRotateMarker(Event*);

	// Update pos on mouse move
	void eventUpdateMousePos(Event*);

	// Crosshair renderer
	void render();
	void invalidateMesh();
	void updateIndicatorMesh();

	//
	vec3 convertScreenPosToVolume(const vec2 &screenPos) const;
	vec2 convertVolPosToScreen(const vec3& volPos) const;

	// Set crosshair center from normalized viewport coords, i.e. [0,1]
	void setVolPosFromScreenPos(vec2 pos);

	// Get normalized viewport coords
	vec2 getScreenPosFromVolPos();

	void rotateNormal(); // to keep normals in sync

private:
    ImageInport inport_;
    ImageOutport outport_;

	// Center of the crosshair in volume coords
	FloatVec3Property planePosition_;

	// Normal pointing out of the image plane, used as crosshair rotation axis
	FloatVec3Property planeNormal_;

	// Up vector so you can configure the orientation of your image
	FloatVec3Property planeUp_; // to link with mprentryexitpointsprocessor

	// Rotation in world coords to keep multiple image planes in sync through property linking
	FloatMat4Property normalRotationMatrix_;

	// Rotation in local coords (axis 0,0,1)
	mat4 markerRotationMatrix_;

	//TODO use color of planes
	FloatVec4Property indicatorColor_;

	// Detect mouse click
	EventProperty mouseSetMarker_;

	// Detect mouse move
	EventProperty mousePositionTracker_;

	// Position from last mouse move event
	vec2 lastMousePos_;

	// How mouse action translates to crosshair behavior:
	// move only == NONE
	// drag center == MOVE
	// drag peripherie == ROTATE
	enum { NONE, MOVE, ROTATE } interactionState_;

	// Crosshair geo and shader
	Shader uiShader_;
	std::unique_ptr<Mesh> meshCrossHair_;
	bool meshDirty_;
};

} // namespace

#endif // IVW_UICROSSHAIROVERLAY_H
