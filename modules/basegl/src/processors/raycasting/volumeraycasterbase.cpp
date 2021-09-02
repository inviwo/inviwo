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

#include <modules/basegl/processors/raycasting/volumeraycasterbase.h>

#include <inviwo/core/util/rendercontext.h>

#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/shader/shadermanager.h>

#include <modules/basegl/raycasting/raycastingcomponent.h>

namespace inviwo {

VolumeRaycasterBase::VolumeRaycasterBase(std::string_view identifier, std::string_view displayName)
    : Processor(identifier, displayName)
    , entryPort_("entry")
    , exitPort_("exit")
    , outport_("outport")
    , shader_{"raycasting/raycaster.frag", Shader::Build::No}
    , components_{} {

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });

    addPort(entryPort_, "images");
    addPort(exitPort_, "images");
    addPort(outport_, "images");
}

VolumeRaycasterBase::~VolumeRaycasterBase() = default;

void VolumeRaycasterBase::registerComponents(util::span<RaycasterComponent*> components) {
    components_.insert(components_.end(), components.begin(), components.end());
    for (auto* comp : components) {
        for (const auto& [port, group] : comp->getInports()) {
            addPort(*port, group);
        }
        for (auto prop : comp->getProperties()) {
            addProperty(*prop);
        }
    }
}

void VolumeRaycasterBase::initializeResources() {
    for (auto* comp : components_) {
        try {
            comp->initializeResources(shader_);
        } catch (...) {
            handleError("setting shader defines", comp->getName());
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
            handleError("adding segments", comp->getName());
        }
    }

    shader_.build();
}

void VolumeRaycasterBase::process() {
    utilgl::activateAndClearTarget(outport_);
    shader_.activate();

    TextureUnitContainer units;
    utilgl::setUniforms(shader_, outport_);
    utilgl::bindAndSetUniforms(shader_, units, entryPort_, ImageType::ColorDepth);
    utilgl::bindAndSetUniforms(shader_, units, exitPort_, ImageType::ColorDepth);
    for (auto& comp : components_) {
        try {
            comp->process(shader_, units);
        } catch (...) {
            handleError("setting shader uniforms", comp->getName());
        }
    }

    utilgl::singleDrawImagePlaneRect();

    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

void VolumeRaycasterBase::handleError(std::string_view action, std::string_view name) const {
    try {
        throw;
    } catch (Exception& e) {
        throw Exception{fmt::format("Error while {} in raycasting segment: {}, message: {}", action,
                                    name, e.getMessage()),
                        e.getContext()};

    } catch (fmt::format_error& e) {
        throw Exception{
            fmt::format("Error while {} in raycasting segment: {}, fmt::format_error: {}", action,
                        name, e.what()),
            IVW_CONTEXT};
    } catch (std::exception& e) {
        throw Exception{fmt::format("Error while {} in raycasting segment: {}, message: {}", action,
                                    name, e.what()),
                        IVW_CONTEXT};
    }
}

}  // namespace inviwo
