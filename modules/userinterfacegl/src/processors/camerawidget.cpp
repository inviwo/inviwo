/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2022 Inviwo Foundation
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

#include <modules/userinterfacegl/processors/camerawidget.h>

#include <inviwo/core/common/inviwoapplication.h>                 // for InviwoApplication
#include <inviwo/core/common/inviwomodule.h>                      // for InviwoModule, ModulePath
#include <inviwo/core/datastructures/camera/camera.h>             // for Camera
#include <inviwo/core/datastructures/camera/perspectivecamera.h>  // for PerspectiveCamera
#include <inviwo/core/datastructures/geometry/mesh.h>             // for Mesh
#include <inviwo/core/datastructures/geometry/typedmesh.h>        // for BasicMesh
#include <inviwo/core/datastructures/image/image.h>               // for Image
#include <inviwo/core/datastructures/image/imagetypes.h>          // for ImageType, ImageType::C...
#include <inviwo/core/interaction/events/pickingevent.h>          // for PickingEvent
#include <inviwo/core/interaction/pickingmapper.h>                // for PickingMapper
#include <inviwo/core/interaction/pickingstate.h>                 // for PickingPressItem, Picki...
#include <inviwo/core/io/datareader.h>                            // for DataReaderType
#include <inviwo/core/io/datareaderfactory.h>                     // for DataReaderFactory
#include <inviwo/core/network/processornetwork.h>                 // for ProcessorNetwork
#include <inviwo/core/ports/imageport.h>                          // for ImageInport, ImageOutport
#include <inviwo/core/processors/processor.h>                     // for Processor
#include <inviwo/core/processors/processorinfo.h>                 // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                // for CodeState, CodeState::E...
#include <inviwo/core/processors/processortags.h>                 // for Tags
#include <inviwo/core/properties/boolcompositeproperty.h>         // for BoolCompositeProperty
#include <inviwo/core/properties/boolproperty.h>                  // for BoolProperty
#include <inviwo/core/properties/buttonproperty.h>                // for ButtonProperty
#include <inviwo/core/properties/cameraproperty.h>                // for CameraProperty
#include <inviwo/core/properties/compositeproperty.h>             // for CompositeProperty
#include <inviwo/core/properties/constraintbehavior.h>            // for ConstraintBehavior, Con...
#include <inviwo/core/properties/invalidationlevel.h>             // for InvalidationLevel, Inva...
#include <inviwo/core/properties/ordinalproperty.h>               // for ordinalColor, FloatProp...
#include <inviwo/core/properties/simplelightingproperty.h>        // for SimpleLightingProperty
#include <inviwo/core/util/exception.h>                           // for Exception
#include <inviwo/core/util/glm.h>                                 // for filled
#include <inviwo/core/util/glmmat.h>                              // for mat4, mat3
#include <inviwo/core/util/glmvec.h>                              // for vec3, vec2, dvec2, vec4
#include <inviwo/core/util/logcentral.h>                          // for LogVerbosity, LogVerbos...
#include <inviwo/core/util/sourcecontext.h>                       // for IVW_CONTEXT
#include <modules/base/algorithm/meshutils.h>                     // for cube
#include <modules/opengl/image/imagegl.h>                         // for ImageGL
#include <modules/opengl/image/layergl.h>                         // for LayerGL
#include <modules/opengl/inviwoopengl.h>                          // for GL_ALWAYS, GL_DEPTH_TEST
#include <modules/opengl/openglutils.h>                           // for BlendModeState, DepthFu...
#include <modules/opengl/rendering/meshdrawergl.h>                // for MeshDrawerGL
#include <modules/opengl/shader/shader.h>                         // for Shader, Shader::Build
#include <modules/opengl/shader/shaderobject.h>                   // for ShaderObject
#include <modules/opengl/shader/shaderutils.h>                    // for setShaderUniforms, addS...
#include <modules/opengl/texture/textureunit.h>                   // for TextureUnit
#include <modules/opengl/texture/textureutils.h>                  // for activateAndClearTarget

