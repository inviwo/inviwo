/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

#ifndef IVW_IMAGEBRIGHTNESSCONTRAST_H
#define IVW_IMAGEBRIGHTNESSCONTRAST_H

#include <modules/postprocessing/postprocessingmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <modules/basegl/processors/imageprocessing/imageglprocessor.h>
#include <inviwo/core/properties/ordinalproperty.h>

namespace inviwo {

/** \docpage{org.inviwo.ImageBrightnessContrast, Image Brightness Contrast}
 * ![](org.inviwo.ImageBrightnessContrast.png?classIdentifier=org.inviwo.ImageBrightnessContrast)
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
 * \class ImageBrightnessContrast
 * \brief <brief description> 
 * <Detailed description from a developer prespective>
 */
class IVW_MODULE_POSTPROCESSING_API ImageBrightnessContrast : public ImageGLProcessor { 
public:
    ImageBrightnessContrast();
    virtual ~ImageBrightnessContrast() = default;
    
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void preProcess(TextureUnitContainer &cont) override;

private:
    FloatProperty brightness_;
    FloatProperty contrast_;
};

} // namespace

#endif // IVW_IMAGEBRIGHTNESSCONTRAST_H

