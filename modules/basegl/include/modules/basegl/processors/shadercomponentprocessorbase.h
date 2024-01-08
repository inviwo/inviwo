/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2024 Inviwo Foundation
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

#include <inviwo/core/ports/imageport.h>       // for ImageOutport
#include <inviwo/core/processors/processor.h>  // for Processor
#include <modules/opengl/shader/shader.h>      // for Shader

#include <memory>       // for shared_ptr
#include <string_view>  // for string_view
#include <type_traits>  // for enable_if_t, is_base_of
#include <utility>      // for pair
#include <vector>       // for vector

namespace inviwo {
class ShaderComponent;
class ShaderResource;
class ShaderType;

/**
 * @brief Base class for processor using ShaderComponents.
 * Derived classes should register a set of ShaderComponents to customize behavior
 */
class IVW_MODULE_BASEGL_API ShaderComponentProcessorBase : public Processor {
protected:
    ShaderComponentProcessorBase(
        const std::vector<std::pair<ShaderType, std::shared_ptr<const ShaderResource>>>&
            shaderResources,
        std::string_view identifier, std::string_view displayName);
    ShaderComponentProcessorBase(const ShaderComponentProcessorBase&) = delete;
    ShaderComponentProcessorBase& operator=(const ShaderComponentProcessorBase&) = delete;
    virtual ~ShaderComponentProcessorBase();

    /**
     * Register a `ShaderComponent`s. The Inport and Properties of each compnents will be
     * added to this in the order registered.
     */
    void registerComponent(ShaderComponent& comps);

    /**
     * Register a set of `ShaderComponent`s. The Inport and Properties of each compnents will be
     * added to this in the order registered.
     *
     */
    template <typename... Ts, typename = std::enable_if_t<
                                  std::conjunction_v<std::is_base_of<ShaderComponent, Ts>...>>>
    void registerComponents(Ts&... comps) {
        (registerComponent(comps), ...);
    }

    virtual void initializeResources() override;
    virtual void process() override;

    /**
     * Handle any error while using the raycasting components.
     * Override to customize error handling.
     * The default implementation will decorate the error with more information and rethrow.
     * @param action what was done with the component
     * @param componentName the name of the component that had the error
     */
    virtual void handleError(std::string_view action, std::string_view componentName) const;

    ImageOutport outport_;
    Shader shader_;
    std::vector<ShaderComponent*> components_;
};

}  // namespace inviwo