#include <algorithm>                                              // for max
#include <cmath>                                                  // for tan
#include <cstdlib>                                                // for abs
#include <functional>                                             // for __base, function
#include <map>                                                    // for map
#include <string>                                                 // for string
#include <string_view>                                            // for string_view
#include <type_traits>                                            // for enable_if<>::type
#include <typeinfo>                                               // for bad_cast
#include <vector>                                                 // for vector, vector<>::value...

#include <flags/flags.h>                                          // for operator&, flags
#include <fmt/core.h>                                             // for format
#include <glm/common.hpp>                                         // for abs
#include <glm/ext/matrix_transform.hpp>                           // for rotate, scale
#include <glm/ext/scalar_constants.hpp>                           // for epsilon, pi
#include <glm/geometric.hpp>                                      // for normalize, cross, length
#include <glm/gtc/matrix_inverse.hpp>                             // for inverseTranspose
#include <glm/gtx/transform.hpp>                                  // for rotate, scale
#include <glm/mat3x3.hpp>                                         // for operator*, mat
#include <glm/mat4x4.hpp>                                         // for mat, operator+, operator*
#include <glm/trigonometric.hpp>                                  // for radians
#include <glm/vec2.hpp>                                           // for operator*, vec<>::(anon...
#include <glm/vec3.hpp>                                           // for operator*, vec<>::(anon...
#include <glm/vec4.hpp>                                           // for operator*, operator+

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo CameraWidget::processorInfo_{
    "org.inviwo.CameraWidget",  // Class identifier
    "Camera Widget",            // Display name
    "UI",                       // Category
    CodeState::Experimental,    // Code state
    "GL, UI"                    // Tags
};
const ProcessorInfo CameraWidget::getProcessorInfo() const { return processorInfo_; }

