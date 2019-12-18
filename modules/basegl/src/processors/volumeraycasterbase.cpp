/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2020 Inviwo Foundation
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

#include <modules/basegl/processors/volumeraycasterbase.h>

#include <inviwo/core/util/rendercontext.h>
#include <inviwo/core/algorithm/boundingbox.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/ostreamjoiner.h>

#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/volume/volumeutils.h>
#include <modules/opengl/shader/shadermanager.h>

#include <modules/basegl/raycasting/backgroundcomponent.h>
#include <modules/basegl/raycasting/isotfcomponent.h>
#include <modules/basegl/raycasting/lightcomponent.h>
#include <modules/basegl/raycasting/positionindicatorcomponent.h>
#include <modules/basegl/raycasting/raycastingcomponent.h>


#include <modules/basegl/raycasting/sampletransformcomponent.h>

namespace inviwo {

VolumeRaycasterBase::VolumeRaycasterBase(
    std::function<std::vector<std::unique_ptr<RaycasterComponent>>(VolumeRaycasterBase&)>
        makeComponents,
    const std::string& identifier, const std::string& displayName)
    : Processor(identifier, displayName)
    , volumePort_("volume")
    , entryPort_("entry")
    , exitPort_("exit")
    , outport_("outport")
    , camera_("camera", "Camera", util::boundingBox(volumePort_))
    , shader_{"raycasting/raycaster.frag", Shader::Build::No}
    , components_{makeComponents(*this)} {

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });

    addPort(volumePort_, "volumes");
    addPort(entryPort_, "images");
    addPort(exitPort_, "images");
    addPort(outport_, "images");

    for (auto& comp : components_) {
        for (const auto& [port, group] : comp->getInports()) {
            addPort(*port, group);
        }
        for (auto prop : comp->getProperties()) {
            addProperty(*prop);
        }
    }
    addProperty(camera_);
}

VolumeRaycasterBase::~VolumeRaycasterBase() {
    for (auto& comp : components_) {
        for (const auto& [port, group] : comp->getInports()) {
            removePort(port);
        }
        for (auto prop : comp->getProperties()) {
            removeProperty(prop);
        }
    }
}

void VolumeRaycasterBase::initializeResources() {
    utilgl::addDefines(shader_, camera_);

    for (auto& comp : components_) {
        comp->setDefines(shader_);
    }

    shader_.getFragmentShaderObject()->clearSegments();
    for (auto& comp : components_) {
        for (auto&& segment : comp->getSegments()) {
            shader_.getFragmentShaderObject()->addSegment(
                ShaderSegment{segment.type, comp->getName(), segment.snippet, segment.priority});
        }
    }

    shader_.build();
}

std::vector<std::unique_ptr<RaycasterComponent>> VolumeRaycasterBase::defaultComponents(
    VolumeRaycasterBase& raycaster) {
    std::vector<std::unique_ptr<RaycasterComponent>> res;

    res.push_back(std::make_unique<BackgroundComponent>(raycaster));
    res.push_back(std::make_unique<RaycastingComponent>());
    res.push_back(std::make_unique<IsoTFComponent>(&raycaster.volumePort_));
    res.push_back(std::make_unique<LightComponent>(&raycaster.camera_));
    res.push_back(std::make_unique<PositionIndicatorComponent>());
    res.push_back(std::make_unique<SampleTransformComponent>());

    return res;
}

void VolumeRaycasterBase::process() {
    utilgl::activateAndClearTarget(outport_);
    shader_.activate();

    TextureUnitContainer units;
    utilgl::bindAndSetUniforms(shader_, units, volumePort_);
    utilgl::bindAndSetUniforms(shader_, units, entryPort_, ImageType::ColorDepth);
    utilgl::bindAndSetUniforms(shader_, units, exitPort_, ImageType::ColorDepth);
    utilgl::setUniforms(shader_, outport_, camera_);
    for (auto& comp : components_) {
        comp->setUniforms(shader_, units);
    }

    utilgl::singleDrawImagePlaneRect();

    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

}  // namespace inviwo
