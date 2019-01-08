/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2018 Inviwo Foundation
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

#ifndef IVW_LIGHTVOLUMEGL_H
#define IVW_LIGHTVOLUMEGL_H

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <modules/opengl/inviwoopengl.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/datastructures/light/baselightsource.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/buffer/framebufferobject.h>
#include <modules/opengl/shader/shader.h>

namespace inviwo {

/** \docpage{org.inviwo.LightVolumeGL, Light Volume}
 * ![](org.inviwo.LightVolumeGL.png?classIdentifier=org.inviwo.LightVolumeGL)
 *
 * ...
 *
 * ### Inports
 *   * __inport__ ...
 *
 * ### Outports
 *   * __outport__ ...
 *
 * ### Properties
 *   * __Light Volume Size__ ...
 *   * __Support Light Color__ ...
 *   * __Float Precision__ ...
 *   * __Transfer function__ ...
 *
 */
class IVW_MODULE_BASEGL_API LightVolumeGL : public Processor {
public:
    LightVolumeGL();
    virtual ~LightVolumeGL() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    void propagation3DTextureParameterFunction(Texture*);

protected:
    void process() override;

    struct PropagationParameters {
        std::unique_ptr<FrameBufferObject> fbo;
        std::unique_ptr<VolumeGL> vol;
        mat4 axisPermutation;
        mat4 axisPermutationINV;
        mat4 axisPermutationLight;
        vec4 permutedLightDirection;

        PropagationParameters() : fbo(), vol() {}
        ~PropagationParameters() = default;
    };

    bool lightSourceChanged();
    bool volumeChanged(bool);

    void volumeSizeOptionChanged();
    void supportColoredLightChanged();
    void floatPrecisionChanged();

    void borderColorTextureParameterFunction();

    void updatePermuationMatrices(const vec3&, PropagationParameters*, PropagationParameters*);

private:
    VolumeInport inport_;
    VolumeOutport outport_;
    std::shared_ptr<Volume> volume_;
    DataInport<LightSource> lightSource_;

    BoolProperty supportColoredLight_;
    OptionPropertyInt volumeSizeOption_;
    TransferFunctionProperty transferFunction_;
    BoolProperty floatPrecision_;

    Shader propagationShader_;
    Shader mergeShader_;

    PropagationParameters propParams_[2];

    std::unique_ptr<FrameBufferObject> mergeFBO_;
    float blendingFactor_;

    bool internalVolumesInvalid_;
    size3_t volumeDimOut_;
    vec3 volumeDimOutF_;
    vec3 volumeDimOutFRCP_;
    vec3 volumeDimInF_;
    vec3 volumeDimInFRCP_;
    vec3 lightDir_;
    vec3 lightPos_;
    vec4 lightColor_;
    LightSourceType lightType_;
    bool calculatedOnes_;
};

}  // namespace inviwo

#endif  // IVW_LIGHTVOLUMEGL_H
