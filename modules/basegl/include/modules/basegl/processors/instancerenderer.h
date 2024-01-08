/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2024 Inviwo Foundation
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

#include <inviwo/core/interaction/cameratrackball.h>        // for CameraTrackball
#include <inviwo/core/ports/imageport.h>                    // for ImageInport, ImageOutport
#include <inviwo/core/ports/meshport.h>                     // for MeshInport
#include <inviwo/core/processors/processor.h>               // for Processor
#include <inviwo/core/processors/processorinfo.h>           // for ProcessorInfo
#include <inviwo/core/properties/cameraproperty.h>          // for CameraProperty
#include <inviwo/core/properties/listproperty.h>            // for ListProperty
#include <inviwo/core/properties/propertyownerobserver.h>   // for PropertyOwnerObserver
#include <inviwo/core/properties/simplelightingproperty.h>  // for SimpleLightingProperty
#include <inviwo/core/properties/stringproperty.h>          // for StringProperty
#include <modules/opengl/shader/shader.h>                   // for Shader
#include <modules/opengl/buffer/bufferobject.h>             // for BufferGL
#include <modules/opengl/texture/textureunit.h>             // for TextureUnit, TextureUnitContai...

#include <array>       // for array
#include <cstddef>     // for size_t
#include <functional>  // for function
#include <memory>      // for unique_ptr, shared_ptr
#include <vector>      // for vector
#include <optional>

namespace inviwo {

class Inport;
class InstanceRenderer;
class Property;
class ShaderObject;
class StringShaderResource;

namespace detail {
/**
 * Helper class for the InstanceRenderer to manage construction, destruction, binding, and setting
 * of uniforms for dynamically created ports.
 */
struct IVW_MODULE_BASEGL_API DynPort {
    DynPort(InstanceRenderer* theRenderer, std::unique_ptr<Inport> aPort,
            std::function<std::optional<size_t>()> aSize, std::function<void(Shader&, size_t)> aSet,
            std::function<void(ShaderObject&)> aAddUniform);

    DynPort(const DynPort&) = delete;
    DynPort& operator=(const DynPort&) = delete;
    DynPort(DynPort&&);
    DynPort& operator=(DynPort&&);
    ~DynPort();

    InstanceRenderer* renderer;
    std::unique_ptr<Inport> port;
    std::function<std::optional<size_t>()> size;
    std::function<void(Shader&, size_t)> set;
    std::function<void(ShaderObject&)> addUniform;
};

struct IVW_MODULE_BASEGL_API DynUniform {
    DynUniform(std::function<void(Shader&, TextureUnitContainer&)> setAndBind,
               std::function<void(ShaderObject&)> addUniform);

    DynUniform(const DynUniform&) = delete;
    DynUniform& operator=(const DynUniform&) = delete;
    DynUniform(DynUniform&&);
    DynUniform& operator=(DynUniform&&);
    ~DynUniform();

    std::function<void(Shader&, TextureUnitContainer&)> setAndBind;
    std::function<void(ShaderObject&)> addUniform;
};

};  // namespace detail

class IVW_MODULE_BASEGL_API InstanceRenderer : public Processor,
                                               public PropertyOwnerObserver,
                                               public PropertyObserver {
public:
    InstanceRenderer();
    virtual ~InstanceRenderer();

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void initializeResources() override;
    virtual void process() override;

    /** Render all meshes.
     * @return The axis-aligned bounding box of all vertices if enableBoundingBoxCalc, otherwise
     * nullopt.
     */
    std::optional<mat4> render(bool enableBoundingBoxCalc);

private:
    void onDidAddProperty(Property* property, size_t index) override;
    void onWillRemoveProperty(Property* property, size_t index) override;

    virtual void onSetDisplayName(Property* property, const std::string& displayName) override;

    void onDidAddPort(Property* property);
    void onDidAddUniform(Property* property);

    void onDidRemovePort(Property* property, size_t index);
    void onDidRemoveUniform(Property* property, size_t index);

    static std::vector<std::unique_ptr<Property>> portPrefabs();
    static std::vector<std::unique_ptr<Property>> uniformPrefabs();

    MeshInport inport_;
    ImageInport background_;
    ImageOutport outport_;

    ListProperty uniforms_;
    std::vector<detail::DynUniform> dynUniforms_;
    ListProperty ports_;
    std::vector<detail::DynPort> vecPorts_;

    CameraProperty camera_;
    CameraTrackball trackball_;
    SimpleLightingProperty lightingProperty_;

    StringProperty setupVert_;
    StringProperty commonCode_;
    std::array<StringProperty, 5> transforms_;

    std::shared_ptr<StringShaderResource> vert_;
    std::shared_ptr<StringShaderResource> frag_;
    Shader shader_;
};
}  // namespace inviwo
