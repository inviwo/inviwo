/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#include <modules/basegl/processors/imageprocessing/imagechannelcombine.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>

namespace inviwo {

    const ProcessorInfo ImageChannelCombine::processorInfo_{
        "org.inviwo.ImageChannelCombine",  // Class identifier
        "Image Channel Combine",           // Display name
        "Image Operation",                 // Category
        CodeState::Experimental,           // Code state
        Tags::GL,                          // Tags
    };
    const ProcessorInfo ImageChannelCombine::getProcessorInfo() const { return processorInfo_; }

    ImageChannelCombine::ImageChannelCombine() :
        Processor()
        , inport0_("inport0", true)
        , inport1_("inport1", true)
        , inport2_("inport2", true)
        , inport3_("inport3", true)
        , outport_("outport", false)
        , alpha_("alpha", "Alpha", 1.0f, 0.0f, 1.0f, 0.001f)
        , shader_("img_channel_combine.frag")
    {
        shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });

        addPort(inport0_);
        addPort(inport1_);
        addPort(inport2_);
        inport3_.setOptional(true);
        addPort(inport3_);

        addPort(outport_);

        addProperty(alpha_);
    }

    void ImageChannelCombine::process() {
        if (inport0_.getData()->getDimensions() != inport1_.getData()->getDimensions() ||
            inport0_.getData()->getDimensions() != inport2_.getData()->getDimensions() ||
            inport3_.isConnected() && inport0_.getData()->getDimensions() != inport3_.getData()->getDimensions()) {    
            throw Exception("The image dimensions of all inports needs to be the same", IVW_CONTEXT);
        }

        if (inport0_.isChanged() || inport1_.isChanged() || inport2_.isChanged() || inport3_.isChanged()) {
            const auto dimensions = inport0_.getData()->getDimensions();
            auto t1 = inport0_.getData()->getDataFormat()->getNumericType(); 
            auto t2 = inport1_.getData()->getDataFormat()->getNumericType(); 
            auto t3 = inport2_.getData()->getDataFormat()->getNumericType(); 
            NumericType type;
            if(t1 == t2 && t1 == t3){ 
                // All have the same numType
                type = t1;
            }else if(t1 == NumericType::Float || t2 == NumericType::Float || t2 == NumericType::Float){
                // At least one is a float type
                type = NumericType::Float;
            } else {
                // At least one are signed, and at least one are unsigned
                type = NumericType::Float;
            }

            auto p0 = inport0_.getData()->getDataFormat()->getPrecision();
            auto p1 = inport1_.getData()->getDataFormat()->getPrecision();
            auto p2 = inport2_.getData()->getDataFormat()->getPrecision();
            size_t prcession = std::max({p0,p1,p2}); 
            DataFormatBase::get(type,4,prcession);

            auto img = std::make_shared<Image>(dimensions, DataVec4UInt8::get());
            outport_.setData(img);
        }

        utilgl::activateAndClearTarget(outport_);
        shader_.activate();
        TextureUnitContainer units;
        utilgl::bindAndSetUniforms(shader_, units, inport0_, ImageType::ColorOnly);
        utilgl::bindAndSetUniforms(shader_, units, inport1_, ImageType::ColorOnly);
        utilgl::bindAndSetUniforms(shader_, units, inport2_, ImageType::ColorOnly);
        if (inport3_.hasData()) {
            utilgl::bindAndSetUniforms(shader_, units, inport3_, ImageType::ColorOnly);
        }
        shader_.setUniform("use_alpha_texture_", inport3_.hasData());
        shader_.setUniform("alpha_", alpha_.get());
        utilgl::setUniforms(shader_, outport_);
        utilgl::singleDrawImagePlaneRect();
        shader_.deactivate();
        utilgl::deactivateCurrentTarget();
    }

}  // namespace inviwo
