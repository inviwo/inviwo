/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2018 Inviwo Foundation
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

#ifndef IVW_VECTORFIELDGENERATOR4D_H
#define IVW_VECTORFIELDGENERATOR4D_H

#include <modules/vectorfieldvisualizationgl/vectorfieldvisualizationglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/processors/processor.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/buffer/framebufferobject.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>

namespace inviwo {

/** \docpage{org.inviwo.VectorFieldGenerator4D, Vector Field Generator 4D}
 * ![](org.inviwo.VectorFieldGenerator4D.png?classIdentifier=org.inviwo.VectorFieldGenerator4D)
 *
 * Description of the processor
 *
 *
 * ### Outports
 *   * __outport__ Describe port.
 *
 * ### Properties
 *   * __Volume size__ Describe property.
 *   * __X__ Describe property.
 *   * __Y__ Describe property.
 *   * __Z__ Describe property.
 *   * __X Range__ Describe property.
 *   * __Y Range__ Describe property.
 *   * __Z Range__ Describe property.
 *   * __T Range__ Describe property.
 *
 */
class IVW_MODULE_VECTORFIELDVISUALIZATIONGL_API VectorFieldGenerator4D : public Processor {
public:
    VectorFieldGenerator4D();
    virtual ~VectorFieldGenerator4D() = default;

    virtual void initializeResources() override;
    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    VolumeSequenceOutport outport_;

    OrdinalProperty<size4_t> size_;

    StringProperty xValue_;
    StringProperty yValue_;
    StringProperty zValue_;

    FloatMinMaxProperty xRange_;
    FloatMinMaxProperty yRange_;
    FloatMinMaxProperty zRange_;
    FloatMinMaxProperty tRange_;

    Shader shader_;
    FrameBufferObject fbo_;
};

}  // namespace inviwo

#endif  // IVW_VECTORFIELDGENERATOR4D_H
