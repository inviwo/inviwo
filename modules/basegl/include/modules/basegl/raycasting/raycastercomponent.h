/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2021 Inviwo Foundation
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
#include <modules/opengl/shader/shadersegment.h>

#include <vector>
#include <string_view>
#include <tuple>

namespace inviwo {

class Property;
class Inport;
class Shader;
class TextureUnitContainer;

class IVW_MODULE_BASEGL_API RaycasterComponent {
public:
    using Placeholder = typename ShaderSegment::Placeholder;

    /**
     * Represents a placeholder in shader code that will be replaced
     */
    struct Segment {
        static constexpr Placeholder include{"#pragma IVW_INCLUDE", "include"};
        static constexpr Placeholder uniform{"#pragma IVW_UNIFORM", "uniform"};
        static constexpr Placeholder setup{"#pragma IVW_SETUP", "setup"};
        static constexpr Placeholder first{"#pragma IVW_FIRST", "first"};
        static constexpr Placeholder loop{"#pragma IVW_LOOP", "loop"};
        static constexpr Placeholder post{"#pragma IVW_POST", "post"};

        std::string snippet;             //!< The replacement code
        Placeholder placeholder = loop;  //!< The placeholder that will be replaced
        size_t priority = 1000;          //!< Different replace
    };

    virtual ~RaycasterComponent() = default;

    /**
     * @brief The name of the RaycasterComponent.
     * Will show up as the source of the line in the shaderwidget when the file is preprocessed and
     * in error messages.
     */
    virtual std::string_view getName() const = 0;

    /**
     * @brief Called from VolumeRaycasterBase::initializeResources
     * Set and shader defines and so on here. The Shader will be recompiles after this.
     * @param shader in current use
     */
    virtual void initializeResources(Shader& shader);

    /**
     * @brief Called from VolumeRaycasterBase::process
     * Set any needed uniforms here, and bind textures etc.
     * @param shader in current use
     * @param container add any used TextureUnits here
     */
    virtual void process(Shader& shader, TextureUnitContainer& container);

    /**
     * @brief Return all Inports and there port groups
     * This gets called in VolumeRaycasterBase::registerComponents which will add then to the
     * processor.
     */
    virtual std::vector<std::tuple<Inport*, std::string>> getInports() { return {}; }

    /**
     * @brief Return all Properties
     * This gets called in VolumeRaycasterBase::registerComponents which will add then to the
     * processor.
     */
    virtual std::vector<Property*> getProperties() { return {}; }

    /**
     * @brief Return all Segments to be injected into the shader.
     * VolumeRaycasterBase::initializeResources after the call to
     * RaycasterComponent::initializeResources.
     */
    virtual std::vector<Segment> getSegments() { return {}; }
};

}  // namespace inviwo
