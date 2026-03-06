/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2026 Inviwo Foundation
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

#include <modules/postprocessing/processors/fog.h>

#include <inviwo/core/datastructures/image/imagetypes.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/processors/processorinfo.h>
#include <inviwo/core/processors/processorstate.h>
#include <inviwo/core/processors/processortags.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/invalidationlevel.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/propertysemantics.h>
#include <inviwo/core/util/glmvec.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>

#include <functional>
#include <string>
#include <string_view>

namespace inviwo {

const ProcessorInfo Fog::processorInfo_{
    "org.inviwo.Fog",       // Class identifier
    "Fog",                  // Display name
    "Postprocessing",       // Category
    CodeState::Stable,      // Code state
    "GL, Image Operation",  // Tags
    R"(Applies a depth-based fog effect to the input image using its depth information.
    The fog is computed using an exponential function and the shape/curve of this exponential
    function is controlled by the density.)"_unindentHelp,
};
const ProcessorInfo& Fog::getProcessorInfo() const { return processorInfo_; }

Fog::Fog()
    : input_("inport", "Input Image."_help)
    , output_("output", "Output Image."_help)
    , color_("color", "Color", util::ordinalColor(vec3(1.f)).set("The color of the fog"_help))
    , density_("density", "Density",
               util::ordinalLength(1.0f, 10.0f).set("The density of the fog"_help))
    , range_("range", "Range",
             "range of the fog [0,1] with respect to near and far clip plane of the camera"_help,
             0.0f, 1.0f, 0.0f, 1.0f)
    , camera_("camera", "Camera")
    , shader_("fullscreenquad.vert", "fog.frag") {
    addPort(input_);
    addPort(output_);
    addProperty(color_);
    addProperty(density_);
    addProperty(range_);
    addProperty(camera_);

    color_.setSemantics(PropertySemantics::Color);

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidOutput); });
}

void Fog::process() {
    utilgl::activateTargetAndCopySource(output_, input_, ImageType::ColorOnly);

    shader_.activate();
    shader_.setUniform("fogColor", color_);
    shader_.setUniform("fogDensity", density_);
    utilgl::setUniforms(shader_, camera_, range_);
    TextureUnitContainer cont;
    utilgl::bindAndSetUniforms(shader_, cont, input_, ImageType::ColorDepth);

    utilgl::singleDrawImagePlaneRect();

    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

}  // namespace inviwo
