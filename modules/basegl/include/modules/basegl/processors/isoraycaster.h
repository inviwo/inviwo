/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2022 Inviwo Foundation
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

#pragma once

#include <modules/basegl/baseglmoduledefine.h>                // for IVW_MODULE_BASEGL_API

#include <inviwo/core/ports/imageport.h>                      // for ImageInport, ImageOutport
#include <inviwo/core/ports/volumeport.h>                     // for VolumeInport
#include <inviwo/core/processors/processor.h>                 // for Processor
#include <inviwo/core/processors/processorinfo.h>             // for ProcessorInfo
#include <inviwo/core/properties/cameraproperty.h>            // for CameraProperty
#include <inviwo/core/properties/optionproperty.h>            // for OptionPropertyInt
#include <inviwo/core/properties/ordinalproperty.h>           // for FloatVec4Property
#include <inviwo/core/properties/simplelightingproperty.h>    // for SimpleLightingProperty
#include <inviwo/core/properties/simpleraycastingproperty.h>  // for SimpleRaycastingProperty
#include <modules/opengl/shader/shader.h>                     // for Shader

namespace inviwo {
class Deserializer;

/** \docpage{org.inviwo.ISORaycaster, ISO Raycaster}
 * ![](org.inviwo.ISORaycaster.png?classIdentifier=org.inviwo.ISORaycaster)
 *
 * ...
 *
 * ### Inports
 *   * __volume__ ...
 *   * __exit-points__ ...
 *   * __entry-points__ ...
 *
 * ### Outports
 *   * __outport__ ...
 *
 * ### Properties
 *   * __Raycasting__ ...
 *   * __Lighting__ ...
 *   * __Render Channel__ ...
 *   * __Camera__ ...
 *
 */
class IVW_MODULE_BASEGL_API ISORaycaster : public Processor {
public:
    ISORaycaster();

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void initializeResources() override;

    // override to do member renaming.
    virtual void deserialize(Deserializer& d) override;

protected:
    virtual void process() override;
    Shader shader_;

private:
    VolumeInport volumePort_;
    ImageInport entryPort_;
    ImageInport exitPort_;
    ImageInport backgroundPort_;
    ImageOutport outport_;

    FloatVec4Property surfaceColor_;

    OptionPropertyInt channel_;

    SimpleRaycastingProperty raycasting_;
    CameraProperty camera_;
    SimpleLightingProperty lighting_;

private:
};

}  // namespace inviwo
