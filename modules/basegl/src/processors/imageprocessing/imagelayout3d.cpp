/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <modules/basegl/processors/imageprocessing/imagelayout3d.h>
#include <inviwo/core/util/assertion.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/spatialdata.h>
#include <inviwo/core/datastructures/camera/perspectivecamera.h>
#include <inviwo/core/interaction/events/resizeevent.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/pickingevent.h>

#include <modules/opengl/texture/textureunit.h>   // for TextureUnitContainer
#include <modules/opengl/texture/textureutils.h>  // for bindAndSetUniforms
#include <modules/opengl/openglutils.h>

#include <glm/gtx/transform.hpp>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ImageLayout3D::processorInfo_{
    "org.inviwo.ImageLayout3D",  // Class identifier
    "Image Layout 3D",           // Display name
    "Image Operation",           // Category
    CodeState::Experimental,     // Code state
    Tags::GL,                    // Tags
    R"(<Explanation of how to use the processor.>)"_unindentHelp};

const ProcessorInfo ImageLayout3D::getProcessorInfo() const { return processorInfo_; }

namespace {

std::vector<std::unique_ptr<Property>> prefab() {
    std::vector<std::unique_ptr<Property>> res;

    auto comp = std::make_unique<CompositeProperty>("inport", "Image Inport");
    comp->addProperty(std::make_unique<FloatProperty>("r", "r", 10, -100, 100));
    comp->addProperty(std::make_unique<FloatProperty>("theta", "theta", 0, -glm::pi<float>() / 2.0f,
                                                      glm::pi<float>() / 2.0f));
    comp->addProperty(
        std::make_unique<FloatProperty>("phi", "phi", 0, -glm::pi<float>(), glm::pi<float>()));

    comp->addProperty(
        std::make_unique<FloatVec2Property>("size", "Size", vec2{2}, vec2{-100}, vec2{100}));

    comp->addProperty(std::make_unique<IntSize2Property>("imageSize", "Image Size", vec2{512},
                                                         vec2{1}, vec2{4000}));

    res.push_back(std::move(comp));

    return res;
}

class Plane : public SpatialEntity<3> {
public:
    Plane() = default;
    Plane(float r, float theta, float phi, const vec2& size)
        : SpatialEntity<3>{toMat(r, theta, phi, size)} {}
    virtual Plane* clone() const override { return new Plane{*this}; }

private:
    static mat4 toMat(float r, float theta, float phi, const vec2& size) {
        const auto scale = glm::scale(vec3{size.x, size.y, 1.0});
        const auto translate = glm::translate(vec3{0, 0, -r});

        const auto rotate1 = glm::rotate(phi, vec3{0, 1, 0});
        const auto thetaAxis = vec3(rotate1 * vec4{1, 0, 0, 1});
        const auto rotate2 = glm::rotate(theta, thetaAxis);

        return rotate1 * rotate2 * translate * scale;
    }
};

constexpr std::string_view vertexShader = util::trim(R"(
uniform mat4 dataToClip;
out vec2 texCoord;
  
void main() {
    texCoord = in_TexCoord.xy;
    gl_Position = dataToClip * in_Vertex;
}
)");

constexpr std::string_view fragmentShader = util::trim(R"(
uniform sampler2D imageColor;
uniform sampler2D imagePicking;
in vec2 texCoord;
 
void main() {
    FragData0 = texture(imageColor, texCoord);
    PickingData = texture(imagePicking, texCoord);
}
)");

constexpr std::string_view vertexShaderForeground = util::trim(R"(
out vec2 texCoord;

void main() {
    texCoord = in_TexCoord.xy;
    gl_Position = in_Vertex;
}
)");

constexpr std::string_view fragmentShaderForeground = util::trim(R"(
uniform sampler2D imageColor;
uniform sampler2D imagePicking;
uniform sampler2D imageDepth;
in vec2 texCoord;
 
void main() {
    vec4 color = texture(imageColor, texCoord);
    if (color.a == 0) {
        discard;
    }
    FragData0 = color;
    PickingData = texture(imagePicking, texCoord);
    gl_FragDepth = texture(imageDepth, texCoord).r;
}
)");

}  // namespace

