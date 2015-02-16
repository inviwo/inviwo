/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include "background.h"
#include <modules/opengl/glwrap/textureunit.h>
#include <modules/opengl/textureutils.h>
#include <modules/opengl/image/imagegl.h>

namespace inviwo {

ProcessorClassIdentifier(Background, "org.inviwo.Background");
ProcessorDisplayName(Background,  "Background");
ProcessorTags(Background, Tags::GL);
ProcessorCategory(Background, "Image Operation");
ProcessorCodeState(Background, CODE_STATE_STABLE);

Background::Background()
    : Processor()
    , inport_("inport")
    , outport_("outport")
    , backgroundStyle_("backgroundStyle", "Style", INVALID_RESOURCES)
    , color1_("color1", "Color 1", vec4(0.0f, 0.0f, 0.0f, 1.0f))
    , color2_("color2", "Color 2", vec4(1.0f))
    , checkerBoardSize_("checkerBoardSize", "Checker Board Size", ivec2(10, 10), ivec2(1, 1),
                        ivec2(256, 256))
    , switchColors_("Switch colors", "switch colors", VALID)
    , shader_(NULL) {
    addPort(inport_);
    addPort(outport_);
    backgroundStyle_.addOption("linearGradient", "Linear gradient", 0);
    backgroundStyle_.addOption("uniformColor", "Uniform color", 1);
    backgroundStyle_.addOption("checkerBoard", "Checker board", 2);
    backgroundStyle_.setCurrentStateAsDefault();
    addProperty(backgroundStyle_);
    color1_.setSemantics(PropertySemantics::Color);
    addProperty(color1_);
    color2_.setSemantics(PropertySemantics::Color);
    addProperty(color2_);
    addProperty(checkerBoardSize_);
    addProperty(switchColors_);
    switchColors_.onChange(this, &Background::switchColors);
}

Background::~Background() {
}

void Background::switchColors() {
    vec4 tmp = color1_.get();
    color1_.set(color2_.get());
    color2_.set(tmp);
}

void Background::initialize() {
    Processor::initialize();
    shader_ = new Shader("background.frag", false);
    initializeResources();
}

void Background::deinitialize() {
    delete shader_;
    shader_ = NULL;
    Processor::deinitialize();
}


bool Background::isReady() const {
    if (inport_.isConnected()) return Processor::isReady();
    return true;
}

void Background::initializeResources() {
    std::string shaderDefine;
    checkerBoardSize_.setVisible(false);

    switch (backgroundStyle_.get()) {
        case 0 : // linear gradient
            shaderDefine = "linearGradient(texCoords)";
            break;

        case 1 : // uniform color
            shaderDefine = "color1_";
            break;

        case 2 : // checker board
            shaderDefine = "checkerBoard(texCoords)";
            checkerBoardSize_.setVisible(true);
            break;
    }

    shader_->getFragmentShaderObject()->addShaderDefine("BACKGROUND_STYLE_FUNCTION", shaderDefine);

    if (inport_.hasData()) {
        shader_->getFragmentShaderObject()->addShaderDefine("SRC_COLOR", "texture(inputTex_, texCoords)");
        hadData_ = true;
    } else {
        shader_->getFragmentShaderObject()->addShaderDefine("SRC_COLOR", "vec4(0.0,0.0,0.0,0.0)");
        hadData_ = false;
    }

    shader_->build();
}

void Background::process() {
    if (inport_.hasData() != hadData_) initializeResources();

    if (inport_.hasData()) inport_.passOnDataToOutport(&outport_);

    utilgl::activateTarget(outport_, COLOR_ONLY);

    TextureUnit srcColorUnit, srcDepthUnit;
    if (inport_.hasData()) utilgl::bindColorTexture(inport_, srcColorUnit);
    
    shader_->activate();
    utilgl::setShaderUniforms(shader_, outport_, "outportParameters_");
    shader_->setUniform("inputTex_", srcColorUnit.getUnitNumber());
    shader_->setUniform("color1_", color1_.get());
    shader_->setUniform("color2_", color2_.get());
    shader_->setUniform("checkerBoardSize_", checkerBoardSize_.get());
    
    utilgl::singleDrawImagePlaneRect();
    
    shader_->deactivate();
    utilgl::deactivateCurrentTarget();
}


} // namespace
