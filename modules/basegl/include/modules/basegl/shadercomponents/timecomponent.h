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

#include <inviwo/core/properties/boolcompositeproperty.h>     // for BoolCompositeProperty
#include <inviwo/core/properties/boolproperty.h>              // for BoolProperty
#include <inviwo/core/properties/invalidationlevel.h>         // for InvalidationLevel
#include <inviwo/core/properties/ordinalproperty.h>           // for IntProperty
#include <inviwo/core/util/timer.h>                           // for Timer
#include <modules/basegl/shadercomponents/shadercomponent.h>  // for ShaderComponent

#include <functional>   // for function
#include <string>       // for string
#include <string_view>  // for string_view
#include <vector>       // for vector

namespace inviwo {
class Property;
class Shader;
class TextureUnitContainer;

/**
 * Adds a ´<name>´ float uniform to the shader with the current "time" in milliseconds, it gets
 * updated using a timer every 33ms.
 */
class IVW_MODULE_BASEGL_API TimeComponent : public ShaderComponent {
public:
    TimeComponent(std::string_view name, std::function<void(InvalidationLevel)> invalidate);

    virtual std::string_view getName() const override;

    virtual void process(Shader& shader, TextureUnitContainer&) override;

    virtual std::vector<Segment> getSegments() override;
    virtual std::vector<Property*> getProperties() override;

    void start();
    void stop();
    void setRunning(bool run);
    bool getRunning() const;

private:
    std::string name_;
    BoolCompositeProperty enabled_;
    BoolProperty running_;
    IntProperty intervalMs_;
    Timer timer_;
};

}  // namespace inviwo
