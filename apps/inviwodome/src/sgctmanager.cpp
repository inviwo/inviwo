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

SgctManager::SgctManager(InviwoApplication& app) : app{app} {}

SgctManager::~SgctManager() {
    clear();
    teardownInteraction();
}

void SgctManager::clear() {
    TRACY_ZONE_SCOPED_NC("Clear", 0xAA0000);

    eventPropagator = [](Event*) {};
    depth = [](dvec2) { return -1.0; };
    canvases.clear();
    cameras.clear();
    app.getWorkspaceManager()->clear();
    modifiedProperties.clear();
}
void SgctManager::loadWorkspace(const std::string& filename) {
    workspace = filename;
    TRACY_ZONE_SCOPED_NC("Load Workspace", 0xAA0000);

    RenderContext::getPtr()->activateDefaultRenderContext();

    clear();
    app.getWorkspaceManager()->load(filename, [&](ExceptionContext ec) {
        try {
            throw;
        } catch (const IgnoreException& e) {
            util::log(e.getContext(),
                      "Incomplete network loading " + filename + " due to " + e.getMessage(),
                      LogLevel::Error);
        }
    });

    auto net = app.getProcessorNetwork();

    LambdaNetworkVisitor visitor{[&](Processor& processor) {
                                     if (auto cp = dynamic_cast<CanvasProcessor*>(&processor)) {
                                         canvases.push_back(cp);
                                     }
                                     return true;
                                 },
                                 [&](Property& property) {
                                     if (sgct::Engine::instance().isMaster()) {
                                         property.onChange([this, p = &property]() {
                                             TRACY_ZONE_SCOPED_NC("Property Modified", 0xAA0000);
                                             util::push_back_unique(modifiedProperties, p);
                                         });
                                     }
                                     return true;
                                 }};
    net->accept(visitor);

    if (auto canvas = getCanvas()) {
        std::unordered_set<Processor*> visited;
        util::traverseNetwork<util::TraversalDirection::Up, util::VisitPattern::Pre>(
            visited, canvas, [&](Processor* p) {
                for (auto camera : p->getPropertiesByType<CameraProperty>()) {
                    cameras.push_back(camera);
                }
            });
    }
    for (auto camera : cameras) {
        camera->setCamera("SGCTCamera");
    }
    if (auto camera = getCamera()) {
        sgct::Engine::instance().setNearAndFarClippingPlanes(camera->getNearPlaneDist(),
                                                             camera->getFarPlaneDist());
    }

    if (auto canvas = getCanvas()) {
        eventPropagator = [canvas](Event* e) { canvas->propagateEvent(e, nullptr); };
        canvas->setEvaluateWhenHidden(true);
        if (auto widget = canvas->getProcessorWidget()) {
            widget->setVisible(false);
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
                    showStats = true;
                    return;
                }
                if (ke->key() == IvwKey::H && ke->state() == KeyState::Release &&
                    ke->modifiers() == KeyModifier::Alt) {
                    showStats = false;
                    return;
                }
                if (ke->key() == IvwKey::R && ke->state() == KeyState::Release &&
                    ke->modifiers() == KeyModifier::Alt) {
                    app.getWorkspaceManager()->load(workspace, [&](ExceptionContext ec) {
                        try {
                            throw;
                        } catch (const IgnoreException& e) {
                            util::log(e.getContext(),
                                      "Incomplete network loading " + workspace + " due to " +
                                          e.getMessage(),
                                      LogLevel::Error);
                        }
                    });
                    LogInfo("Reloaded " << workspace);
                    return;
                }
            }

            eventPropagator(e);
        },
        [this](dvec2 p) { return depth(p); }));
}
void SgctManager::teardownInteraction() {
    interactionManagers.clear();
    userdata.clear();
}
void SgctManager::evaluate(const ::sgct::RenderData& renderData) {
    TRACY_ZONE_SCOPED_NC("Evaluate", 0xAA0000);
    TRACY_GPU_ZONE_C("Evaluate", 0xAA0000);

    auto net = app.getProcessorNetwork();

    CanvasGL::defaultGLState();

    {
        TRACY_ZONE_SCOPED_NC("Update cameras", 0x770000);
        const auto size = renderData.window.resolution();
        const size2_t newSize{size.x, size.y};
        if (auto canvas = getCanvas(); canvas && canvas->getCanvasSize() != newSize) {
            canvas->setCanvasSize(newSize);
        }

        for (auto camera : cameras) {
            auto& cam = dynamic_cast<SGCTCamera&>(camera->get());
            cam.setExternal(renderData);
        }
    }

    {
        TRACY_ZONE_SCOPED_NC("Evaluate Network", 0x770000);
        TRACY_GPU_ZONE_C("Evaluate Network", 0x770000);
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
void SgctManager::copy() {
    TRACY_ZONE_SCOPED_NC("Copy", 0xAA0000);
    TRACY_GPU_ZONE_C("Copy", 0xAA0000);

    if (!copyShader) createShader();
    // Copy inviwo output
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
void SgctManager::getUpdates(std::ostream& os) {
    TRACY_ZONE_SCOPED_NC("Serialize Changes", 0xAA0000);

    std::vector<Property*> uniqueAfterLinks;
    std::vector<std::string> paths;

    {
        TRACY_ZONE_SCOPED_NC("Find modifications", 0xAA0000);

        std::vector<Property*> unique;
        {
            const auto getOwners = [](Property* p) {
                std::vector<PropertyOwner*> owners;
                PropertyOwner* owner = p->getOwner();
                while (owner) {
                    owners.push_back(owner);
                    owner = owner->getOwner();
                }
                std::reverse(owners.begin(), owners.end());
                return owners;
            };

            using PathAndProperty = std::pair<std::vector<PropertyOwner*>, Property*>;
            std::vector<PathAndProperty> pathAndProperties;
            std::transform(modifiedProperties.begin(), modifiedProperties.end(),
                           std::back_inserter(pathAndProperties), [&](auto* p) {
                               return PathAndProperty{getOwners(p), p};
                           });

            std::sort(pathAndProperties.begin(), pathAndProperties.end(),
                      [](const PathAndProperty& a, const PathAndProperty& b) {
                          if (a.first == b.first) {
                              return a.second < b.second;
                          } else {
                              return a.first < b.first;
                          }
                      });

            auto newEnd =
                std::unique(pathAndProperties.begin(), pathAndProperties.end(),
                            [](const PathAndProperty& a, const PathAndProperty& b) {
                                const auto& pathA = a.first;
                                const auto& pathB = b.first;
                                const auto& propA = a.second;
                                const auto& propB = b.second;
                                if (pathA.size() > pathB.size()) return false;
                                if (pathA.size() == pathB.size())
                                    return pathA == pathB && propA == propB;
                                return std::equal(pathA.begin(), pathA.end(), pathB.begin(),
                                                  pathB.begin() + pathA.size());
                            });

            std::transform(pathAndProperties.begin(), newEnd, std::back_inserter(unique),
                           [](const PathAndProperty& a) { return a.second; });
        }

        // Only send one of the properties in a set of linked properties
        auto net = app.getProcessorNetwork();
        auto linked = [&](Property* a, Property* b) {
            auto alinks = net->getPropertiesLinkedTo(a);
            if (!util::contains(alinks, b)) return false;
            auto blinks = net->getPropertiesLinkedTo(b);
            if (!util::contains(blinks, a)) return false;

            return true;
        };

        for (auto* p : unique) {
            if (!util::contains_if(uniqueAfterLinks, [&](Property* b) { return linked(p, b); })) {
                uniqueAfterLinks.push_back(p);
            }
        }

        std::transform(uniqueAfterLinks.begin(), uniqueAfterLinks.end(), std::back_inserter(paths),
                       [](auto* p) { return p->getPath(); });
    }

    Serializer s{""};
    {
        TRACY_ZONE_SCOPED_NC("Create DOM", 0xAA0000);
        s.serialize("paths", paths);
        s.serialize("modified", uniqueAfterLinks);
    }
    {
        TRACY_ZONE_SCOPED_NC("Write DOM", 0xAA0000);
        s.writeFile(os, false);
    }

    TRACY_ZONE_VALUE(static_cast<int64_t>(uniqueAfterLinks.size()));
    TRACY_PLOT("Serialized Property Count", static_cast<int64_t>(uniqueAfterLinks.size()));
}
void SgctManager::applyUpdate(std::istream& is) {
    TRACY_ZONE_SCOPED_NC("Deserialize Changes", 0xAA0000);

    Deserializer d(is, "");

    {
        TRACY_ZONE_SCOPED_NC("Apply DOM", 0xAA0000);
        std::vector<std::string> paths;
        std::vector<Property*> modifiedProps;
        auto net = app.getProcessorNetwork();

        d.deserialize("paths", paths);

        for (auto& path : paths) {
            modifiedProps.push_back(net->getProperty(path));
        }

        {
            TRACY_ZONE_SCOPED_NC("Deserialize modifiedProps", 0xAA0000);
            d.deserialize("modified", modifiedProps);
        }
    }
}
}  // namespace inviwo