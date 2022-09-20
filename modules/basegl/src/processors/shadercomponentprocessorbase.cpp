/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2022 Inviwo Foundation
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

#include <modules/basegl/processors/shadercomponentprocessorbase.h>

#include <inviwo/core/ports/imageport.h>                      // for ImageOutport
#include <inviwo/core/processors/processor.h>                 // for Processor
#include <inviwo/core/properties/invalidationlevel.h>         // for InvalidationLevel, Invalida...
#include <inviwo/core/util/exception.h>                       // for Exception
#include <inviwo/core/util/sourcecontext.h>                   // for IVW_CONTEXT
#include <modules/basegl/shadercomponents/shadercomponent.h>  // for ShaderComponent, ShaderComp...
#include <modules/opengl/shader/shader.h>                     // for Shader, Shader::Build
#include <modules/opengl/shader/shaderobject.h>               // for ShaderObject
#include <modules/opengl/shader/shadersegment.h>              // for ShaderSegment
#include <modules/opengl/shader/shaderutils.h>                // for setUniforms
#include <modules/opengl/texture/textureunit.h>               // for TextureUnitContainer
#include <modules/opengl/texture/textureutils.h>              // for activateAndClearTarget, dea...

#include <exception>   // for exception
#include <functional>  // for __base
#include <string>      // for string
#include <tuple>       // for tuple, tuple_element<>::type

#include <fmt/core.h>    // for format
#include <fmt/format.h>  // for format_error

namespace inviwo {
class Property;
class ShaderResource;
class ShaderType;

ShaderComponentProcessorBase::ShaderComponentProcessorBase(
    const std::vector<std::pair<ShaderType, std::shared_ptr<const ShaderResource>>>&
        shaderResources,
    std::string_view identifier, std::string_view displayName)
    : Processor(identifier, displayName)
    , outport_("outport")
    , shader_{shaderResources, Shader::Build::No}
    , components_{} {

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });

    addPort(outport_, "images");
}

ShaderComponentProcessorBase::~ShaderComponentProcessorBase() = default;

void ShaderComponentProcessorBase::registerComponent(ShaderComponent& component) {
    components_.push_back(&component);
    for (const auto& [port, group] : component.getInports()) {
        addPort(*port, group);
    }
    for (auto prop : component.getProperties()) {
        addProperty(*prop);
    }
}

void ShaderComponentProcessorBase::initializeResources() {
    for (auto* comp : components_) {
        try {
            comp->initializeResources(shader_);
        } catch (...) {
            handleError("initializeResources", comp->getName());
        }
    }

    auto* fso = shader_.getFragmentShaderObject();
    fso->clearOutDeclarations();

    fso->clearSegments();
    for (auto* comp : components_) {
        try {
            for (auto&& segment : comp->getSegments()) {
                fso->addSegment(ShaderSegment{segment.placeholder, std::string(comp->getName()),
                                              segment.snippet, segment.priority});
            }
        } catch (...) {
            handleError("adding shader segments", comp->getName());
        }
    }

    shader_.build();
}

void ShaderComponentProcessorBase::process() {
    utilgl::activateAndClearTarget(outport_);
    shader_.activate();

    TextureUnitContainer units;
    utilgl::setUniforms(shader_, outport_);

    for (auto& comp : components_) {
        try {
            comp->process(shader_, units);
        } catch (...) {
            handleError("processing", comp->getName());
        }
    }

    utilgl::singleDrawImagePlaneRect();

    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

void ShaderComponentProcessorBase::handleError(std::string_view action,
                                               std::string_view componentName) const {
    try {
        throw;
    } catch (Exception& e) {
        throw Exception{fmt::format("Error while {} in shader component: {}, message: {}", action,
                                    componentName, e.getMessage()),
                        e.getContext()};

    } catch (fmt::format_error& e) {
        throw Exception{fmt::format("Error while {} in shader component: {}, fmt::format_error: {}",
                                    action, componentName, e.what()),
                        IVW_CONTEXT};
    } catch (std::exception& e) {
        throw Exception{fmt::format("Error while {} in shader component: {}, message: {}", action,
                                    componentName, e.what()),
                        IVW_CONTEXT};
    }
}

}  // namespace inviwo
