/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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
#include <inviwo/core/ports/imageport.h>          // for ImageMultiInport, ImageOutport
#include <inviwo/core/properties/listproperty.h>  // for ListProperty
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/interaction/pickingstate.h>
#include <modules/base/properties/transformlistproperty.h>

#include <modules/opengl/shader/shader.h>  // for Shader

namespace inviwo {

class ImageLayout3D;

namespace detail {
/**
 * Helper class for the InstanceRenderer to manage construction, destruction, binding, and setting
 * of uniforms for dynamically created ports.
 */
struct IVW_MODULE_BASEGL_API DynImagePort {
    DynImagePort(ImageLayout3D* layoutProcessor, std::string_view identifier,
                 TransformationList* transform, FloatVec2Property* size,
                 IntSize2Property* imageSize);

    DynImagePort(const DynImagePort&) = delete;
    DynImagePort& operator=(const DynImagePort&) = delete;
    DynImagePort(DynImagePort&&) noexcept;
    DynImagePort& operator=(DynImagePort&&) noexcept;
    ~DynImagePort();

    ImageLayout3D* layoutProcessor;
    std::unique_ptr<ImageInport> port;
    TransformationList* transform;
    FloatVec2Property* size;
    IntSize2Property* imageSize;
    size2_t oldImageSize;
    std::shared_ptr<std::function<void()>> sizeCallback;

    struct Picking {
        dvec3 pres{0};
        dvec3 prev{0};
        PickingPressState state{PickingPressState::None};
    };

    Picking picking;
};

};  // namespace detail

class IVW_MODULE_BASEGL_API ImageLayout3D : public Processor, public PropertyOwnerObserver {
public:
    ImageLayout3D();
    virtual ~ImageLayout3D();

    virtual void process() override;

    virtual void propagateEvent(Event*, Outport* source) override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    void onDidAddProperty(Property* property, size_t index) override;
    void onWillRemoveProperty(Property* property, size_t index) override;

    ImageOutport outport_;
    ImageInport background_;
    ImageInport foreground_;

    ListProperty ports_;
    std::vector<detail::DynImagePort> vecPorts_;
    CameraProperty camera_;
    Shader shader_;
    Shader shaderForeground_;
};

}  // namespace inviwo