CameraWidget::CameraWidget()
    : Processor()
    , inport_("inport")
    , outport_("outport")

    , settings_("settings", "Settings")
    , enabled_("enabled", "Enabled", true)
    , invertDirections_("invertDirections", "Invert Directions", false, InvalidationLevel::Valid)
    , useObjectRotAxis_("useObjectRotationAxis", "Use Vertical Object Axis for Rotation", false)
    , showRollWidget_("showRollWidget", "Camera Roll", true)
    , showDollyWidget_("showDolly", "Camera Dolly", false)
    , speed_("speed", "Speed (deg per pixel)", 0.25f, 0.01f, 5.0f, 0.05f, InvalidationLevel::Valid)
    , angleIncrement_("angleIncrement", "Angle (deg) per Click", 15.0f, 0.05f, 90.0f, 0.5f,
                      InvalidationLevel::Valid)
    , minTouchMovement_("minTouchMovement", "Min Touch Movement (pixel)", 5, 0, 25, 1,
                        InvalidationLevel::Valid)

    , appearance_("appearance", "Appearance")
    , scaling_("scaling", "Scaling", 0.4f, 0.01f, 5.0f, 0.01f)
    , position_("position", "Position", vec2(0.01f, 0.01f), vec2(0.0f), vec2(1.0f), vec2(0.01f))
    , anchorPos_("Anchor", "Anchor", vec2(-1.0f, 1.0f), vec2(-1.0f), vec2(1.0f), vec2(0.01f))
    , showCube_("showCube", "Show Orientation Cube", true)
    , customColorComposite_("enableCustomColor", "Custom Widget Color", false,
                            InvalidationLevel::InvalidResources)
    , axisColoring_("axisColoring", "RGB Axis Coloring", false, InvalidationLevel::InvalidResources)
    , userColor_{"userColor", "User Color", util::ordinalColor(0.7f, 0.7f, 0.7f)}
    , cubeColor_{"cubeColor", "Cube Color", util::ordinalColor(0.11f, 0.42f, 0.63f)}

    , interactions_("interactions", "Interactions")

    , rotateUpBtn_("rotateUpButton", "Rotate Up")
    , rotateDownBtn_("rotateDownButton", "Rotate Down")
    , rotateLeftBtn_("rotateLeftButton", "Rotate Left")
    , rotateRightBtn_("rotateRightButton", "Rotate Right")

    , outputProps_("outputProperties", "Output")
    , camera_("camera", "Camera")
    , rotMatrix_{"rotMatrix",
                 "Rotation Matrix",
                 mat4(1.0f),
                 {util::filled<mat4>(-10.0f), ConstraintBehavior::Ignore},
                 {util::filled<mat4>(+10.0f), ConstraintBehavior::Ignore}}

    , internalProps_("internalProperties", "Internal Properties")
    , internalCamera_(vec3(0.0f, 0.0f, 14.6f), vec3(0.0f), vec3(0.0f, 1.0f, 0.0f), 1.0f, 30.0f,
                      38.0f, 1.0f)
    , lightingProperty_("internalLighting", "Lighting", nullptr)

    , picking_(this, numInteractionWidgets, [&](PickingEvent* p) { objectPicked(p); })
    , shader_("widgetrenderer.vert", "geometryrendering.frag", Shader::Build::No)
    , cubeShader_("geometryrendering.vert", "geometryrendering.frag", Shader::Build::No)
    , overlayShader_("img_identity.vert", "widgettexture.frag")
    , isMouseBeingPressedAndHold_(false)
    , mouseWasMoved_(false)
    , currentPickingID_(-1) {

    addPort(inport_).setOptional(true);
    addPort(outport_);

    settings_.setCollapsed(true);
    internalProps_.setCollapsed(true);

    // interaction settings
    enabled_.onChange([this]() { picking_.setEnabled(enabled_); });
    settings_.addProperties(enabled_, invertDirections_, useObjectRotAxis_, showRollWidget_,
                            showDollyWidget_, speed_, angleIncrement_, minTouchMovement_);

    // widget appearance
    customColorComposite_.addProperties(axisColoring_, userColor_);
    appearance_.addProperties(position_, anchorPos_, scaling_, showCube_, cubeColor_,
                              customColorComposite_);

    // button interactions
    interactions_.setCollapsed(true);

    rotateUpBtn_.onChange([&]() { singleStepInteraction(Interaction::VerticalRotation, true); });
    rotateDownBtn_.onChange([&]() { singleStepInteraction(Interaction::VerticalRotation, false); });
    rotateLeftBtn_.onChange(
        [&]() { singleStepInteraction(Interaction::HorizontalRotation, true); });
    rotateRightBtn_.onChange(
        [&]() { singleStepInteraction(Interaction::HorizontalRotation, false); });

    interactions_.addProperties(rotateUpBtn_, rotateDownBtn_, rotateLeftBtn_, rotateRightBtn_);

    // output properties
    camera_.setCollapsed(true);
    outputProps_.addProperties(camera_, rotMatrix_);

    lightingProperty_.setCollapsed(true);
    internalProps_.addProperty(lightingProperty_);

    addProperty(settings_);
    addProperty(appearance_);
    addProperty(interactions_);
    addProperty(outputProps_);
    addProperty(internalProps_);

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
    cubeShader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
    overlayShader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });

    lightingProperty_.ambientColor_ = vec3(0.75f);
    lightingProperty_.diffuseColor_ = vec3(0.6f);
    lightingProperty_.specularColor_ = vec3(0.12f);
    lightingProperty_.lightPosition_.set(vec3(2.5f, 5.6f, 20.0f), vec3(-100.0f), vec3(100.0f),
                                         vec3(1.0f));
    lightingProperty_.setCurrentStateAsDefault();

    // update internal picking IDs. The mesh consists of 5 elements, each of them has a clockwise
    // and a counter clockwise / zoom in and zoom out part.
    std::array<Interaction, 5> dir = {Interaction::HorizontalRotation,
                                      Interaction::VerticalRotation, Interaction::FreeRotation,
                                      Interaction::Roll, Interaction::Zoom};

    for (int i = 0; i < numInteractionWidgets; ++i) {
        pickingIDs_[i] = {picking_.getPickingId(i), dir[i / 2], ((i % 2) == 0)};
    }
}

CameraWidget::~CameraWidget() = default;

void CameraWidget::process() {
    if (meshes_[0].get() == nullptr) {
        loadMesh();
    }

    // determine size of the widget
    const vec2 referenceSize(
        300.0f);  // reference size (width/height) in pixel for scale equal to 1.0
    ivec2 widgetSize(scaling_.get() * referenceSize);

    updateWidgetTexture(widgetSize);

    // combine the previously rendered widget image with the input
    if (inport_.isReady()) {
        utilgl::activateTargetAndCopySource(outport_, inport_);
    } else {
        utilgl::activateAndClearTarget(outport_, ImageType::ColorDepthPicking);
    }

    utilgl::GlBoolState depthTest(GL_DEPTH_TEST, true);
    utilgl::BlendModeState blending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    drawWidgetTexture();
    utilgl::deactivateCurrentTarget();
}

