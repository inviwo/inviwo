/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include "volumecombiner.h"
#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/glwrap/textureunit.h>
#include <inviwo/core/util/shuntingyard.h>
#include <modules/opengl/textureutils.h>
#include <modules/opengl/shaderutils.h>
#include <modules/opengl/volumeutils.h>

namespace inviwo {

ProcessorClassIdentifier(VolumeCombiner, "org.inviwo.VolumeCombiner");
ProcessorDisplayName(VolumeCombiner, "Volume Combiner");
ProcessorTags(VolumeCombiner, Tags::GL);
ProcessorCategory(VolumeCombiner, "Volume Operation");
ProcessorCodeState(VolumeCombiner, CODE_STATE_EXPERIMENTAL);

VolumeCombiner::VolumeCombiner()
    : Processor()
    , inport_("inport")
    , outport_("outport")
    , eqn_("eqn", "Equation", "s1*v1")
    , scales_("scales", "Scale factors")
    , shader_("volume_gpu.vert", "volume_gpu.geom", "volume_combiner.frag", false)
    , fbo_()
    , validEquation_(false) {

    addPort(inport_);
    addPort(outport_);
    addProperty(eqn_);
    addProperty(scales_);

    inport_.onChange([this]() {
        size_t i = 0;
        for (const auto& item : inport_.getSourceVectorData()) {
            if (scales_.getProperties().size() <= i) {
                auto p = new FloatProperty(
                    "scale" + (i == 0 ? "" : toString(i)),
                    "s" + toString(i + 1) + ", " + item.first->getProcessor()->getIdentifier(),
                    1.0f, -2.f, 2.f, 0.01f);
                p->setSerializationMode(PropertySerializationMode::ALL);
                scales_.addProperty(p);
            } else {
                scales_.getProperties()[i]->setDisplayName(
                    "s" + toString(i + 1) + ", " + item.first->getProcessor()->getIdentifier());
            }
            i++;
        }
        while (scales_.getProperties().size() > i) {
            auto p = scales_.removeProperty(scales_.getProperties().back());
            delete p;
        }
        buildEquation();
    });

    eqn_.onChange([this]() { 
        if (Processor::isReady()) {
            buildEquation(); 
        }
    });
}

void VolumeCombiner::initialize() {
    Processor::initialize();
    buildEquation();
}

bool VolumeCombiner::isReady() const  {
    return Processor::isReady() && validEquation_;
}

void VolumeCombiner::buildEquation() {
    try {
        std::map<std::string, double> vars = {};
        std::map<std::string, std::string> symbols;
        std::stringstream uniforms;
        std::stringstream sample;
        
        int i = 0;
        for (const auto& dummy : inport_) {
            const std::string id(i == 0 ? "" : toString(i));
            
            symbols["s" + toString(i + 1)] = "scale" + id;
            symbols["v" + toString(i + 1)] = "vol" + id;

            uniforms << "uniform sampler3D volume" << id << ";";
            uniforms << "uniform VolumeParameters volume" << id << "Parameters;";
            uniforms << "uniform float scale" << id << ";";

            sample << "vec4 vol" << id << "= getNormalizedVoxel(volume" << id << ", volume" << id
                   << "Parameters, texCoord_.xyz);";
            i++;
        }

        std::string eqn = shuntingyard::Calculator::shaderCode(eqn_.get(), vars, symbols);

        shader_.getFragmentShaderObject()->addShaderDefine("GEN_UNIFORMS", uniforms.str());
        shader_.getFragmentShaderObject()->addShaderDefine("GEN_SAMPLING", sample.str());
        shader_.getFragmentShaderObject()->addShaderDefine("EQUATION", eqn);
        shader_.build();
        validEquation_ = true;

    } catch (Exception& e) {
        validEquation_ = false;
        LogProcessorError("Error: " << e.getMessage());
    }
}

void VolumeCombiner::process() {
    if (inport_.isChanged()) {
        const DataFormatBase* format = inport_.getData()->getDataFormat();
        Volume* volume = new Volume(inport_.getData()->getDimensions(), format);
        volume->setModelMatrix(inport_.getData()->getModelMatrix());
        volume->setWorldMatrix(inport_.getData()->getWorldMatrix());
        // pass on metadata
        volume->copyMetaDataFrom(*inport_.getData());
        volume->dataMap_.dataRange = inport_.getData()->dataMap_.dataRange;
        volume->dataMap_.valueRange = inport_.getData()->dataMap_.valueRange;
        outport_.setData(volume);
    }
    shader_.activate();

    TextureUnitContainer cont;
    int i = 0;
    for (const auto& vol : inport_) {
        utilgl::bindAndSetUniforms(&shader_, cont, &vol, "volume" + (i == 0 ? "" : toString(i)));
        i++;
    }

    for (auto prop : scales_.getProperties()) {
        const FloatProperty& prop2 = *static_cast<FloatProperty*>(prop);
        utilgl::setUniforms(&shader_, prop2);
    }

    const size3_t dim{inport_.getData()->getDimensions()};
    fbo_.activate();
    glViewport(0, 0, static_cast<GLsizei>(dim.x), static_cast<GLsizei>(dim.y));

    if (inport_.isChanged()) {
        VolumeGL* outVolumeGL = outport_.getData()->getEditableRepresentation<VolumeGL>();
        fbo_.attachColorTexture(outVolumeGL->getTexture().get(), 0);
    }

    utilgl::multiDrawImagePlaneRect(static_cast<int>(dim.z));

    shader_.deactivate();
    fbo_.deactivate();
}



}  // namespace
