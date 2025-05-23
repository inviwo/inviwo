/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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

#include <inviwo/core/datastructures/image/image.h>   // for Image
#include <inviwo/core/interaction/cameratrackball.h>  // for CameraTrackball
#include <inviwo/core/interaction/pickingmapper.h>    // for PickingMapper
#include <inviwo/core/ports/imageport.h>              // for ImageInport, ImageOutport
#include <inviwo/core/ports/meshport.h>               // for MeshInport
#include <inviwo/core/processors/processor.h>         // for Processor
#include <inviwo/core/processors/processorinfo.h>     // for ProcessorInfo
#include <inviwo/core/properties/cameraproperty.h>    // for CameraProperty
#include <inviwo/core/properties/optionproperty.h>    // for OptionPropertyInt
#include <inviwo/core/properties/ordinalproperty.h>   // for FloatVec3Property, FloatVec4Property
#include <modules/opengl/image/imagecompositor.h>     // for ImageCompositor
#include <modules/opengl/shader/shader.h>             // for Shader

#include <memory>    // for shared_ptr, unique_ptr
#include <optional>  // for optional

namespace inviwo {

class Mesh;
class MeshDrawerGL;
class PickingEvent;

/**
 * @brief Composite image with geometry where geometry repositioned through picking
 */
class IVW_MODULE_BASEGL_API MeshPicking : public Processor {
public:
    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    MeshPicking();
    virtual ~MeshPicking();

    virtual void process() override;

    void render();

    void handlePickingEvent(PickingEvent*);

    void updatePosition(PickingEvent* p);

private:
    MeshInport meshInport_;
    ImageInport imageInport_;
    ImageOutport outport_;

    OptionPropertyInt cullFace_;

    FloatVec3Property position_;
    FloatVec4Property highlightColor_;

    CameraProperty camera_;
    CameraTrackball trackball_;

    PickingMapper picking_;

    Shader shader_;

    std::shared_ptr<const Mesh> mesh_;
    std::unique_ptr<MeshDrawerGL> drawer_;

    std::optional<ImageCompositor> compositor_;
    std::optional<Image> tmp_;

    bool highlight_ = false;
};

}  // namespace inviwo
