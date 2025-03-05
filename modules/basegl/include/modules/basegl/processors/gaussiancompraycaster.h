/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/ports/imageport.h>
#include <modules/opengl/shader/shader.h>

#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/camera/camera.h>
#include <inviwo/core/datastructures/camera/perspectivecamera.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <modules/opengl/texture/texture2d.h>
#include <modules/opengl/image/layergl.h>

#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/shader/shaderutils.h>

#include <modules/basegl/baseglmoduledefine.h>  // for IVW_MODULE_BASEG...

#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCo...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCo...
#include <inviwo/core/processors/processorinfo.h>                       // for ProcessorInfo
#include <inviwo/core/util/zip.h>                                       // for zipper

#include <modules/basegl/shadercomponents/backgroundcomponent.h>         // for BackgroundComponent
#include <modules/basegl/shadercomponents/cameracomponent.h>             // for CameraComponent
#include <modules/basegl/shadercomponents/entryexitcomponent.h>          // for EntryExitComponent
#include <modules/basegl/shadercomponents/isotfcomponent.h>              // for IsoTFComponent
#include <modules/basegl/shadercomponents/lightcomponent.h>              // for LightComponent
#include <modules/basegl/shadercomponents/positionindicatorcomponent.h>  // for PositionIndicato...
#include <modules/basegl/shadercomponents/raycastingcomponent.h>         // for RaycastingComponent
#include <modules/basegl/shadercomponents/sampletransformcomponent.h>    // for SampleTransformC...

#include <inviwo/core/interaction/cameratrackball.h>
#include <inviwo/core/ports/layerport.h>
#include <inviwo/core/ports/volumeport.h>          // for VolumeInport
#include <inviwo/core/processors/poolprocessor.h>  // for PoolProcessor

#include <inviwo/core/properties/cameraproperty.h>           // for CameraProperty
#include <inviwo/core/properties/eventproperty.h>            // for EventProperty
#include <inviwo/core/properties/isotfproperty.h>            // for IsoTFProperty
#include <inviwo/core/properties/optionproperty.h>           // for OptionPropertyInt
#include <inviwo/core/properties/raycastingproperty.h>       // for RaycastingProperty
#include <inviwo/core/properties/simplelightingproperty.h>   // for SimpleLightingProperty
#include <inviwo/core/properties/volumeindicatorproperty.h>  // for VolumeIndicatorProperty
#include <modules/base/processors/tfselector.h>


namespace inviwo {

class IVW_MODULE_BASEGL_API GaussianCompRayCaster : public Processor {
public:
    GaussianCompRayCaster();
    virtual void initializeResources() override;
    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    DataInport<std::vector<vec4>> points_;
    ImageOutport outport_;
    Shader shaderGaussian_;
    IntSize2Property dimensions_;
    IntSize2Property groupSize_;
    FloatProperty sigma_;
    CameraProperty cam_;
    CameraTrackball trackball_;
    IsoTFProperty isotfComposite_;
};

}  // namespace inviwo
