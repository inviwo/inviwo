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

#ifndef IVW_LIGHTINGRAYCASTER_H
#define IVW_LIGHTINGRAYCASTER_H

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/properties/simplelightingproperty.h>
#include <inviwo/core/properties/simpleraycastingproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/volumeport.h>
#include <modules/opengl/shader/shader.h>

namespace inviwo {

/** \docpage{org.inviwo.LightingRaycaster, Lighting Raycaster}
 * ![](org.inviwo.LightingRaycaster.png?classIdentifier=org.inviwo.LightingRaycaster)
 *
 * ...
 * 
 * ### Inports
 *   * __volume__ ...
 *   * __exit-points__ ...
 *   * __lightVolume__ ...
 *   * __entry-points__ ...
 * 
 * ### Outports
 *   * __outport__ ...
 * 
 * ### Properties
 *   * __Raycasting__ ...
 *   * __Transfer function__ ...
 *   * __Camera__ ...
 *   * __Enable Light Color__ ...
 *   * __Lighting__ ...
 *   * __Render Channel__ ...
 *
 */
class IVW_MODULE_BASEGL_API LightingRaycaster : public Processor {
public:
    LightingRaycaster();

    InviwoProcessorInfo();

    virtual void initializeResources();

    virtual void deserialize(IvwDeserializer& d); 

protected:
    virtual void process();

    Shader shader_;

private:
    void onVolumeChange();

    VolumeInport volumePort_;
    ImageInport entryPort_;
    ImageInport exitPort_;
    VolumeInport lightVolumePort_;
    ImageOutport outport_;

    BoolProperty enableLightColor_;
    TransferFunctionProperty transferFunction_;
    OptionPropertyInt channel_;

    SimpleRaycastingProperty raycasting_;
    CameraProperty camera_;
    SimpleLightingProperty lighting_;
};

} // namespace

#endif // IVW_LIGHTINGRAYCASTER_H
