/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#ifndef IVW_TMIP_H
#define IVW_TMIP_H

#include <modules/vectorfieldvisualizationgl/vectorfieldvisualizationglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/volumeport.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/buffer/framebufferobject.h>

namespace inviwo {

/** \docpage{org.inviwo.TMIP, TMIP}
 * ![](org.inviwo.TMIP.png?classIdentifier=org.inviwo.TMIP)
 * Explanation of how to use the processor.
 *
 * ### Inports
 *   * __<Inport1>__ <description>.
 *
 * ### Outports
 *   * __<Outport1>__ <description>.
 *
 * ### Properties
 *   * __<Prop1>__ <description>.
 *   * __<Prop2>__ <description>
 */

/**
 * \class TMIP
 * \brief <brief description>
 * <Detailed description from a developer prespective>
 */
class IVW_MODULE_VECTORFIELDVISUALIZATIONGL_API TMIP : public Processor {
public:

    enum class OutputType {
        Scalar , 
        HighestVelocity
    };

    TMIP();
    virtual ~TMIP() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void process() override;

    virtual void initializeResources() override;

private:
    void initShader(Shader &s, int samplers);
    std::shared_ptr<Volume> iteration(Shader &s, std::shared_ptr<Volume> vol, std::shared_ptr<Volume> target,
                                      std::vector<std::shared_ptr<Volume>>::const_iterator start,
                                      std::vector<std::shared_ptr<Volume>>::const_iterator end);

    int maxSamplers_;

    DataInport<std::vector<std::shared_ptr<Volume>>> inport_;
    VolumeOutport outport_;


    TemplateOptionProperty<OutputType> outputType_;

    std::shared_ptr<Volume> volume0_;
    std::shared_ptr<Volume> volume1_;

    Shader shader_;
    Shader shaderLast_;
    FrameBufferObject fbo_;
};

}  // namespace

#endif  // IVW_TMIP_H
