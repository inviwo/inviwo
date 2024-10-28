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

#include <modules/vectorfieldvisualizationgl/processors/datageneration/vectorfieldgenerator2d.h>

#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/imagetypes.h>
#include <modules/opengl/image/layergl.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/shader/standardshaders.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/openglutils.h>

namespace inviwo {

const ProcessorInfo VectorFieldGenerator2D::processorInfo_{
    "org.inviwo.VectorFieldGenerator2D",  // Class identifier
    "Vector Field Generator 2D",          // Display name
    "Data Creation",                      // Category
    CodeState::Stable,                    // Code state
    Tags::GL | Tag{"Layer"},              // Tags
    "Generate a 2D vector field by using separate functions in u and v direction."_unindentHelp,
};

const ProcessorInfo VectorFieldGenerator2D::getProcessorInfo() const { return processorInfo_; }

VectorFieldGenerator2D::VectorFieldGenerator2D()
    : Processor()
    , outport_{"outport"}
    , size_{"size", "Field size",
            util::ordinalCount(ivec2{16}, ivec2{1024}).set("Dimensions of the vector field"_help)}
    , domainU_("xRange", "U Domain",
               "Domain in u direction for which the functions are evaluated."_help, -1.0f, 1.0f,
               -10.0f, 10.0f)
    , domainV_("yRange", "V Domain",
               "Domain in v direction for which the functions are evaluated."_help, -1.0f, 1.0f,
               -10.0f, 10.0f)
    , xValue_("x", "U",
              "Function u(x,y) evaluated along u direction (uses GLSL functionality)."_help, "-x",
              InvalidationLevel::InvalidResources)
    , yValue_("y", "V",
              "Function v(x,y) evaluated along v direction (uses GLSL functionality)."_help, "y",
              InvalidationLevel::InvalidResources)
    , information_("Information", "Data Information")
    , basis_("Basis", "Basis and Offset")
    , shader_("vectorfieldgenerator2d.frag", Shader::Build::No) {

    addPort(outport_);

    domainU_.setSemantics(PropertySemantics::Text);
    domainV_.setSemantics(PropertySemantics::Text);
    addProperties(size_, xValue_, yValue_, domainU_, domainV_, information_, basis_);

    setAllPropertiesCurrentStateAsDefault();
}

VectorFieldGenerator2D::~VectorFieldGenerator2D() = default;

void VectorFieldGenerator2D::initializeResources() {
    shader_.getFragmentShaderObject()->addShaderDefine("U_VALUE(x,y)", xValue_.get());
    shader_.getFragmentShaderObject()->addShaderDefine("V_VALUE(x,y)", yValue_.get());

    shader_.build();
}

void VectorFieldGenerator2D::process() {
    bool reattach = false;
    
    const LayerConfig newConfig{.dimensions = size_.get(),
                                .format = DataVec2Float32::get(),
                                .swizzleMask = swizzlemasks::defaultData(0)};
    if (!layer_ || config != newConfig) {
        config = newConfig;
        layer_ = std::make_shared<Layer>(config);
        reattach = true;
        outport_.setData(layer_);
    }

    shader_.activate();
    utilgl::setUniforms(shader_, domainU_, domainV_);

    const ivec2 dim{size_.get()};
    {
        utilgl::Activate activateFbo{&fbo_};
        utilgl::ViewportState viewport{0, 0, static_cast<GLsizei>(dim.x),
                                       static_cast<GLsizei>(dim.y)};

        // We always need to ask for an editable representation, this will invalidate any other
        // representations
        auto destLayer = layer_->getEditableRepresentation<LayerGL>();
        if (reattach) {
            fbo_.attachColorTexture(destLayer->getTexture().get(), 0);
        }

        utilgl::singleDrawImagePlaneRect();
    }

    shader_.deactivate();

    const bool deserializing = getNetwork()->isDeserializing();
    basis_.updateForNewEntity(*layer_, deserializing);
    information_.updateForNewLayer(
        *layer_, deserializing ? util::OverwriteState::Yes : util::OverwriteState::No);
    basis_.updateEntity(*layer_);
    information_.updateLayer(*layer_);
}

}  // namespace inviwo