void CameraWidget::initializeResources() {
    // shading defines
    utilgl::addShaderDefines(shader_, lightingProperty_);
    utilgl::addShaderDefines(cubeShader_, lightingProperty_);

    shader_.getVertexShaderObject()->setShaderDefine("CUSTOM_COLOR", customColorComposite_,
                                                     axisColoring_.get() ? "1" : "0");

    shader_.build();

    cubeShader_.getVertexShaderObject()->addShaderDefine("OVERRIDE_COLOR_BUFFER");
    cubeShader_.build();

    // initialize shader uniform containing all picking colors, the mesh will fetch the appropriate
    // one using the object ID stored in the second texture coordinate set
    shader_.activate();
    std::vector<vec3> colors;
    colors.resize(numInteractionWidgets);
    // fill vector with picking colors
    for (int i = 0; i < numInteractionWidgets; ++i) {
        colors[i] = picking_.getColor(i);
    }
    shader_.setUniform("pickColors", colors);

    // set up default colors for the mesh (horizontal: red, vertical: green, center: light gray,
    // roll: light blue, zoom: light gray)
    std::array<vec4, 5> color = {vec4(0.8f, 0.5f, 0.5f, 1.0f), vec4(0.5f, 0.8f, 0.5f, 1.0f),
                                 vec4(vec3(0.5f), 1.0f), vec4(0.4f, 0.5f, 0.8f, 1.0f),
                                 vec4(vec3(0.5f), 1.0f)};
    shader_.setUniform("meshColors_", color);

    shader_.deactivate();
}

void CameraWidget::updateWidgetTexture(const ivec2& widgetSize) {
    if (!widgetImage_.get() || (ivec2(widgetImage_->getDimensions()) != widgetSize)) {
        widgetImage_ = std::make_unique<Image>(widgetSize, outport_.getDataFormat());
        widgetImageGL_ = widgetImage_->getEditableRepresentation<ImageGL>();
    }
    // enable fbo for rendering only the widget
    widgetImageGL_->activateBuffer(ImageType::ColorDepthPicking);
    utilgl::clearCurrentTarget();

    shader_.activate();

    utilgl::setShaderUniforms(shader_, internalCamera_, "camera");
    utilgl::setShaderUniforms(shader_, lightingProperty_, "lighting");
    shader_.setUniform("overrideColor", userColor_.get());

    auto* meshDrawer = meshDrawers_[showRollWidget_.get() ? 1 : 0].get();
    utilgl::setShaderUniforms(shader_, *meshDrawer->getMesh(), "geometry");

    meshDrawer->draw();

    // draw zoom buttons
    if (showDollyWidget_) {
        utilgl::setShaderUniforms(shader_, *meshDrawers_[2]->getMesh(), "geometry");
        meshDrawers_[2]->draw();
    }

    if (showCube_) {
        // draw cube behind the widget using the view matrix of the output camera
        cubeShader_.activate();
        utilgl::setShaderUniforms(cubeShader_, internalCamera_, "camera");
        utilgl::setShaderUniforms(cubeShader_, lightingProperty_, "lighting");

        // apply custom transformation
        mat4 worldMatrix(camera_.viewMatrix());
        // consider only the camera rotation and overwrite the translation by moving the cube back a
        // bit
        worldMatrix[3] = vec4(0.0f, 0.0f, -8.0f, 1.0f);
        mat3 normalMatrix(glm::inverseTranspose(worldMatrix));

        cubeShader_.setUniform("geometry.dataToWorld", worldMatrix);
        cubeShader_.setUniform("geometry.dataToWorldNormalMatrix", normalMatrix);
        cubeShader_.setUniform("overrideColor", cubeColor_.get());

        meshDrawers_[3]->draw();
    }

    widgetImageGL_->deactivateBuffer();
}

