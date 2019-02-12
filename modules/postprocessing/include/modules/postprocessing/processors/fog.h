/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#ifndef IVW_FOG_H
#define IVW_FOG_H

#include <modules/postprocessing/postprocessingmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <modules/opengl/shader/shader.h>

namespace inviwo {

/** \docpage{org.inviwo.Fog, Fog}
 * ![](org.inviwo.Fog.png?classIdentifier=org.inviwo.Fog)
 * Use any image with a proper depth channel as input. It will apply a colored fog to the
 * color-layer based on the normalized and linearized depth. The fog is computed from an exponential
 * function and the shape/curve of this exponential function is controlled by the density.
 *
 * ### Inports
 *   * __ImageInport__ Input Image.
 *
 * ### Outports
 *   * __ImageOutport__ Output Image.
 *
 * ### Properties
 *   * __Color__    The color of the fog
 *   * __Density__  The density of the fog
 *   * __Range__    range of the fog [0,1] with respect to near and far clip plane of the camera
 */

/**
 * \class Fog
 * \brief Fog post process. (Computed using depth-layer and applied to color-layer)
 */
class IVW_MODULE_POSTPROCESSING_API Fog : public Processor {
public:
    Fog();
    virtual ~Fog() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void process() override;

private:
    ImageInport input_;
    ImageOutport output_;
    FloatVec3Property color_;
    FloatProperty density_;
    FloatMinMaxProperty range_;
    CameraProperty camera_;
    Shader shader_;
};

}  // namespace inviwo

#endif  // IVW_FOG_H