namespace detail {

DynImagePort::DynImagePort(ImageLayout3D* layoutProcessor, std::string_view identifier,
                           FloatProperty* r, FloatProperty* theta, FloatProperty* phi,
                           FloatVec2Property* aSize, IntSize2Property* anImageSize)
    : layoutProcessor{layoutProcessor}
    , port{std::make_unique<ImageInport>(identifier, identifier)}
    , r{r}
    , theta{theta}
    , phi{phi}
    , size{aSize}
    , imageSize{anImageSize}
    , oldImageSize{imageSize->get()}
    , sizeCallback{imageSize->onChangeScoped([this]() {
        ResizeEvent e{imageSize->get(), oldImageSize};
        port->propagateEvent(&e);
        oldImageSize = imageSize->get();
    })}
    , picking{} {

    port->setOptional(true);
    layoutProcessor->addPort(*port, "InsetImages");
}

DynImagePort::DynImagePort(DynImagePort&& rhs)
    : layoutProcessor{nullptr}
    , port{nullptr}
    , r{std::move(rhs.r)}
    , theta{std::move(rhs.theta)}
    , phi{std::move(rhs.phi)}
    , size{std::move(rhs.size)}
    , imageSize{std::move(rhs.imageSize)}
    , oldImageSize{rhs.oldImageSize}
    , sizeCallback{imageSize->onChangeScoped([this]() {
        ResizeEvent e{imageSize->get(), oldImageSize};
        port->propagateEvent(&e);
        oldImageSize = imageSize->get();
    })}
    , picking{rhs.picking} {

    std::swap(rhs.layoutProcessor, layoutProcessor);
    std::swap(rhs.port, port);
}

DynImagePort& DynImagePort::operator=(DynImagePort&& that) {
    if (this != &that) {
        std::swap(that.layoutProcessor, layoutProcessor);
        std::swap(that.port, port);
        std::swap(that.size, size);
        std::swap(that.r, r);
        std::swap(that.theta, theta);
        std::swap(that.phi, phi);
        std::swap(that.imageSize, imageSize);
        std::swap(that.oldImageSize, oldImageSize);

        sizeCallback = imageSize->onChangeScoped([this]() {
            ResizeEvent e{imageSize->get(), oldImageSize};
            port->propagateEvent(&e);
            oldImageSize = imageSize->get();
        });
        that.sizeCallback.reset();

        std::swap(that.picking, picking);
    }
    return *this;
}

DynImagePort::~DynImagePort() {
    if (port) layoutProcessor->removePort(port.get());
}

}  // namespace detail

ImageLayout3D::ImageLayout3D()
    : Processor{}
    , outport_{"outport", "Resulting layout of input images"_help}
    , background_{"background"}
    , foreground_{"foreground"}
    , ports_{"port", "Ports", R"(And and remove image ports for layouting)"_help, {prefab()}}
    , camera_{"camera", "Camera"}
    , shader_{{{ShaderType::Vertex,
                std::make_shared<StringShaderResource>("ImageLayout3D.vert", vertexShader)},
               {ShaderType::Fragment,
                std::make_shared<StringShaderResource>("ImageLayout3D.frag", fragmentShader)}},
              Shader::Build::Yes}
    , shaderForeground_{
          {{ShaderType::Vertex, std::make_shared<StringShaderResource>(
                                    "ImageLayout3DForeground.vert", vertexShaderForeground)},
           {ShaderType::Fragment, std::make_shared<StringShaderResource>(
                                      "ImageLayout3DForeground.frag", fragmentShaderForeground)}},
          Shader::Build::Yes} {

    addPorts(outport_, background_, foreground_);
    background_.setOptional(true);
    foreground_.setOptional(true);
    addProperties(ports_, camera_);

    ports_.PropertyOwnerObservable::addObserver(this);
}

