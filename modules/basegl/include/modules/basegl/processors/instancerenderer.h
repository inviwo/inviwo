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
#include <inviwo/core/interaction/cameratrackball.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/simplelightingproperty.h>
#include <inviwo/core/properties/listproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/rendering/meshdrawer.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/stdextensions.h>

#include <modules/opengl/shader/shader.h>
#include <modules/opengl/shader/shadersegment.h>

#include <vector>
#include <tuple>

namespace inviwo {

class InstanceRenderer;

namespace detail {
/**
 * Helper class for the InstanceRenderer to manage construction, destruction, binding, and setting
 * of uniforms for dynamically created ports.
 */
struct IVW_MODULE_BASEGL_API DynPortManager {
    DynPortManager(InstanceRenderer* theRenderer, std::unique_ptr<Inport> aPort,
                   std::function<size_t()> aSize, std::function<void(Shader&, size_t)> aSet,
                   std::function<void(ShaderObject&)> aAddUniform);

    DynPortManager(const DynPortManager&) = delete;
    DynPortManager& operator=(const DynPortManager&) = delete;
    DynPortManager(DynPortManager&&);
    DynPortManager& operator=(DynPortManager&&);
    ~DynPortManager();

    InstanceRenderer* renderer;
    std::unique_ptr<Inport> port;
    std::function<size_t()> size;
    std::function<void(Shader&, size_t)> set;
    std::function<void(ShaderObject&)> addUniform;
};

};  // namespace detail

class IVW_MODULE_BASEGL_API InstanceRenderer : public Processor, public PropertyOwnerObserver {
public:
    InstanceRenderer();
    virtual ~InstanceRenderer();

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void initializeResources() override;
    virtual void process() override;

private:
    void onDidAddProperty(Property* property, size_t index) override;
    void onWillRemoveProperty(Property* property, size_t index) override;

    static std::vector<std::unique_ptr<Property>> prefabs();

    MeshInport inport_;
    ImageInport background_;
    ImageOutport outport_;

    ListProperty ports_;
    std::vector<detail::DynPortManager> vecPorts_;

    CameraProperty camera_;
    CameraTrackball trackball_;
    SimpleLightingProperty lightingProperty_;

    std::array<StringProperty, 5> transforms_;
    StringProperty setupVert_;

    std::shared_ptr<StringShaderResource> vert_;
    std::shared_ptr<StringShaderResource> frag_;

    Shader shader_;
};
}  // namespace inviwo
