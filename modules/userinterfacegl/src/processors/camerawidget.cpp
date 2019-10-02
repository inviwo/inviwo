/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>
#include <inviwo/core/rendering/meshdrawerfactory.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/touchevent.h>
#include <inviwo/core/interaction/events/pickingevent.h>
#include <inviwo/core/util/filesystem.h>
#include <modules/opengl/geometry/meshgl.h>
#include <modules/opengl/image/imagegl.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/glformats.h>

#include <modules/base/algorithm/meshutils.h>

#include <modules/assimp/assimpreader.h>

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
    , userColor_("userColor", "User Color", vec4(vec3(0.7f), 1.0f), vec4(0.0f), vec4(1.0f))
    , cubeColor_("cubeColor", "Cube Color", vec4(0.11f, 0.42f, 0.63f, 1.0f), vec4(0.0f), vec4(1.0f))

    , interactions_("interactions", "Interactions")

    , rotateUpBtn_("rotateUpButton", "Rotate Up")
    , rotateDownBtn_("rotateDownButton", "Rotate Down")
    , rotateLeftBtn_("rotateLeftButton", "Rotate Left")
    , rotateRightBtn_("rotateRightButton", "Rotate Right")

    , outputProps_("outputProperties", "Output")
    , camera_("camera", "Camera")
    , rotMatrix_("rotMatrix", "Rotation Matrix", mat4(1.0f))

    , internalProps_("internalProperties", "Internal Properties")
    , internalCamera_(vec3(0.0f, 0.0f, 14.6f), vec3(0.0f), vec3(0.0f, 1.0f, 0.0f), 1.0f, 30.0f,
                      38.0f, 1.0f)
    , lightingProperty_("internalLighting", "Lighting", nullptr)

    , picking_(this, numInteractionWidgets, [&](PickingEvent *p) { objectPicked(p); })
    , shader_("widgetrenderer.vert", "geometryrendering.frag", false)
    , cubeShader_("geometryrendering.vert", "geometryrendering.frag", false)
    , overlayShader_("img_identity.vert", "widgettexture.frag")
    , isMouseBeingPressedAndHold_(false)
    , mouseWasMoved_(false)
    , currentPickingID_(-1) {
    addPort(inport_);
    addPort(outport_);

    inport_.setOptional(true);

    userColor_.setSemantics(PropertySemantics::Color);
    userColor_.setCurrentStateAsDefault();
    cubeColor_.setSemantics(PropertySemantics::Color);
    cubeColor_.setCurrentStateAsDefault();

    settings_.setCollapsed(true);
    settings_.setCurrentStateAsDefault();
    internalProps_.setCollapsed(true);
    internalProps_.setCurrentStateAsDefault();

    // interaction settings
    enabled_.onChange([this]() { picking_.setEnabled(enabled_); });
    settings_.addProperty(enabled_);
    settings_.addProperty(invertDirections_);
    settings_.addProperty(useObjectRotAxis_);
    settings_.addProperty(showRollWidget_);
    settings_.addProperty(showDollyWidget_);
    settings_.addProperty(speed_);
    settings_.addProperty(angleIncrement_);
    settings_.addProperty(minTouchMovement_);

    // widget appearance
    customColorComposite_.addProperty(axisColoring_);
    customColorComposite_.addProperty(userColor_);
    appearance_.addProperty(position_);
    appearance_.addProperty(anchorPos_);
    appearance_.addProperty(scaling_);
    appearance_.addProperty(showCube_);
    appearance_.addProperty(cubeColor_);
    appearance_.addProperty(customColorComposite_);

    // button interactions
    interactions_.setCollapsed(true);

    rotateUpBtn_.onChange([&]() { singleStepInteraction(Interaction::VerticalRotation, true); });
    rotateDownBtn_.onChange([&]() { singleStepInteraction(Interaction::VerticalRotation, false); });
    rotateLeftBtn_.onChange(
        [&]() { singleStepInteraction(Interaction::HorizontalRotation, true); });
    rotateRightBtn_.onChange(
        [&]() { singleStepInteraction(Interaction::HorizontalRotation, false); });

    interactions_.addProperty(rotateUpBtn_);
    interactions_.addProperty(rotateDownBtn_);
    interactions_.addProperty(rotateLeftBtn_);
    interactions_.addProperty(rotateRightBtn_);

    // output properties
    camera_.setCollapsed(true);
    outputProps_.addProperty(camera_);
    outputProps_.addProperty(rotMatrix_);

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
    lightingProperty_.lightPosition_ = vec3(2.5f, 5.6f, 20.0f);
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

    // combine the previously rendered widget image with the input
    if (inport_.isReady()) {
        utilgl::activateTargetAndCopySource(outport_, inport_);
    } else {
        utilgl::activateAndClearTarget(outport_, ImageType::ColorDepthPicking);
    }

    utilgl::GlBoolState depthTest(GL_DEPTH_TEST, true);
    utilgl::BlendModeState blending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // determine size of the widget
    const vec2 referenceSize(
        300.0f);  // reference size (width/height) in pixel for scale equal to 1.0
    ivec2 widgetSize(scaling_.get() * referenceSize);

    updateWidgetTexture(widgetSize);
    drawWidgetTexture();
    utilgl::deactivateCurrentTarget();
}

