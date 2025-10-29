/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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

#include <modules/basegl/baseglmoduledefine.h>  // for IVW_MODULE_BASEGL_API

#include <inviwo/core/ports/imageport.h>                     // for ImageInport, ImageOutport
#include <inviwo/core/ports/volumeport.h>                    // for VolumeInport
#include <inviwo/core/processors/poolprocessor.h>            // for PoolProcessor
#include <inviwo/core/processors/processorinfo.h>            // for ProcessorInfo
#include <inviwo/core/properties/cameraproperty.h>           // for CameraProperty
#include <inviwo/core/properties/eventproperty.h>            // for EventProperty
#include <inviwo/core/properties/isotfproperty.h>            // for IsoTFProperty
#include <inviwo/core/properties/optionproperty.h>           // for OptionPropertyInt
#include <inviwo/core/properties/raycastingproperty.h>       // for RaycastingProperty
#include <inviwo/core/properties/simplelightingproperty.h>   // for SimpleLightingProperty
#include <inviwo/core/properties/volumeindicatorproperty.h>  // for VolumeIndicatorProperty
#include <modules/opengl/shader/shader.h>                    // for Shader

#include <modules/opengl/uniform/uniform.h>

namespace inviwo {
class Deserializer;
class Event;
class Volume;

class IVW_MODULE_BASEGL_API VolumeRaycaster : public PoolProcessor {
public:
    VolumeRaycaster();
    virtual ~VolumeRaycaster() = default;

    virtual void initializeResources() override;

    // override to do member renaming.
    virtual void deserialize(Deserializer& d) override;
    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void process() override;
    void raycast(const Volume& volume);

    void toggleShading(Event*);

    Shader shader_;
    VolumeInport volumePort_;
    ImageInport entryPort_;
    ImageInport exitPort_;
    ImageInport backgroundPort_;
    ImageOutport outport_;

    uniform::UniformWrapper<OptionPropertyInt> channel_;
    uniform::UniformWrapper<RaycastingProperty> raycasting_;
    uniform::UniformWrapper<IsoTFProperty> isotf_;
    uniform::UniformWrapper<CameraProperty> camera_;
    uniform::UniformWrapper<SimpleLightingProperty> lighting_;
    uniform::UniformWrapper<VolumeIndicatorProperty> indicator_;
    EventProperty toggleShading_;

    uniform::UniformManager<Volume> volumeUniforms_;
    uniform::UniformManager<Image> entryUniforms_;
    uniform::UniformManager<Image> exitUniforms_;
    uniform::UniformManager<Image> backgroundUniforms_;
    uniform::UniformManager<Image> outUniforms_;
    uniform::UniformManager<bool> useNormals_;

    TextureNumberCounter texCounter_{};
    TextureNumber volumeNum{texCounter_};
    TextureNumber tfNum{texCounter_};
    TextureNumbers<4> entryNums{texCounter_};
    TextureNumbers<2> exitNums{texCounter_};
    TextureNumbers<3> backgroundNums{texCounter_};
};

}  // namespace inviwo