void CameraWidget::drawWidgetTexture() {
    ivec2 widgetSize(widgetImageGL_->getDimensions());

    vec2 dstSize(outport_.getDimensions());
    vec2 centerPos(position_.get() * dstSize);
    // draw widget image on top of the input image
    // consider anchor position
    vec2 shift = 0.5f * vec2(widgetSize) * (anchorPos_.get() + vec2(1.0f, 1.0f));
    centerPos.x -= shift.x;
    // negate y axis
    centerPos.y = dstSize.y - (centerPos.y + shift.y);

    ivec4 viewport(centerPos.x, centerPos.y, widgetSize.x, widgetSize.y);
    utilgl::ViewportState viewportState(viewport);
    utilgl::DepthFuncState depthFunc(GL_ALWAYS);

    TextureUnit colorTexUnit, depthTexUnit, pickingTexUnit;
    overlayShader_.activate();
    overlayShader_.setUniform("color_", colorTexUnit);
    overlayShader_.setUniform("depth_", depthTexUnit);
    overlayShader_.setUniform("picking_", pickingTexUnit);

    if (auto layer = widgetImageGL_->getColorLayerGL()) {
        layer->bindTexture(colorTexUnit);
    }
    if (auto layer = widgetImageGL_->getDepthLayerGL()) {
        layer->bindTexture(depthTexUnit);
    }
    if (auto layer = widgetImageGL_->getPickingLayerGL()) {
        layer->bindTexture(pickingTexUnit);
    }
    utilgl::singleDrawImagePlaneRect();

    overlayShader_.deactivate();
}

void CameraWidget::objectPicked(PickingEvent* e) {
    const auto pickedID = static_cast<int>(e->getPickedId());
    if (pickedID >= static_cast<int>(pickingIDs_.size())) {
        return;
    }

    bool triggerMoveEvent = false;
    bool triggerSingleEvent = false;

    if (e->getPressState() != PickingPressState::None) {
        if (e->getPressState() == PickingPressState::Press &&
            e->getPressItem() & PickingPressItem::Primary) {
            // initial activation with button press
            isMouseBeingPressedAndHold_ = true;
            mouseWasMoved_ = false;
            currentPickingID_ = static_cast<int>(e->getPickedId());
            saveInitialCameraState();
        } else if (e->getPressState() == PickingPressState::Move &&
                   e->getPressItems() & PickingPressItem::Primary) {
            // check whether mouse has been moved for more than 1 pixel
            if (!mouseWasMoved_) {
                const dvec2 delta(e->getDeltaPressedPosition() * dvec2(e->getCanvasSize()));
                const double squaredDist = delta.x * delta.x + delta.y * delta.y;
                mouseWasMoved_ = (squaredDist > 1.0);
            }
            triggerMoveEvent = true;
        } else if (e->getPressState() == PickingPressState::Release &&
                   e->getPressItem() & PickingPressItem::Primary) {
            triggerSingleEvent =
                (isMouseBeingPressedAndHold_ && (currentPickingID_ >= 0) && !mouseWasMoved_);
            isMouseBeingPressedAndHold_ = false;
            if (currentPickingID_ >= 0) {
                currentPickingID_ = -1;
                invalidate(InvalidationLevel::InvalidOutput);
            }
        }
        e->markAsUsed();
    }

    if (triggerMoveEvent) {
        interaction(pickingIDs_[currentPickingID_].dir,
                    e->getDeltaPosition() * dvec2(e->getCanvasSize()));

    } else if (triggerSingleEvent) {
        singleStepInteraction(pickingIDs_[pickedID].dir, pickingIDs_[pickedID].clockwise);
    }
}

void CameraWidget::saveInitialCameraState() {
    // save current camera vectors (direction, up, and right) to be able to do absolute
    // rotations
    auto& cam = camera_.get();
    vec3 camDir(glm::normalize(cam.getDirection()));
    vec3 camUp(glm::normalize(cam.getLookUp()));
    vec3 camRight(glm::cross(camDir, camUp));
    // make sure, up is orthogonal to both, dir and right
    camUp = glm::cross(camRight, camDir);
    // update camera
    camera_.setLookUp(camUp);

    initialState_.camDir = camDir;
    initialState_.camUp = camUp;
    initialState_.camRight = camRight;
    try {
        auto& perspCam = dynamic_cast<PerspectiveCamera&>(cam);
        double fovy = perspCam.getFovy();
        initialState_.zoom_ = 1.0 / std::tan(fovy * glm::pi<double>() / 360.0);
    } catch (std::bad_cast&) {
        // camera is not a PerspectiveCamera
        initialState_.zoom_ = 1.0;
    }
}

