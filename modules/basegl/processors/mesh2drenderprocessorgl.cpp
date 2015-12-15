/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#include "mesh2drenderprocessorgl.h"

#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <inviwo/core/interaction/trackball.h>
#include <inviwo/core/rendering/meshdrawerfactory.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/common/inviwoapplication.h>

#include <modules/opengl/geometry/meshgl.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/openglutils.h>
#include <inviwo/core/datastructures/coordinatetransformer.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo Mesh2DRenderProcessorGL::processorInfo_{
    "org.inviwo.Mesh2DRenderProcessorGL",  // Class identifier
    "2D Mesh Renderer",                    // Display name
    "Geometry Rendering",                  // Category
    CodeState::Experimental,               // Code state
    Tags::GL,                              // Tags
};
const ProcessorInfo Mesh2DRenderProcessorGL::getProcessorInfo() const {
    return processorInfo_;
}

Mesh2DRenderProcessorGL::Mesh2DRenderProcessorGL()
    : Processor()
    , inport_("geometry.inport")
    , imageInport_("imageInport")
    , outport_("image.outport")
    , enableDepthTest_("enableDepthTest","Enable Depth Test" ,true)
    , shader_("mesh2drendering.vert", "mesh2drendering.frag") {
    addPort(inport_);
    addPort(imageInport_);
    addPort(outport_);
    imageInport_.setOptional(true);

    addProperty(enableDepthTest_);

    inport_.onChange(this, &Mesh2DRenderProcessorGL::updateDrawers);
}

Mesh2DRenderProcessorGL::~Mesh2DRenderProcessorGL() {}

void Mesh2DRenderProcessorGL::process() {
    if (imageInport_.isConnected()) {
        utilgl::activateTargetAndCopySource(outport_, imageInport_, ImageType::ColorDepth);
    } else {
        utilgl::activateAndClearTarget(outport_, ImageType::ColorDepth);
    }
    shader_.activate();

    mat4 proj = glm::ortho(-0.0f, 1.0f, -0.0f, 1.0f, -200.0f, 100.0f);
    shader_.setUniform("projectionMatrix", proj);

    utilgl::GlBoolState depthTest(GL_DEPTH_TEST, enableDepthTest_);

    for (auto& drawer : drawers_) {
        utilgl::setShaderUniforms(shader_, *(drawer.second->getGeometry()), "geometry_");
        drawer.second->draw();
    }

    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

void Mesh2DRenderProcessorGL::updateDrawers() {
    auto changed = inport_.getChangedOutports();
    DrawerMap temp;
    std::swap(temp, drawers_);

    std::map<const Outport*, std::vector<std::shared_ptr<const Mesh>>> data;
    for (auto& elem : inport_.getSourceVectorData()) {
        data[elem.first].push_back(elem.second);
    }

    for (auto elem : data) {
        auto ibegin = temp.lower_bound(elem.first);
        auto iend = temp.upper_bound(elem.first);

        if (util::contains(changed, elem.first) || ibegin == temp.end() ||
            static_cast<long>(elem.second.size()) !=
                std::distance(ibegin, iend)) {  // data is changed or new.

            for (auto geo : elem.second) {
                auto factory = getNetwork()->getApplication()->getMeshDrawerFactory();
                if (auto renderer = factory->create(geo.get())) {
                    drawers_.emplace(std::make_pair(elem.first, std::move(renderer)));
                }
            }
        } else {  // reuse the old data.
            drawers_.insert(std::make_move_iterator(ibegin), std::make_move_iterator(iend));
        }
    }
}

}  // namespace

