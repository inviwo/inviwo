/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2021 Inviwo Foundation
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

#include <modules/basegl/processors/background.h>

#include <inviwo/core/util/stringconversion.h>

#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/image/imagegl.h>
#include <modules/opengl/image/layergl.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/inviwoopengl.h>

namespace inviwo {

const ProcessorInfo Background::processorInfo_{
    "org.inviwo.Background",  // Class identifier
    "Background",             // Display name
    "Image Operation",        // Category
    CodeState::Stable,        // Code state
    Tags::GL,                 // Tags
};
const ProcessorInfo Background::getProcessorInfo() const { return processorInfo_; }

Background::Background()
    : Processor()
    , inport_("inport")
    , outport_("outport")
    , backgroundStyle_("backgroundStyle", "Style",
                       {{"linearGradientVertical", "Linear gradient (Vertical)",
                         BackgroundStyle::LinearVertical},
                        {"linearGradientHorizontal", "Linear gradient (Horizontal)",
                         BackgroundStyle::LinearHorizontal},
                        {"linearGradientSpherical", "Linear gradient (Spherical)",
                         BackgroundStyle::LinearSpherical},
                        {"uniformColor", "Uniform color", BackgroundStyle::Uniform},
                        {"checkerBoard", "Checker board", BackgroundStyle::CheckerBoard}},
                       0, InvalidationLevel::InvalidResources)
    , bgColor1_("bgColor1", "Color 1", util::ordinalColor(0.0f, 0.0f, 0.0f, 1.0f))
    , bgColor2_("bgColor2", "Color 2", util::ordinalColor(1.0f, 1.0f, 1.0f, 1.0f))
    , checkerBoardSize_("checkerBoardSize", "Checker Board Size", ivec2(10, 10), ivec2(1, 1),
                        ivec2(256, 256))
    , switchColors_("switchColors", "Switch Colors", InvalidationLevel::Valid)
    , blendMode_("blendMode", "Blend mode",
                 {{"backtofront", "Back To Front (Pre-multiplied)", BlendMode::BackToFront},
                  {"alphamixing", "Alpha Mixing", BlendMode::AlphaMixing}},
                 0, InvalidationLevel::InvalidResources)
    , shader_("background.frag", Shader::Build::No) {
    addPort(inport_);
    addPort(outport_);
    inport_.setOptional(true);

    addProperty(backgroundStyle_);
    addProperty(bgColor1_);
    addProperty(bgColor2_);
    addProperty(checkerBoardSize_);
    addProperty(switchColors_);
    addProperty(blendMode_);

    switchColors_.onChange([&]() {
        vec4 tmp = bgColor1_.get();
        bgColor1_.set(bgColor2_.get());
        bgColor2_.set(tmp);
    });
    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
}

Background::~Background() = default;

void Background::initializeResources() {
    auto fs = shader_.getFragmentShaderObject();

    std::string_view bgStyleValue;
    switch (backgroundStyle_.get()) {
        default:
        case BackgroundStyle::LinearVertical:  // linear gradient
            bgStyleValue = "linearGradientVertical(texCoord)";
            checkerBoardSize_.setVisible(false);
            break;
        case BackgroundStyle::LinearHorizontal:  // linear gradient
            bgStyleValue = "linearGradientHorizontal(texCoord)";
            checkerBoardSize_.setVisible(false);
            break;
        case BackgroundStyle::LinearSpherical:  // linear spherical gradient
            bgStyleValue = "linearGradientSpherical(texCoord)";
            checkerBoardSize_.setVisible(false);
            break;
        case BackgroundStyle::Uniform:  // uniform color
            bgStyleValue = "bgColor1";
            checkerBoardSize_.setVisible(false);
            break;
        case BackgroundStyle::CheckerBoard:  // checker board
            bgStyleValue = "checkerBoard(texCoord)";
            checkerBoardSize_.setVisible(true);
            break;
    }
    fs->addShaderDefine("BACKGROUND_STYLE_FUNCTION", bgStyleValue);

    if (inport_.isReady()) {
        fs->addShaderDefine("SRC_COLOR", "texture(inportColor, texCoord)");
        // set shader inputs to match number of available color layers
        updateShaderInputs();

        fs->addShaderDefine("PICKING_LAYER");
        fs->addShaderDefine("DEPTH_LAYER");

        hadData_ = true;
    } else {
        fs->addShaderDefine("SRC_COLOR", "vec4(0)");
        fs->removeShaderDefine("PICKING_LAYER");
        fs->removeShaderDefine("DEPTH_LAYER");

        hadData_ = false;
    }

    const std::string_view blendfunc = [&]() {
        switch (blendMode_.get()) {
            case BlendMode::BackToFront:
                return "blendBackToFront";
            case BlendMode::AlphaMixing:
                [[fallthrough]];
            default:
                return "blendAlphaCompositing";
        }
    }();
    fs->addShaderDefine("BLENDFUNC", blendfunc);

    shader_.build();
}

void Background::process() {
    if (inport_.isReady() != hadData_) initializeResources();

    if (inport_.isReady()) {
        // Check data format, make sure we always have 4 channels
        auto inDataFromat = inport_.getData()->getDataFormat();
        auto format =
            DataFormatBase::get(inDataFromat->getNumericType(), 4,
                                inDataFromat->getSize() * 8 / inDataFromat->getComponents());

        if (outport_.getData()->getDataFormat() != format) {
            outport_.setData(std::make_shared<Image>(outport_.getDimensions(), format));
        }
    }
    utilgl::activateTarget(outport_, ImageType::AllLayers);

    shader_.activate();

    if (inport_.isReady()) {
        TextureUnitContainer textureUnits;

        auto image = inport_.getData();
        utilgl::bindAndSetUniforms(shader_, textureUnits, inport_, ImageType::AllLayers);

        {
            // bind all additional color layers
            const auto numColorLayers = image->getNumberOfColorLayers();
            auto imageGL = image->getRepresentation<ImageGL>();
            for (size_t i = 1; i < numColorLayers; ++i) {
                TextureUnit texUnit;
                imageGL->getColorLayerGL(i)->bindTexture(texUnit.getEnum());
                shader_.setUniform("color" + toString<size_t>(i), texUnit.getUnitNumber());
                textureUnits.push_back(std::move(texUnit));
            }
        }
    }

    utilgl::setUniforms(shader_, outport_, bgColor1_, bgColor2_, checkerBoardSize_);
    utilgl::singleDrawImagePlaneRect();
    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

void Background::updateShaderInputs() {
    const auto numColorLayers = inport_.getData()->getNumberOfColorLayers();
    auto fs = shader_.getFragmentShaderObject();

    if (numColorLayers > 1) {
        fs->addShaderDefine("ADDITIONAL_COLOR_LAYERS");

        // location 0 is reserved for FragData0, location 1 for PickingData
        size_t outputLocation = 2;

        StrBuffer buff;
        buff.append("\n");
        for (size_t i = 1; i < numColorLayers; ++i) {
            buff.append("layout(location = {}) out vec4 FragData{};\n", outputLocation++, i);
        }
        for (size_t i = 1; i < numColorLayers; ++i) {
            buff.append("uniform sampler2D color{};\n", i);
        }
        fs->addShaderDefine("ADDITIONAL_COLOR_LAYER_OUT_UNIFORMS", buff);

        buff.clear();
        for (size_t i = 1; i < numColorLayers - 1; ++i) {
            buff.append("FragData{0} = texture(color{0}, texCoord.xy); \\\n", i);
        }
        buff.append("FragData{0} = texture(color{0}, texCoord.xy); \n", numColorLayers - 1);
        fs->addShaderDefine("ADDITIONAL_COLOR_LAYER_WRITE", buff);

    } else {
        fs->removeShaderDefine("ADDITIONAL_COLOR_LAYERS");
        fs->removeShaderDefine("ADDITIONAL_COLOR_LAYER_OUT_UNIFORMS");
        fs->removeShaderDefine("ADDITIONAL_COLOR_LAYER_WRITE");
    }
}

}  // namespace inviwo