void CameraWidget::loadMesh() {
    auto load = [this](std::string_view file) -> std::shared_ptr<const Mesh> {
        auto app = getInviwoApplication();
        auto module = app->getModuleByIdentifier("UserInterfaceGL");
        if (!module) {
            throw Exception("Could not locate module 'UserInterfaceGL'", IVW_CONTEXT);
        }

        auto reader = app->getDataReaderFactory()->getReaderForTypeAndExtension<Mesh>("fbx");
        if (!reader) {
            throw Exception("Could not fbx mesh reader", IVW_CONTEXT);
        }
        reader->setOption("FixInvalidData", false);
        reader->setOption("LogLevel", LogVerbosity::Error);

        return reader->readData(
            fmt::format("{}/meshes/{}", module->getPath(ModulePath::Data), file));
    };

    auto cache = [](std::weak_ptr<const Mesh>& cache, auto func,
                    auto... args) -> std::shared_ptr<const Mesh> {
        if (auto mesh = cache.lock()) {
            return mesh;
        } else {
            mesh = func(args...);
            cache = mesh;
            return mesh;
        }
    };

    static std::array<std::weak_ptr<const Mesh>, 4> meshes;

    // camera interaction widgets
    meshes_[0] = cache(meshes[0], load, "camera-widget.fbx");
    // widget including "camera roll" arrow
    meshes_[1] = cache(meshes[1], load, "camera-widget-roll.fbx");
    // camera zoom buttons
    meshes_[2] = cache(meshes[2], load, "camera-zoom.fbx");
    // cube mesh for orientation
    meshes_[3] = cache(meshes[3], []() {
        const vec3 cubeScale(8.0f);
        mat4 transform(glm::scale(vec3(cubeScale)));
        transform[3] = vec4(-0.5f * cubeScale, 1.0f);
        return meshutil::cube(transform, vec4(0.6f, 0.42f, 0.42f, 1.0f));
    });

    for (std::size_t i = 0; i < meshes_.size(); ++i) {
        if (auto mesh = meshes_[i].get()) {
            meshDrawers_[i] = std::make_unique<MeshDrawerGL>(mesh);
        }
    }
}

void CameraWidget::interaction(Interaction dir, dvec2 mouseDelta) {
    switch (dir) {
        case Interaction::HorizontalRotation:
        case Interaction::VerticalRotation:
        case Interaction::FreeRotation:
        case Interaction::Roll:
            rotation(dir, mouseDelta);
            break;
        case Interaction::Zoom:
            zoom(mouseDelta);
            break;
        case Interaction::None:
        default:
            break;
    }
}

void CameraWidget::singleStepInteraction(Interaction dir, bool clockwise) {
    switch (dir) {
        case Interaction::HorizontalRotation:
        case Interaction::VerticalRotation:
        case Interaction::FreeRotation:
        case Interaction::Roll:
            saveInitialCameraState();
            singleStepRotation(dir, clockwise);
            break;
        case Interaction::Zoom:
            saveInitialCameraState();
            singleStepZoom(clockwise);
            break;
        case Interaction::None:
        default:
            break;
    }
}

