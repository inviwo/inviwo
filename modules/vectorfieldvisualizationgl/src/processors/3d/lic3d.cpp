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

#include <modules/vectorfieldvisualizationgl/processors/3d/lic3d.h>

#include <inviwo/core/datastructures/datamapper.h>                         // for DataMapper
#include <inviwo/core/datastructures/transferfunction.h>                   // for TransferFunction
#include <inviwo/core/ports/volumeport.h>                                  // for VolumeInport
#include <inviwo/core/processors/processorinfo.h>                          // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                         // for CodeState, Cod...
#include <inviwo/core/processors/processortags.h>                          // for Tags, Tags::GL
#include <inviwo/core/properties/boolproperty.h>                           // for BoolProperty
#include <inviwo/core/properties/ordinalproperty.h>                        // for FloatProperty
#include <inviwo/core/properties/transferfunctionproperty.h>               // for TransferFuncti...
#include <inviwo/core/util/formats.h>                                      // for DataFormat
#include <inviwo/core/util/glmvec.h>                                       // for dvec2, vec4
#include <modules/basegl/processors/volumeprocessing/volumeglprocessor.h>  // for VolumeGLProcessor
#include <modules/opengl/shader/shader.h>                                  // for Shader
#include <modules/opengl/shader/shaderutils.h>                             // for setUniforms
#include <modules/opengl/texture/textureutils.h>                           // for bindAndSetUnif...
#include <modules/opengl/volume/volumeutils.h>                             // for bindAndSetUnif...

#include <string>       // for string
#include <string_view>  // for string_view
#include <type_traits>  // for remove_extent_t

#include <glm/mat3x3.hpp>  // for mat
#include <glm/matrix.hpp>  // for inverse

namespace inviwo {
class TextureUnitContainer;

const ProcessorInfo LIC3D::processorInfo_{
    "org.inviwo.LIC3D",            // Class identifier
    "LIC3D",                       // Display name
    "Vector Field Visualization",  // Category
    CodeState::Stable,             // Code state
    Tags::GL,                      // Tags
};

const ProcessorInfo LIC3D::getProcessorInfo() const { return processorInfo_; }

LIC3D::LIC3D()
    : VolumeGLProcessor("lic3d.frag")
    , vectorField_("vectorField")
    , samples_("samples", "Number of steps", 200, 3, 1000)
    , stepLength_("stepLength", "Step Length", 0.003f, 0.0001f, 0.01f, 0.0001f)
    , normalizeVectors_("normalizeVectors", "Normalize vectors", true)
    , intensityMapping_("intensityMapping", "Enable intensity remapping", false)
    , noiseRepeat_("noiseRepeat", "Noise Repeat", 2, 0.01f, 20)
    , tf_("tf", "Velocity Transfer function")
    , velocityScale_("velocityScale", "Velocity Scale (inverse)", 1, 0, 10)
    , alphaScale_("alphaScale", "Alpha Scale", 500, 0.01f, 100000, 0.01f)
    , noiseVolume_(nullptr) {
    addPort(vectorField_);

    addProperty(samples_);
    addProperty(stepLength_);
    addProperty(normalizeVectors_);
    addProperty(intensityMapping_);
    addProperty(noiseRepeat_);
    addProperty(tf_);
    addProperty(velocityScale_);
    addProperty(alphaScale_);

    tf_.get().clear();
    tf_.get().add(0.0, vec4(0, 0, 1, 1));
    tf_.get().add(0.5, vec4(1, 1, 0, 1));
    tf_.get().add(1.0, vec4(1, 0, 0, 1));

    setAllPropertiesCurrentStateAsDefault();

    this->dataFormat_ = DataVec4UInt8::get();
}

void LIC3D::preProcess(TextureUnitContainer& cont) {

    utilgl::bindAndSetUniforms(shader_, cont, *vectorField_.getData().get(), "vectorField");
    utilgl::setUniforms(shader_, samples_, stepLength_, normalizeVectors_, intensityMapping_,
                        noiseRepeat_, alphaScale_, velocityScale_);

    utilgl::bindAndSetUniforms(shader_, cont, tf_);

    shader_.setUniform("invBasis", glm::inverse(vectorField_.getData()->getBasis()));
}

void LIC3D::postProcess() {
    volume_->dataMap.valueRange = volume_->dataMap.dataRange = dvec2(0, 255);
}

}  // namespace inviwo