ImageLayout3D::~ImageLayout3D() { ports_.PropertyOwnerObservable::removeObserver(this); }

void ImageLayout3D::onDidAddProperty(Property* property, size_t index) {
    auto comp = dynamic_cast<CompositeProperty*>(property);
    IVW_ASSERT(comp, "comp should always exist");
    auto r = dynamic_cast<FloatProperty*>(comp->getPropertyByIdentifier("r"));
    IVW_ASSERT(r, "r should always exist");
    auto theta = dynamic_cast<FloatProperty*>(comp->getPropertyByIdentifier("theta"));
    IVW_ASSERT(theta, "theta should always exist");
    auto phi = dynamic_cast<FloatProperty*>(comp->getPropertyByIdentifier("phi"));
    IVW_ASSERT(phi, "phi should always exist");
    auto size = dynamic_cast<FloatVec2Property*>(comp->getPropertyByIdentifier("size"));
    IVW_ASSERT(size, "size should always exist");
    auto imageSize = dynamic_cast<IntSize2Property*>(comp->getPropertyByIdentifier("imageSize"));
    IVW_ASSERT(imageSize, "imageSize should always exist");

    vecPorts_.emplace_back(this, property->getIdentifier(), r, theta, phi, size, imageSize);
}

void ImageLayout3D::onWillRemoveProperty(Property* property, size_t index) {
    vecPorts_.erase(vecPorts_.begin() + index);
    invalidate(InvalidationLevel::InvalidResources);
}