void CameraWidget::rotation(Interaction dir, dvec2 mouseDelta) {
    // use delta between initial mouse position and current position for rotation around the
    // corresponding axis
    float distance = 0.0f;
    vec3 rotAxis(0.0f);
    switch (dir) {
        case Interaction::HorizontalRotation:
            distance = static_cast<float>(mouseDelta.x);
            rotAxis = initialState_.camUp;
            if (useObjectRotAxis_.get()) {
                rotAxis = getObjectRotationAxis(rotAxis);
            }
            break;
        case Interaction::VerticalRotation:
            distance = static_cast<float>(-mouseDelta.y);
            rotAxis = initialState_.camRight;
            break;
        case Interaction::FreeRotation: {
            vec2 delta(mouseDelta);
            distance = glm::length(delta);
            const auto& cam = camera_.get();
            rotAxis = glm::normalize(
                -delta.y * glm::normalize(glm::cross(cam.getDirection(), cam.getLookUp())) +
                delta.x * cam.getLookUp());
            break;
        }
        case Interaction::Roll:
            distance = static_cast<float>(mouseDelta.x);
            rotAxis = initialState_.camDir;
            break;
        case Interaction::Zoom:
        case Interaction::None:
            return;
        default:
            return;
    }
    if (std::abs(distance) < glm::epsilon<float>()) {
        // practically no change
        return;
    }

    float angle = glm::radians(distance) * speed_.get();
    if (invertDirections_.get()) {
        angle = -angle;
    }

    mat4 rotMatrix(glm::rotate(-angle, rotAxis));
    updateOutput(rotMatrix);
}

void CameraWidget::singleStepRotation(Interaction dir, bool clockwise) {
    vec3 rotAxis(0.0f);
    switch (dir) {
        case Interaction::HorizontalRotation:
            rotAxis = initialState_.camUp;
            if (useObjectRotAxis_.get()) {
                rotAxis = getObjectRotationAxis(rotAxis);
            }
            break;
        case Interaction::VerticalRotation:
            rotAxis = initialState_.camRight;
            break;
        case Interaction::Roll:
            rotAxis = initialState_.camDir;
            break;
        case Interaction::FreeRotation:
        case Interaction::Zoom:
        case Interaction::None:
            return;
        default:
            return;
    }

    float angle = glm::radians(angleIncrement_.get()) * (clockwise ? -1.0f : 1.0f);
    if (invertDirections_.get()) {
        angle = -angle;
    }

    mat4 rotMatrix(glm::rotate(-angle, rotAxis));
    updateOutput(rotMatrix);
}

void CameraWidget::zoom(dvec2 delta) {
    double f = -delta.y / 50.0;

    auto& cam = camera_.get();
    double focalLength = glm::length(cam.getDirection());
    focalLength = std::max(0.01, focalLength + f * initialState_.zoom_);

    // update camera look from position
    vec3 campos(camera_.getLookTo() - initialState_.camDir * static_cast<float>(focalLength));
    camera_.setLookFrom(campos);
}

void CameraWidget::singleStepZoom(bool zoomIn) {
    // adjust zoom direction
    dvec2 delta(angleIncrement_.get() * (zoomIn ? -1.0f : 1.0f));
    zoom(delta);
}

void CameraWidget::updateOutput(const mat4& rotation) {
    // update camera
    mat3 m(rotation);
    auto& cam = camera_.get();
    vec3 v(cam.getDirection());
    camera_.setLook(cam.getLookTo() - m * v, cam.getLookTo(), m * cam.getLookUp());

    // update rotation matrix
    rotMatrix_.set(rotation * rotMatrix_.get());
}

int CameraWidget::interactionDirectionToInt(Interaction dir) { return static_cast<int>(dir); }

CameraWidget::Interaction CameraWidget::intToInteractionDirection(int dir) {
    switch (dir) {
        case static_cast<int>(Interaction::HorizontalRotation):
            return Interaction::HorizontalRotation;
        case static_cast<int>(Interaction::VerticalRotation):
            return Interaction::VerticalRotation;
        case static_cast<int>(Interaction::FreeRotation):
            return Interaction::FreeRotation;
        default:
            return Interaction::None;
    }
}

vec3 CameraWidget::getObjectRotationAxis(const vec3& rotAxis) const {
    vec3 axis(glm::abs(rotAxis));
    // use closest object axis for horizontal rotation
    if ((axis.x > axis.y) && (axis.x > axis.z)) {
        axis = vec3(rotAxis.x, 0.0f, 0.0f);
    } else if ((axis.y > axis.x) && (axis.y > axis.z)) {
        axis = vec3(0.0f, rotAxis.y, 0.0f);
    } else {
        axis = vec3(0.0f, 0.0f, rotAxis.z);
    }
    return glm::normalize(axis);
}

}  // namespace inviwo