void CameraWidget::initializeResources() {
    // shading defines
    utilgl::addShaderDefines(shader_, lightingProperty_);
    utilgl::addShaderDefines(cubeShader_, lightingProperty_);

    if (customColorComposite_.isChecked()) {
        shader_.getVertexShaderObject()->addShaderDefine("CUSTOM_COLOR",
                                                         axisColoring_.get() ? "1" : "0");
    } else {
        shader_.getVertexShaderObject()->removeShaderDefine("CUSTOM_COLOR");
    }
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

void CameraWidget::updateWidgetTexture(const ivec2 &widgetSize) {

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

    auto *meshDrawer = meshDrawers_[showRollWidget_.get() ? 1 : 0].get();
    utilgl::setShaderUniforms(shader_, *meshDrawer->getMesh(), "geometry");

    meshDrawer->draw();

    // draw zoom buttons
    if (showDollyWidget_.get()) {
        utilgl::setShaderUniforms(shader_, *meshDrawers_[2]->getMesh(), "geometry");
        meshDrawers_[2]->draw();
    }

    if (showCube_.get()) {
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

void CameraWidget::objectPicked(PickingEvent *e) {
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
    auto &cam = camera_.get();
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
        auto &perspCam = dynamic_cast<PerspectiveCamera &>(cam);
        double fovy = perspCam.getFovy();
        initialState_.zoom_ = 1.0 / std::tan(fovy * glm::pi<double>() / 360.0);
    } catch (std::bad_cast &) {
        // camera is not a PerspectiveCamera
        initialState_.zoom_ = 1.0;
    }
}

void CameraWidget::loadMesh() {
    auto module = InviwoApplication::getPtr()->getModuleByIdentifier("UserInterfaceGL");
    if (!module) {
        throw Exception("Could not locate module 'UserInterfaceGL'");
    }

    std::string basePath(module->getPath(ModulePath::Data));
    basePath += "/meshes/";

    static std::array<std::shared_ptr<const Mesh>, 4> meshes = [basePath]() {
        std::array<std::shared_ptr<const Mesh>, 4> res;

        AssimpReader meshReader;
        meshReader.setLogLevel(AssimpLogLevel::Error);
        meshReader.setFixInvalidDataFlag(false);

        // camera interaction widgets
        res[0] = meshReader.readData(basePath + "camera-widget.fbx");
        // widget including "camera roll" arrow
        res[1] = meshReader.readData(basePath + "camera-widget-roll.fbx");
        // camera zoom buttons
        res[2] = meshReader.readData(basePath + "camera-zoom.fbx");

        // cube mesh for orientation
        const vec3 cubeScale(8.0f);
        mat4 transform(glm::scale(vec3(cubeScale)));
        transform[3] = vec4(-0.5f * cubeScale, 1.0f);
        res[3] = meshutil::cube(transform, vec4(0.6f, 0.42f, 0.42f, 1.0f));

        return res;
    }();

    meshes_ = meshes;

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
            const auto &cam = camera_.get();
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

    auto &cam = camera_.get();
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

void CameraWidget::updateOutput(const mat4 &rotation) {
    // update camera
    mat3 m(rotation);
    auto &cam = camera_.get();
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

vec3 CameraWidget::getObjectRotationAxis(const vec3 &rotAxis) const {
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