void ImageLayout3D::propagateEvent(Event* event, Outport* source) {
    if (!event->markAsVisited(this)) return;

    invokeEvent(event);
    bool used = event->hasBeenUsed();
    if (used) return;

    if (auto* mouseEvent = event->getAs<MouseEvent>()) {
        for (auto& port : vecPorts_) {

            Plane p{port.r->get(), port.theta->get(), port.phi->get(), port.size->get()};
            auto& ct = p.getCoordinateTransformer(camera_.get());
            auto m = ct.getWorldToDataMatrix();

            auto ndc = mouseEvent->ndc();
            auto worldPos = camera_.getWorldPosFromNormalizedDeviceCoords(ndc);

            auto trans = vec3{m * vec4{worldPos, 1.0f}};

            if (std::abs(trans.x) < 1.0f && std::abs(trans.y) < 1.0 && std::abs(trans.z) < 0.1) {
                //LogWarn(fmt::format("Match {} {}", port.port->getIdentifier(), trans));

                MouseEvent newEvent(*mouseEvent);
                newEvent.setCanvasSize(port.imageSize->get());
                newEvent.setPosNormalized((dvec2{trans} + dvec2{1.0}) * 0.5);
                newEvent.setDepth(trans.z);
                port.port->propagateEvent(&newEvent);
                used |= newEvent.hasBeenUsed();
            }
        }
    } else if (auto* pickingEvent = event->getAs<PickingEvent>()) {
        if (auto wrappedMouseEvent = pickingEvent->getEventAs<MouseEvent>()) {

            for (auto& port : vecPorts_) {

                Plane p{port.r->get(), port.theta->get(), port.phi->get(), port.size->get()};
                auto& ct = p.getCoordinateTransformer(camera_.get());
                auto m = ct.getWorldToDataMatrix();

                auto mapNDC = [&](vec3 ndc) {
                    auto worldPos = camera_.getWorldPosFromNormalizedDeviceCoords(ndc);
                    return vec3{m * vec4{worldPos, 1.0f}};
                };
                auto trans = mapNDC(wrappedMouseEvent->ndc());

                if (std::abs(trans.x) < 1.0f && std::abs(trans.y) < 1.0 &&
                    std::abs(trans.z) < 0.1) {

                    MouseEvent newEvent(*wrappedMouseEvent);
                    newEvent.setCanvasSize(port.imageSize->get());
                    newEvent.setPosNormalized((dvec2{trans} + dvec2{1.0}) * 0.5);

                    size2_t pos{dvec2{port.imageSize->get() - size2_t{1}} *
                                (dvec2{trans} + dvec2{1.0}) * 0.5};
                    const auto depth = port.port->getData()->readPixel(pos, LayerType::Depth);
                    const auto ndcDepth = 2.0 * depth.x - 1.0;

                    newEvent.setDepth(ndcDepth);

                    auto currNDC = mapNDC(pickingEvent->getNDC());
                    currNDC.z = ndcDepth;

                    if (pickingEvent->getPressState() == PickingPressState::Press &&
                        port.picking.state == PickingPressState::None) {
                        port.picking.state = PickingPressState::Press;
                        port.picking.pres = currNDC;
                    } else if (pickingEvent->getPressState() == PickingPressState::None) {
                        port.picking.state = PickingPressState::None;
                    }

                    PickingEvent newPickingEvent{pickingEvent->getPickingAction(),
                                                 &newEvent,
                                                 pickingEvent->getState(),
                                                 pickingEvent->getPressState(),
                                                 pickingEvent->getPressItem(),
                                                 pickingEvent->getHoverState(),
                                                 pickingEvent->getPressItems(),
                                                 pickingEvent->getGlobalPickingId(),
                                                 pickingEvent->getCurrentGlobalPickingId(),
                                                 pickingEvent->getPressedGlobalPickingId(),
                                                 pickingEvent->getPreviousGlobalPickingId(),
                                                 port.picking.pres,
                                                 port.picking.prev};
                    /*
                    LogWarn(fmt::format("Pick {} curr {} prev {} press {} delta {}",
                                        port.port->getIdentifier(), newPickingEvent.getNDC(),
                                        newPickingEvent.getPreviousNDC(),
                                        newPickingEvent.getPressedNDC(),
                                        newPickingEvent.getDeltaPressedPosition()));
                    */
                    port.port->propagateEvent(&newPickingEvent);
                    used |= newPickingEvent.hasBeenUsed();

                    pickingEvent->markAsUsed();

                    port.picking.prev = currNDC;
                }
            }
        }
        return;
    }

    if (used) return;

    for (auto inport : getInports()) {
        if (event->shouldPropagateTo(inport, this, source)) {
            inport->propagateEvent(event);
            used |= event->markAsUnused();
        }
    }

    if (auto* resizeEvent = event->getAs<ResizeEvent>()) {
        for (auto& port : vecPorts_) {
            ResizeEvent e{port.imageSize->get(), port.oldImageSize};
            port.port->propagateEvent(&e);
            port.oldImageSize = port.imageSize->get();
        }
    }

    event->setUsed(used);
}

void ImageLayout3D::process() {

    utilgl::activateTargetAndClearOrCopySource(outport_, background_);
    shader_.activate();

    for (auto& port : vecPorts_) {
        if (!port.port->isReady()) continue;

        TextureUnitContainer units;
        utilgl::bindAndSetUniforms(shader_, units, *(port.port->getData()), "image",
                                   ImageType::ColorPicking);

        Plane p{port.r->get(), port.theta->get(), port.phi->get(), port.size->get()};
        auto& ct = p.getCoordinateTransformer(camera_.get());
        const auto dataToClip = ct.getDataToClipMatrix();
        shader_.setUniform("dataToClip", dataToClip);
        utilgl::singleDrawImagePlaneRect();
    }

    shader_.deactivate();

    if (foreground_.isReady()) {
        shaderForeground_.activate();
        utilgl::GlBoolState depthTest(GL_DEPTH_TEST, GL_TRUE);
        utilgl::BlendModeState blend{GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA};

        TextureUnitContainer units;
        utilgl::bindAndSetUniforms(shaderForeground_, units, *foreground_.getData(), "image",
                                   ImageType::ColorDepthPicking);

        utilgl::singleDrawImagePlaneRect();

        shaderForeground_.deactivate();
    }

    utilgl::deactivateCurrentTarget();
}

}  // namespace inviwo
