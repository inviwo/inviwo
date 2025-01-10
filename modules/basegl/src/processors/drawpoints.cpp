/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2025 Inviwo Foundation
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

#include <modules/basegl/processors/drawpoints.h>

#include <inviwo/core/datastructures/buffer/buffer.h>                   // for Buffer, BufferBase
#include <inviwo/core/datastructures/buffer/bufferram.h>                // for Vec2BufferRAM
#include <inviwo/core/datastructures/geometry/geometrytype.h>           // for BufferType, Buffe...
#include <inviwo/core/datastructures/geometry/mesh.h>                   // for Mesh
#include <inviwo/core/datastructures/image/imagetypes.h>                // for ImageType, ImageT...
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/interaction/events/keyboardevent.h>               // for KeyboardEvent
#include <inviwo/core/interaction/events/keyboardkeys.h>                // for KeyModifier, KeyM...
#include <inviwo/core/interaction/events/mousebuttons.h>                // for MouseStates, Mous...
#include <inviwo/core/interaction/events/mouseevent.h>                  // for MouseEvent
#include <inviwo/core/ports/imageport.h>                                // for ImageInport, Imag...
#include <inviwo/core/processors/processor.h>                           // for Processor
#include <inviwo/core/processors/processorinfo.h>                       // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                      // for CodeState, CodeSt...
#include <inviwo/core/processors/processortags.h>                       // for Tags, Tags::GL
#include <inviwo/core/properties/buttonproperty.h>                      // for ButtonProperty
#include <inviwo/core/properties/eventproperty.h>                       // for EventProperty
#include <inviwo/core/properties/invalidationlevel.h>                   // for InvalidationLevel
#include <inviwo/core/properties/ordinalproperty.h>                     // for FloatVec4Property
#include <inviwo/core/properties/propertysemantics.h>                   // for PropertySemantics
#include <inviwo/core/util/glmvec.h>                                    // for vec2, vec4
#include <modules/opengl/inviwoopengl.h>                                // for GLfloat
#include <modules/opengl/openglutils.h>                                 // for PointSizeState
#include <modules/opengl/rendering/meshdrawergl.h>                      // for MeshDrawerGL
#include <modules/opengl/shader/shader.h>                               // for Shader
#include <modules/opengl/texture/textureutils.h>                        // for activateTargetAnd...

#include <functional>     // for __base
#include <memory>         // for unique_ptr, make_...
#include <string>         // for string
#include <string_view>    // for string_view
#include <unordered_map>  // for unordered_map
#include <unordered_set>  // for unordered_set

#include <flags/flags.h>  // for any
#include <glm/vec3.hpp>   // for vec, vec<>::(anon...

namespace inviwo {
class Event;

const ProcessorInfo DrawPoints::processorInfo_{
    "org.inviwo.DrawPoints",  // Class identifier
    "Draw Points",            // Display name
    "Drawing",                // Category
    CodeState::Stable,        // Code state
    Tags::GL,                 // Tags
};
const ProcessorInfo& DrawPoints::getProcessorInfo() const { return processorInfo_; }

DrawPoints::DrawPoints()
    : Processor()
    , inport_("inputImage")
    , outport_("outputImage")
    , pointSize_("pointSize", "Point Size", 5, 1, 10)
    , pointColor_("pointColor", "Point Color", vec4(1.f))
    , clearButton_("clearButton", "Clear Drawing")
    , mouseDraw_(
          "mouseDraw", "Draw Point", [this](Event* e) { eventDraw(e); }, MouseButton::Left,
          MouseStates(flags::any), KeyModifier::Control)
    , keyEnableDraw_(
          "keyEnableDraw", "Enable Draw", [this](Event* e) { eventEnableDraw(e); }, IvwKey::D,
          KeyStates(flags::any), KeyModifier::Control)
    , points_(DrawType::Points, ConnectivityType::None)
    , pointDrawer_(&points_)
    , pointShader_("img_color.frag")
    , drawModeEnabled_(false) {
    addPort(inport_);
    addPort(outport_);

    addProperty(pointSize_);
    pointColor_.setSemantics(PropertySemantics::Color);
    addProperty(pointColor_);
    clearButton_.onChange([this]() { clearPoints(); });
    addProperty(clearButton_);

    addProperty(mouseDraw_);
    addProperty(keyEnableDraw_);

    pointShader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
    points_.addBuffer(BufferType::PositionAttrib, std::make_shared<Buffer<vec2>>());
}

DrawPoints::~DrawPoints() = default;

void DrawPoints::process() {
    utilgl::activateTargetAndCopySource(outport_, inport_, ImageType::ColorOnly);
    {
        utilgl::PointSizeState pointsize(static_cast<GLfloat>(pointSize_));
        pointShader_.activate();
        pointShader_.setUniform("color", pointColor_.get());
        pointDrawer_.draw();
        pointShader_.deactivate();
    }
    utilgl::deactivateCurrentTarget();
}

void DrawPoints::addPoint(vec2 p) {
    auto buff =
        static_cast<Vec2BufferRAM*>(points_.getBuffer(0)->getEditableRepresentation<BufferRAM>());
    buff->add(p);
}

void DrawPoints::clearPoints() {
    auto buff =
        static_cast<Vec2BufferRAM*>(points_.getBuffer(0)->getEditableRepresentation<BufferRAM>());

    buff->clear();
}

void DrawPoints::eventDraw(Event* event) {
    if (!drawModeEnabled_) return;

    auto mouseEvent = static_cast<MouseEvent*>(event);
    auto point = mouseEvent->ndc();

    addPoint(vec2(point.x, point.y));

    invalidate(InvalidationLevel::InvalidOutput);
}

void DrawPoints::eventEnableDraw(Event* event) {
    KeyboardEvent* keyEvent = static_cast<KeyboardEvent*>(event);
    drawModeEnabled_ = (keyEvent->state() != KeyState::Release);
}

}  // namespace inviwo
