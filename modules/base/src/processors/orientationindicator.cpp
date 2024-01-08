/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2024 Inviwo Foundation
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

#include <inviwo/core/algorithm/markdown.h>             // for operator""_help
#include <inviwo/core/datastructures/camera/camera.h>   // for mat4, Camera
#include <inviwo/core/ports/meshport.h>                 // for MeshOutport
#include <inviwo/core/processors/processor.h>           // for Processor
#include <inviwo/core/processors/processorinfo.h>       // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>      // for CodeState, CodeState::Stable
#include <inviwo/core/processors/processortags.h>       // for Tags, Tags::CPU
#include <inviwo/core/properties/cameraproperty.h>      // for CameraProperty
#include <inviwo/core/properties/compositeproperty.h>   // for CompositeProperty
#include <inviwo/core/properties/constraintbehavior.h>  // for ConstraintBehavior, ConstraintBeh...
#include <inviwo/core/properties/optionproperty.h>      // for OptionPropertyOption, OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>     // for ordinalColor, ordinalScale, Float...
#include <inviwo/core/util/glmmat.h>                    // for mat3
#include <inviwo/core/util/glmvec.h>                    // for vec3, vec2, vec4
#include <inviwo/core/util/staticstring.h>              // for operator+
#include <modules/base/algorithm/meshutils.h>           // for arrow, sphere

#include <type_traits>  // for remove_extent_t

#include <glm/ext/matrix_transform.hpp>       // for scale
#include <glm/geometric.hpp>                  // for distance
#include <glm/gtx/scalar_multiplication.hpp>  // for operator*
#include <glm/gtx/transform.hpp>              // for scale
#include <glm/mat4x4.hpp>                     // for operator*, mat, mat<>::col_type
#include <glm/vec2.hpp>                       // for vec, operator*, operator-, operator+
#include <glm/vec3.hpp>                       // for vec, operator*, operator+, operator/
#include <glm/vec4.hpp>                       // for operator*, operator+, vec, vec<>:...

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo OrientationIndicator::processorInfo_{
    "org.inviwo.OrientationIndicator",  // Class identifier
    "Orientation Indicator",            // Display name
    "Information",                      // Category
    CodeState::Stable,                  // Code state
    Tags::CPU,                          // Tags
    "Generates a mesh with three arrows indicating the direction of the three coordinate axes."_help};

const ProcessorInfo OrientationIndicator::getProcessorInfo() const { return processorInfo_; }

OrientationIndicator::OrientationIndicator()
    : Processor()
    , outport_{"mesh", "The generated indicator mesh"_help}
    , baseColor_{"baseColor", "Base Color",
                 util::ordinalColor(vec4(1.0f)).set("Color of the central sphere"_help)}
    , xColor_{"xColor", "X axis Color",
              util::ordinalColor(vec4(1.0f, 0.0f, 0.0f, 1.0f)).set("Color of the first arrow"_help)}
    , yColor_{"yColor", "Y axis Color",
              util::ordinalColor(vec4(0.0f, 1.0f, 0.0f, 1.0f))
                  .set("Color of the second arrow"_help)}
    , zColor_{"zColor", "Z axis Color",
              util::ordinalColor(vec4(0.0f, 0.0f, 1.0f, 1.0f)).set("Color of the third arrow"_help)}
    , scale_{"scale", "Scale",
             util::ordinalScale(.05f, 1.f).set("Overall scaling factor for the widget"_help)}
    , axisScale_{"axisScale", "Axis scale",
                 util::ordinalScale(vec3(1.0f), 10.f).set("scaling factors for each arrow"_help)}
    , radius_{"radius", "Radius", util::ordinalScale(1.0f, 10.f).set("Radius of arrows"_help)}

    , location_{"location", "Location"}
    , locationType_{"locationType",
                    "Location Type",
                    "Locate the widget in image space (2D) or world space (3D)"_help,
                    {{"2d", "2D", Location::TwoD}, {"3d", "3D", Location::ThreeD}},
                    0}
    , viewCoords_{"viewCoords",
                  "View Coords",
                  "Where to put the widget in normalized image coordinates"_help,
                  vec2(0.05f),
                  {vec2(0.f), ConstraintBehavior::Immutable},
                  {vec2(1.f), ConstraintBehavior::Immutable}}
    , cam_{"cam", "Camera"}
    , offset_{"offset", "Offset",
              util::ordinalSymmetricVector(vec3(0.0f), 100.0f)
                  .set("Where to put the widget in world space"_help)} {

    addPort(outport_);
    addProperties(baseColor_, xColor_, yColor_, zColor_, scale_, radius_, axisScale_, location_);
    location_.addProperties(locationType_, viewCoords_, cam_, offset_);

    cam_.setCurrentStateAsDefault();

    auto setVisibility = [&]() {
        bool twoD = locationType_.get() == Location::TwoD;
        bool threeD = locationType_.get() == Location::ThreeD;
        viewCoords_.setVisible(twoD);
        cam_.setVisible(twoD);
        offset_.setVisible(threeD);
    };

    locationType_.onChange(setVisibility);
    setVisibility();
}

void OrientationIndicator::process() {
    if (!mesh_ || axisScale_.isModified() || radius_.isModified()) {
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
