/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2019 Inviwo Foundation
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

#include <modules/base/processors/orientationindicator.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>

#include <modules/base/algorithm/meshutils.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo OrientationIndicator::processorInfo_{
    "org.inviwo.OrientationIndicator",  // Class identifier
    "Orientation Indicator",            // Display name
    "Information",                      // Category
    CodeState::Experimental,            // Code state
    Tags::None,                         // Tags
};
const ProcessorInfo OrientationIndicator::getProcessorInfo() const { return processorInfo_; }

OrientationIndicator::OrientationIndicator()
    : Processor()
    , outport_("mesh")
    , baseColor_("baseColor", "Base Color", vec4(1.0f))
    , xColor_("xColor", "X axis Color", vec4(1.0f, 0.0f, 0.0f, 1.0f))
    , yColor_("yColor", "Y axis Color", vec4(0.0f, 1.0f, 0.0f, 1.0f))
    , zColor_("zColor", "Z axis Color", vec4(0.0f, 0.0f, 1.0f, 1.0f))
    , scale_("scale", "Scale", .05f, 0.001f, 1.f, 0.001f)
    , axisScale_("axisScale", "Axis scale", vec3(1.0f), vec3(0.001f), vec3(10.f), vec3(0.001f))
    , radius_("radius", "Radius", 1.0f, 0.001f, 10.f, 0.001f)

    , location_("location", "Location")
    , locationType_("locationType", "Location Type")

    , viewCoords_("viewCoords", "View Coords", vec2(0.05f), vec2(0.f), vec2(1.f))
    , cam_("cam", "Camera")
    , offset_("offset", "Offset", vec3(0.0f), vec3(-100.0f), vec3(100.0f)) {

    addPort(outport_);
    addProperty(baseColor_);
    addProperty(xColor_);
    addProperty(yColor_);
    addProperty(zColor_);
    addProperty(scale_);
    addProperty(radius_);
    addProperty(axisScale_);
    addProperty(location_);
    location_.addProperty(locationType_);
    location_.addProperty(viewCoords_);
    location_.addProperty(cam_);
    location_.addProperty(offset_);

    cam_.setCurrentStateAsDefault();

    locationType_.addOption("2d", "2D", Location::TwoD);
    locationType_.addOption("3d", "3D", Location::ThreeD);

    auto setVisibility = [&]() {
        bool twod = locationType_.get() == Location::TwoD;
        bool threed = locationType_.get() == Location::ThreeD;
        viewCoords_.setVisible(twod);
        cam_.setVisible(twod);
        offset_.setVisible(threed);
    };

    locationType_.onChange(setVisibility);
    setVisibility();
    locationType_.setCurrentStateAsDefault();

    baseColor_.setSemantics(PropertySemantics::Color);
    xColor_.setSemantics(PropertySemantics::Color);
    yColor_.setSemantics(PropertySemantics::Color);
    zColor_.setSemantics(PropertySemantics::Color);
}

void OrientationIndicator::process() {
    if (!mesh_ || axisScale_.isModified()) {
        updateMesh();
    }

    auto start = offset_.get();
    float scale = scale_.get();

    if (locationType_.get() == Location::TwoD) {
        auto p = viewCoords_.get();
        auto p2 = viewCoords_.get() + vec2(scale, 0.0f);
        p = p * 2.0f - 1.0f;
        p2 = p2 * 2.0f - 1.0f;
        vec4 viewPos(p, 0.f, 1.f);
        vec4 viewPos2(p2, 0.f, 1.f);
        auto m = cam_.get().getInverseViewMatrix() * cam_.get().getInverseProjectionMatrix();
        auto point = m * viewPos;
        auto point2 = m * viewPos2;
        start = vec3(point) / point.w;
        auto end = vec3(point2) / point2.w;
        scale = glm::distance(start, end);
    }

    mat4 basisOffset(glm::scale(vec3(scale)));
    basisOffset[3] = vec4(start, 1.0f);
    mesh_->setModelMatrix(basisOffset);

    outport_.setData(mesh_);
}

void OrientationIndicator::updateMesh() {
    vec3 origin(0.0f);
    mat3 basis(glm::scale(axisScale_.get()));

    const static float baseRadius = 0.06f;      // radius
    const static float baseHeadradius = 0.15f;  // outer radius of arrow tips
    vec3 radius = axisScale_.get() * radius_.get() * baseRadius;
    vec3 headradius = axisScale_.get() * radius_.get() * baseHeadradius;

    // build indicator using a sphere and three arrows
    auto base = meshutil::sphere(origin, radius_.get() * baseRadius * 2, baseColor_);
    auto xArrow =
        meshutil::arrow(origin, origin + basis[0], xColor_, radius.x, 0.25, headradius.x, 64);
    auto yArrow =
        meshutil::arrow(origin, origin + basis[1], yColor_, radius.y, 0.25, headradius.y, 64);
    auto zArrow =
        meshutil::arrow(origin, origin + basis[2], zColor_, radius.z, 0.25, headradius.z, 64);
    base->append(xArrow.get());
    base->append(yArrow.get());
    base->append(zArrow.get());

    mesh_ = base;
}

}  // namespace inviwo
