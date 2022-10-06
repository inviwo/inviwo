/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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

#include <inviwo/dome/sgctmanager.h>

#include <inviwo/core/util/rendercontext.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/util/stdextensions.h>

#include <inviwo/core/interaction/events/event.h>
#include <inviwo/core/interaction/events/keyboardevent.h>
#include <inviwo/core/interaction/events/eventpropagator.h>

#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/canvasprocessor.h>
#include <inviwo/core/processors/processorwidget.h>

#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/network/networkvisitor.h>
#include <inviwo/core/network/lambdanetworkvisitor.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/network/networkutils.h>

#include <modules/opengl/canvasgl.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/image/layergl.h>

#include <modules/opengl/shader/shader.h>
#include <modules/opengl/shader/shaderresource.h>
#include <modules/opengl/shader/shadertype.h>
#include <modules/opengl/shader/shaderobject.h>

#include <modules/glfw/glfwuserdata.h>
#include <modules/glfw/glfwwindoweventmanager.h>

#include <inviwo/sgct/datastructures/sgctcamera.h>

#include <sgct/engine.h>

#include <inviwo/tracy/tracy.h>
#include <inviwo/tracy/tracyopengl.h>

#include <functional>
#include <algorithm>
#include <string_view>
#include <algorithm>

namespace inviwo {

namespace {

constexpr std::string_view copyVertStr = R"(
out vec3 texCoord;
void main() {
    texCoord = in_TexCoord;
    gl_Position = in_Vertex;
}
)";

constexpr std::string_view copyFragStr = R"(
uniform sampler2D color;
uniform sampler2D depth;
in vec3 texCoord;
void main() {
    FragData0 = texture(color, texCoord.xy);
    gl_FragDepth = texture(depth, texCoord.xy).r;
}
)";

#include <inviwo/core/network/networkvisitor.h>

}  // namespace

SgctManager::SgctManager(InviwoApplication& app) : app{app} {
    app.getProcessorNetwork()->addObserver(this);
}

SgctManager::~SgctManager() {
    app.getProcessorNetwork()->removeObserver(this);
    teardownInteraction();
}

void SgctManager::onProcessorNetworkDidAddProcessor(Processor* processor) {
    if (!processor) return;

    if (auto cp = dynamic_cast<CanvasProcessor*>(processor)) {
        if (canvases.empty()) {
            eventTarget = cp;
            cp->setEvaluateWhenHidden(true);
            if (auto widget = cp->getProcessorWidget()) {
                widget->setVisible(false);
            }
        }
        canvases.push_back(cp);
    }
    for (auto* camera : processor->getPropertiesByType<CameraProperty>(true)) {
        if (cameras.empty()) {
            sgct::Engine::instance().setNearAndFarClippingPlanes(camera->getNearPlaneDist(),
                                                                 camera->getFarPlaneDist());
        }
        cameras.push_back(camera);
    }
}

void SgctManager::onProcessorNetworkWillRemoveProcessor(Processor* processor) {
    if (!processor) return;

    if (auto cp = dynamic_cast<CanvasProcessor*>(processor)) {
        if (auto it = std::find(canvases.begin(), canvases.end(), cp); it != canvases.end()) {
            canvases.erase(it);
        }
    }
    for (auto camera : processor->getPropertiesByType<CameraProperty>(true)) {
        if (auto it = std::find(cameras.begin(), cameras.end(), camera); it != cameras.end()) {
            cameras.erase(it);
        }
    }
}

void SgctManager::setupUpInteraction(GLFWwindow* win) {
    userdata.push_back(std::make_unique<GLFWUserData>(win));
    interactionManagers.push_back(std::make_unique<GLFWWindowEventManager>(
        win,
        [this](Event* e) {
            TRACY_ZONE_SCOPED_NC("Handle Event", 0xAA0000);
            if (auto ke = e->getAs<KeyboardEvent>()) {
                if (ke->key() == IvwKey::S && ke->state() == KeyState::Release &&
                    ke->modifiers() == KeyModifier::Alt) {
                    if (onStatChange) onStatChange(true);
                    return;
                }
                if (ke->key() == IvwKey::H && ke->state() == KeyState::Release &&
                    ke->modifiers() == KeyModifier::Alt) {
                    if (onStatChange) onStatChange(false);
                    return;
                }
            }
            if (eventTarget) {
                eventTarget->propagateEvent(e, nullptr);
            }
        },
        // Todo figure out proper depth.
        [this](dvec2 p) { return -1.0; }));
}
void SgctManager::teardownInteraction() {
    interactionManagers.clear();
    userdata.clear();
}
void SgctManager::evaluate(const ::sgct::RenderData& renderData) {
    TRACY_ZONE_SCOPED_NC("Evaluate", 0xAA0000);
    TRACY_GPU_ZONE_C("Evaluate", 0xAA0000);

    CanvasGL::defaultGLState();
    {
        TRACY_ZONE_SCOPED_NC("Update cameras", 0x770000);
        const auto size = renderData.window.resolution();
        const size2_t newSize{size.x, size.y};
        if (auto canvas = getCanvas(); canvas && canvas->getCanvasSize() != newSize) {
            canvas->setCanvasSize(newSize);
        }

        for (auto* camera : cameras) {
            if (auto* cam = dynamic_cast<SGCTCamera*>(&camera->get())) {
                cam->setExternal(renderData);
            }
        }
    }

    {
        TRACY_ZONE_SCOPED_NC("Evaluate Network", 0x770000);
        TRACY_GPU_ZONE_C("Evaluate Network", 0x770000);
        auto net = app.getProcessorNetwork();
        net->unlock();
        // network evaluated
        net->lock();
    }
}

void SgctManager::createShader() {
    const auto vert = std::make_shared<StringShaderResource>("DomeCopy.vert", copyVertStr);
    const auto frag = std::make_shared<StringShaderResource>("DomeCopy.frag", copyFragStr);

    using S = std::vector<std::pair<ShaderType, std::shared_ptr<const ShaderResource>>>;
    copyShader = std::make_unique<Shader>(
        S{{ShaderType::Vertex, vert}, {ShaderType::Fragment, frag}}, Shader::Build::Yes);
}
void SgctManager::copy() {  // Copy inviwo output
    TRACY_ZONE_SCOPED_NC("Copy", 0xAA0000);
    TRACY_GPU_ZONE_C("Copy", 0xAA0000);

    if (!copyShader) createShader();

    if (auto canvas = getCanvas()) {
        if (auto img = canvas->inport_.getData()) {
            TextureUnit colorUnit;
            TextureUnit depthUnit;
            img->getColorLayer(0)->getRepresentation<LayerGL>()->bindTexture(colorUnit.getEnum());
            img->getDepthLayer()->getRepresentation<LayerGL>()->bindTexture(depthUnit.getEnum());

            copyShader->activate();
            copyShader->setUniform("color", colorUnit.getUnitNumber());
            copyShader->setUniform("depth", depthUnit.getUnitNumber());
            utilgl::singleDrawImagePlaneRect();
            copyShader->deactivate();
        }
    }
}

}  // namespace inviwo
