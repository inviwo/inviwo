/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2021 Inviwo Foundation
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

#include <modules/basegl/processors/volumeslicegl.h>
#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/image/layergl.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/volume/volumeutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/openglutils.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/geometry/plane.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/interaction/events/keyboardevent.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/gestureevent.h>
#include <inviwo/core/interaction/events/wheelevent.h>
#include <inviwo/core/interaction/events/eventmatcher.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/io/serialization/versionconverter.h>
#include <inviwo/core/network/networklock.h>
#include <limits>

namespace inviwo {

const ProcessorInfo VolumeSliceGL::processorInfo_{
    "org.inviwo.VolumeSliceGL",  // Class identifier
    "Volume Slice Viewer",       // Display name
    "Volume Operation",          // Category
    CodeState::Stable,           // Code state
    Tags::GL,                    // Tags
};
const ProcessorInfo VolumeSliceGL::getProcessorInfo() const { return processorInfo_; }

VolumeSliceGL::VolumeSliceGL()
    : Processor()
    , inport_("volume")
    , outport_("outport", DataFormat<glm::u8vec4>::get())
    , shader_("standard.vert", "volumeslice.frag", Shader::Build::No)
    , indicatorShader_("standard.vert", "standard.frag", Shader::Build::Yes)
    , trafoGroup_("trafoGroup", "Transformations")
    , pickGroup_("pickGroup", "Position Selection")
    , tfGroup_("tfGroup", "Transfer Function Properties")
    , sliceAlongAxis_("sliceAxis", "Slice along axis",
                      {{"x", "y-z plane (X axis)", static_cast<int>(CartesianCoordinateAxis::X)},
                       {"y", "z-x plane (Y axis)", static_cast<int>(CartesianCoordinateAxis::Y)},
                       {"z", "x-y plane (Z axis)", static_cast<int>(CartesianCoordinateAxis::Z)},
                       {"p", "Plane Equation", 3}},
                      0)
    , sliceX_("sliceX", "X Volume Position", 128, 1, 256, 1, InvalidationLevel::Valid)
    , sliceY_("sliceY", "Y Volume Position", 128, 1, 256, 1, InvalidationLevel::Valid)
    , sliceZ_("sliceZ", "Z Volume Position", 128, 1, 256, 1, InvalidationLevel::Valid)
    , worldPosition_("worldPosition_", "World Position", vec3(0.0f), vec3(-10.0f), vec3(10.0f),
                     vec3(0.01f), InvalidationLevel::Valid)
    , planeNormal_("planeNormal", "Plane Normal", vec3(1.f, 0.f, 0.f), vec3(-1.f, -1.f, -1.f),
                   vec3(1.f, 1.f, 1.f), vec3(0.01f, 0.01f, 0.01f))
    , planePosition_("planePosition", "Plane Position", vec3(0.5f), vec3(0.0f), vec3(1.0f))
    , imageScale_("imageScale", "Scale", 1.0f, 0.1f, 10.0f)
    , rotationAroundAxis_("rotation", "Rotation (ccw)",
                          {{"0", "0 deg", 0},
                           {"90", "90 deg", 1},
                           {"180", "180 deg", 2},
                           {"270", "270 deg", 3},
                           {"free", "Free Rotation", 4}},
                          0, InvalidationLevel::Valid)
    , imageRotation_("imageRotation", "Angle", 0, 0, glm::radians(360.f))
    , flipHorizontal_("flipHorizontal", "Horizontal Flip", false)
    , flipVertical_("flipVertical", "Vertical Flip", false)
    , volumeWrapping_("volumeWrapping", "Volume Texture Wrapping",
                      {{"color", "Fill with Color", GL_CLAMP_TO_EDGE},
                       {"edge", "Fill with Edge", GL_CLAMP_TO_EDGE},
                       {"repeat", "Repeat", GL_REPEAT},
                       {"m-repeat", "Mirrored Repeat", GL_MIRRORED_REPEAT}},
                      0)
    , fillColor_("fillColor", "Fill Color", vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f), vec4(1.0f),
                 vec4(0.01f), InvalidationLevel::InvalidOutput, PropertySemantics::Color)
    , posPicking_("posPicking", "Enable Picking", false)
    , showIndicator_("showIndicator", "Show Position Indicator", true)
    , indicatorColor_("indicatorColor", "Indicator Color", vec4(1.0f, 0.8f, 0.1f, 0.8f), vec4(0.0f),
                      vec4(1.0f), vec4(0.01f), InvalidationLevel::InvalidOutput,
                      PropertySemantics::Color)
    , indicatorSize_("indicatorSize", "Indicator Size", 4.0f, 0.0f, 100.0f, 0.01f,
                     InvalidationLevel::InvalidOutput)
    , tfMappingEnabled_("tfMappingEnabled", "Enable Transfer Function", true,
                        InvalidationLevel::InvalidResources)
    , transferFunction_("transferFunction", "Transfer Function", &inport_)
    , tfAlphaOffset_("alphaOffset", "Alpha Offset", 0.0f, 0.0f, 1.0f, 0.01f)
    , sampleQuery_("sampleQuery", "Sampling Query", false)
    , normalizedSample_("normalizedSample", "Normalized Output", vec4(0.0f),
                        vec4(std::numeric_limits<float>::lowest()),
                        vec4(std::numeric_limits<float>::max()), vec4(0.001f),
                        InvalidationLevel::Valid, PropertySemantics::Text)
    , volumeSample_("volumeSample", "Sample Output", vec4(0.0f),
                    vec4(std::numeric_limits<float>::lowest()),
                    vec4(std::numeric_limits<float>::max()), vec4(0.001f), InvalidationLevel::Valid,
                    PropertySemantics::Text)
    , handleInteractionEvents_("handleEvents", "Handle Interaction Events", true,
                               InvalidationLevel::Valid)
    , mouseShiftSlice_(
          "mouseShiftSlice", "Mouse Slice Shift", [this](Event* e) { eventShiftSlice(e); },
          std::make_unique<WheelEventMatcher>())

    , mouseSetMarker_(
          "mouseSetMarker", "Mouse Set Marker", [this](Event* e) { eventSetMarker(e); },
          MouseButton::Left, MouseState::Press | MouseState::Move)
    , mousePositionTracker_(
          "mousePositionTracker", "Mouse Position Tracker",
          [this](Event* e) { eventUpdateMousePos(e); }, MouseButton::None, MouseState::Move)

    , stepSliceUp_(
          "stepSliceUp", "Key Slice Up", [this](Event* e) { eventStepSliceUp(e); }, IvwKey::W,
          KeyState::Press)
    , stepSliceDown_(
          "stepSliceDown", "Key Slice Down", [this](Event* e) { eventStepSliceDown(e); }, IvwKey::S,
          KeyState::Press)
    , gestureShiftSlice_(
          "gestureShiftSlice", "Gesture Slice Shift",
          [this](Event* e) { eventGestureShiftSlice(e); },
          std::make_unique<GestureEventMatcher>(GestureType::Pan, GestureStates(flags::any), 3))
    , meshDirty_(true)
    , updating_(false)
    , sliceRotation_(1.0f)
    , inverseSliceRotation_(1.0f)
    , volumeDimensions_(8u)
    , texToWorld_(1.0f) {
    addPort(inport_);
    addPort(outport_);

    inport_.onChange([this]() { updateMaxSliceNumber(); });

    sliceAlongAxis_.onChange([this]() { modeChange(); });
    addProperties(sliceAlongAxis_, sliceX_, sliceY_, sliceZ_);

    // Invalidate selected voxel cursor when current slice changes
    sliceX_.onChange([this]() { sliceChange(); });
    sliceY_.onChange([this]() { sliceChange(); });
    sliceZ_.onChange([this]() { sliceChange(); });

    addProperties(planeNormal_, planePosition_);

    planePosition_.onChange([this]() { positionChange(); });
    planeNormal_.onChange([this]() { planeSettingsChanged(); });

    volumeWrapping_.onChange([&]() {
        if (volumeWrapping_.getSelectedIdentifier() == "color") {
            shader_.getFragmentShaderObject()->addShaderDefine("COLOR_FILL_ENABLED");
            fillColor_.setVisible(true);
        } else {
            shader_.getFragmentShaderObject()->removeShaderDefine("COLOR_FILL_ENABLED");
            fillColor_.setVisible(false);
        }
        shader_.build();
    });
    shader_.getFragmentShaderObject()->addShaderDefine("COLOR_FILL_ENABLED");
    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
    indicatorShader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });

    imageRotation_.setVisible(false);

    trafoGroup_.addProperties(rotationAroundAxis_, imageRotation_, imageScale_, flipHorizontal_,
                              flipVertical_, volumeWrapping_, fillColor_);

    rotationAroundAxis_.onChange([this]() { rotationModeChange(); });
    imageRotation_.onChange([this]() { planeSettingsChanged(); });
    imageScale_.onChange([this]() { planeSettingsChanged(); });
    flipHorizontal_.onChange([this]() { planeSettingsChanged(); });
    flipVertical_.onChange([this]() { planeSettingsChanged(); });

    addProperty(trafoGroup_);

    // Position Selection
    pickGroup_.addProperties(posPicking_, showIndicator_, indicatorColor_, indicatorSize_);

    posPicking_.onChange([this]() { modeChange(); });
    indicatorColor_.onChange([this]() { invalidateMesh(); });
    indicatorSize_.onChange([this]() { invalidateMesh(); });
    showIndicator_.setReadOnly(posPicking_.get());
    indicatorColor_.setSemantics(PropertySemantics::Color);

    addProperty(pickGroup_);

    // Transfer Function
    tfGroup_.addProperty(tfMappingEnabled_);
    // Make sure that opacity does not affect the mapped color.
    if (!transferFunction_.get().empty()) {
        transferFunction_.get()[0].setAlpha(1.f);
    }
    transferFunction_.setCurrentStateAsDefault();
    tfGroup_.addProperty(transferFunction_);
    tfGroup_.addProperty(tfAlphaOffset_);
    addProperty(tfGroup_);

    sampleQuery_.onChange([&]() {
        if (!sampleQuery_.isChecked()) {
            normalizedSample_.set(vec4(-std::numeric_limits<float>::infinity()));
            volumeSample_.set(vec4(-std::numeric_limits<float>::infinity()));
        }
    });

    normalizedSample_.setReadOnly(true);
    normalizedSample_.setSerializationMode(PropertySerializationMode::None);
    normalizedSample_.setCurrentStateAsDefault();
    volumeSample_.setReadOnly(true);
    volumeSample_.setSerializationMode(PropertySerializationMode::None);
    volumeSample_.setCurrentStateAsDefault();

    sampleQuery_.addProperty(normalizedSample_);
    sampleQuery_.addProperty(volumeSample_);
    addProperty(sampleQuery_);

    worldPosition_.setReadOnly(true);
    addProperty(worldPosition_);
    worldPosition_.onChange([this]() { updateFromWorldPosition(); });

    addProperty(handleInteractionEvents_);

    mouseShiftSlice_.setVisible(false);
    mouseShiftSlice_.setCurrentStateAsDefault();
    addProperty(mouseShiftSlice_);

    addProperty(stepSliceUp_);
    addProperty(stepSliceDown_);
    addProperty(mouseSetMarker_);

    mousePositionTracker_.setVisible(false);
    mousePositionTracker_.setCurrentStateAsDefault();
    addProperty(mousePositionTracker_);

    gestureShiftSlice_.setVisible(false);
    gestureShiftSlice_.setCurrentStateAsDefault();
    addProperty(gestureShiftSlice_);
}

VolumeSliceGL::~VolumeSliceGL() = default;

void VolumeSliceGL::initializeResources() {
    updateMaxSliceNumber();

    if (tfMappingEnabled_.get()) {
        shader_.getFragmentShaderObject()->addShaderDefine("TF_MAPPING_ENABLED");
        transferFunction_.setVisible(true);
        tfAlphaOffset_.setVisible(true);
    } else {
        shader_.getFragmentShaderObject()->removeShaderDefine("TF_MAPPING_ENABLED");
        transferFunction_.setVisible(false);
        tfAlphaOffset_.setVisible(false);
    }
    shader_.build();
    planeSettingsChanged();
}

void VolumeSliceGL::invokeEvent(Event* event) {
    if (dynamic_cast<InteractionEvent*>(event) && !handleInteractionEvents_) return;
    Processor::invokeEvent(event);
    if (event->getAs<ResizeEvent>()) {
        planeSettingsChanged();
    }
}

void VolumeSliceGL::modeChange() {
    NetworkLock lock(this);

    sliceX_.setReadOnly(true);
    sliceY_.setReadOnly(true);
    sliceZ_.setReadOnly(true);
    planePosition_.setReadOnly(true);
    planeNormal_.setReadOnly(true);

    switch (sliceAlongAxis_.get()) {
        case static_cast<int>(CartesianCoordinateAxis::X):
            sliceX_.setReadOnly(false);
            planeNormal_.set(vec3(-1.0f, 0.0f, 0.0f));
            sliceChange();
            break;
        case static_cast<int>(CartesianCoordinateAxis::Y):
            sliceY_.setReadOnly(false);
            planeNormal_.set(vec3(0.0f, -1.0f, 0.0f));
            sliceChange();
            break;
        case static_cast<int>(CartesianCoordinateAxis::Z):
            sliceZ_.setReadOnly(false);
            planeNormal_.set(vec3(0.0f, 0.0f, -1.0f));
            sliceChange();
            break;
        case 3:  // General plane
        default:
            planePosition_.setReadOnly(false);
            planeNormal_.setReadOnly(false);
            break;
    }

    showIndicator_.setReadOnly(!posPicking_.get());

    planeSettingsChanged();
}

void VolumeSliceGL::planeSettingsChanged() {
    if (!inport_.hasData()) return;

    // Make sure we keep the aspect of the input data.

    // In texture space
    const vec3 normal = glm::normalize(planeNormal_.get());
    const Plane plane(planePosition_.get(), normal);

    // In worldSpace, ignoring translation because it should not affect rotation (fixes issue #875)
    const mat4 texToWorld(
        mat3(inport_.getData()->getCoordinateTransformer().getTextureToWorldMatrix()));

    const vec3 worldNormal(
        glm::normalize(vec3(glm::inverseTranspose(texToWorld) * vec4(normal, 0.0f))));
    const mat4 boxrotation(glm::toMat4(glm::rotation(worldNormal, vec3(0.0f, 0.0f, 1.0f))));

    // Construct the edges of a unit box and intersect with the plane.
    std::vector<std::optional<vec3>> points;
    points.reserve(12);

    points.push_back(plane.getIntersection(vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f)));
    points.push_back(plane.getIntersection(vec3(1.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 0.0f)));
    points.push_back(plane.getIntersection(vec3(1.0f, 1.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f)));
    points.push_back(plane.getIntersection(vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f)));

    points.push_back(plane.getIntersection(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f)));
    points.push_back(plane.getIntersection(vec3(1.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 1.0f)));
    points.push_back(plane.getIntersection(vec3(1.0f, 1.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f)));
    points.push_back(plane.getIntersection(vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 1.0f, 1.0f)));

    points.push_back(plane.getIntersection(vec3(0.0f, 0.0f, 1.0f), vec3(1.0f, 0.0f, 1.0f)));
    points.push_back(plane.getIntersection(vec3(1.0f, 0.0f, 1.0f), vec3(1.0f, 1.0f, 1.0f)));
    points.push_back(plane.getIntersection(vec3(1.0f, 1.0f, 1.0f), vec3(0.0f, 1.0f, 1.0f)));
    points.push_back(plane.getIntersection(vec3(0.0f, 1.0f, 1.0f), vec3(0.0f, 0.0f, 1.0f)));

    // Calculate the aspect of the intersected plane in world space.
    vec2 xrange(std::numeric_limits<float>::max(), std::numeric_limits<float>::min());
    vec2 yrange(std::numeric_limits<float>::max(), std::numeric_limits<float>::min());
    for (auto& point : points) {
        if (point) {
            vec4 corner = vec4(*point, 1.0f);
            corner = boxrotation * texToWorld * corner;

            xrange[0] = std::min(xrange[0], corner.x);
            xrange[1] = std::max(xrange[1], corner.x);
            yrange[0] = std::min(yrange[0], corner.y);
            yrange[1] = std::max(yrange[1], corner.y);
        }
    }
    const float sourceRatio = glm::abs((xrange[1] - xrange[0]) / (yrange[1] - yrange[0]));

    // Goal: define a transformation that maps the view 2D texture coordinates into
    // 3D texture coordinates at at some plane in the volume.
    const mat4 flipMatX(-1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1);
    const mat4 flipMatY(1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1);

    const vec2 targetDim(outport_.getDimensions());
    const float targetRatio = targetDim.x / targetDim.y;

    const vec3 scaleSource(imageScale_ / (sourceRatio > 1.0f ? 1.0f : sourceRatio),
                           imageScale_ * (sourceRatio > 1.0f ? sourceRatio : 1.0f), 1.0f);

    const vec3 scaleTarget(1.0f * (targetRatio < 1.0f ? 1.0f : targetRatio),
                           1.0f / (targetRatio < 1.0f ? targetRatio : 1.0f), 1.0f);

    mat4 rotation(
        glm::translate(vec3(0.5f)) * glm::toMat4(glm::rotation(vec3(0.0f, 0.0f, 1.0f), normal)) *
        glm::scale(scaleSource) * glm::rotate(imageRotation_.get(), vec3(0.0f, 0.0f, 1.0f)) *
        glm::scale(scaleTarget) * glm::translate(vec3(-0.5f)));

    if (flipHorizontal_) rotation *= flipMatX;
    if (flipVertical_) rotation *= flipMatY;

    // Save the inverse rotation.
    sliceRotation_ = rotation;
    inverseSliceRotation_ = glm::inverse(rotation);

    invalidateMesh();
    return;
}

void VolumeSliceGL::process() {
    if (inport_.isChanged()) {
        if (volumeDimensions_ != inport_.getData()->getDimensions()) {
            volumeDimensions_ = inport_.getData()->getDimensions();
            updateMaxSliceNumber();
            modeChange();
        }
        if (texToWorld_ !=
            inport_.getData()->getCoordinateTransformer().getTextureToWorldMatrix()) {
            texToWorld_ = inport_.getData()->getCoordinateTransformer().getTextureToWorldMatrix();
            planeSettingsChanged();
        }
    }

    utilgl::activateAndClearTarget(outport_, ImageType::ColorOnly);
    shader_.activate();

    TextureUnitContainer units;
    utilgl::bindAndSetUniforms(shader_, units, inport_);

    utilgl::TexParameter wraps(units[0], GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, volumeWrapping_.get());
    utilgl::TexParameter wrapt(units[0], GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, volumeWrapping_.get());
    utilgl::TexParameter wrapr(units[0], GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, volumeWrapping_.get());

    if (tfMappingEnabled_) utilgl::bindAndSetUniforms(shader_, units, transferFunction_);

    utilgl::setUniforms(shader_, tfAlphaOffset_, fillColor_);
    shader_.setUniform("sliceRotation", sliceRotation_);
    shader_.setUniform("slice", (inverseSliceRotation_ * vec4(planePosition_.get(), 1.0f)).z);
    shader_.setUniform("dataToClip", mat4(1.0f));

    utilgl::singleDrawImagePlaneRect();
    shader_.deactivate();

    if (posPicking_.get() && showIndicator_.get()) renderPositionIndicator();
    utilgl::deactivateCurrentTarget();

    // update volume sample from indicator position, if active
    if (sampleQuery_.isChecked() && posPicking_.get()) {
        // convert world volume position to voxel coords
        auto volume = inport_.getData();
        const mat4 worldToIndex(volume->getCoordinateTransformer().getWorldToIndexMatrix());
        const vec3 indexPos(ivec3(worldToIndex * vec4(worldPosition_.get(), 1)));
        if (glm::all(glm::greaterThanEqual(indexPos, vec3(0))) && glm::all(glm::lessThan(indexPos, vec3(volume->getDimensions())))) {
            const auto volumeRAM = volume->getRepresentation<VolumeRAM>();
            normalizedSample_.set(volumeRAM->getAsNormalizedDVec4(indexPos));
            volumeSample_.set(volumeRAM->getAsDVec4(indexPos));
        }
    }
}

void VolumeSliceGL::renderPositionIndicator() {
    if (meshDirty_) {
        updateIndicatorMesh();
    }

    MeshDrawerGL drawer(&meshCrossHair_);

    utilgl::GlBoolState smooth(GL_LINE_SMOOTH, true);
    utilgl::BlendModeState blend(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    indicatorShader_.activate();
    indicatorShader_.setUniform("dataToClip", mat4(1.0f));

    utilgl::DepthFuncState depth(GL_ALWAYS);
    drawer.draw();
    indicatorShader_.deactivate();
}

VolumeSliceGL::ColoredMesh2D VolumeSliceGL::createIndicatorMesh() {

    // indices for cross lines
    auto indexBuf1 =
        util::makeIndexBuffer(util::table([&](int i) { return static_cast<uint32_t>(i); }, 0, 8));

    // indices for box lines
    auto indexBuf2 =
        util::makeIndexBuffer(util::table([&](int i) { return static_cast<uint32_t>(i); }, 8, 12));

    ColoredMesh2D meshCrossHair(DrawType::Lines, ConnectivityType::None);
    meshCrossHair.setModelMatrix(mat4(1.0f));
    // Add 12 arbitrary vertices, which need to be updated later
    meshCrossHair.addVertices({{vec2(0.f), vec4(1.f)},
                               {vec2(0.f), vec4(1.f)},
                               {vec2(0.f), vec4(1.f)},
                               {vec2(0.f), vec4(1.f)},
                               {vec2(0.f), vec4(1.f)},
                               {vec2(0.f), vec4(1.f)},
                               {vec2(0.f), vec4(1.f)},
                               {vec2(0.f), vec4(1.f)},
                               {vec2(0.f), vec4(1.f)},
                               {vec2(0.f), vec4(1.f)},
                               {vec2(0.f), vec4(1.f)},
                               {vec2(0.f), vec4(1.f)}});
    meshCrossHair.addIndices(Mesh::MeshInfo(DrawType::Lines, ConnectivityType::None), indexBuf1);
    meshCrossHair.addIndices(Mesh::MeshInfo(DrawType::Lines, ConnectivityType::Loop), indexBuf2);

    return meshCrossHair;
}

void VolumeSliceGL::updateIndicatorMesh() {
    const vec2 pos = getScreenPosFromVolPos();

    const size2_t canvasSize(outport_.getDimensions());
    const vec2 indicatorSize =
        vec2(indicatorSize_.get() / canvasSize.x, indicatorSize_.get() / canvasSize.y);

    // add two vertical and two horizontal lines with a gap around the selected position
    auto posBuf =
        meshCrossHair_.getTypedEditableRAMRepresentation<buffertraits::PositionsBuffer2D>();
    posBuf->getDataContainer() = {
        // horizontal
        vec2(-0.5f, pos.y) * 2.0f - 1.0f, vec2(pos.x - indicatorSize.x, pos.y) * 2.0f - 1.0f,
        vec2(pos.x + indicatorSize.x, pos.y) * 2.0f - 1.0f, vec2(1.5f, pos.y) * 2.0f - 1.0f,

        // vertical
        vec2(pos.x, -0.5f) * 2.0f - 1.0f, vec2(pos.x, pos.y - indicatorSize.y) * 2.0f - 1.0f,
        vec2(pos.x, pos.y + indicatorSize.y) * 2.0f - 1.0f, vec2(pos.x, 1.5f) * 2.0f - 1.0f,

        // box
        vec2(pos.x - indicatorSize.x, pos.y - indicatorSize.y) * 2.0f - 1.0f,
        vec2(pos.x + indicatorSize.x, pos.y - indicatorSize.y) * 2.0f - 1.0f,
        vec2(pos.x + indicatorSize.x, pos.y + indicatorSize.y) * 2.0f - 1.0f,
        vec2(pos.x - indicatorSize.x, pos.y + indicatorSize.y) * 2.0f - 1.0f};

    auto colorBuf = meshCrossHair_.getTypedEditableRAMRepresentation<buffertraits::ColorsBuffer>();
    const vec4 color(indicatorColor_.get());
    colorBuf->getDataContainer() = std::vector<vec4>(12, color);

    meshDirty_ = false;
}

void VolumeSliceGL::invalidateMesh() { meshDirty_ = true; }

void VolumeSliceGL::shiftSlice(int shift) {
    switch (sliceAlongAxis_.get()) {
        case 0:  // x axis
        {
            int newValue = sliceX_.get() + shift;
            newValue = glm::clamp(newValue, sliceX_.getMinValue(), sliceX_.getMaxValue());
            sliceX_.set(newValue);
            break;
        }
        case 1:  // y axis
        {
            int newValue = sliceY_.get() + shift;
            newValue = glm::clamp(newValue, sliceY_.getMinValue(), sliceY_.getMaxValue());
            sliceY_.set(newValue);
            break;
        }
        case 2:  // z axis
        {
            int newValue = sliceZ_.get() + shift;
            newValue = glm::clamp(newValue, sliceZ_.getMinValue(), sliceZ_.getMaxValue());
            sliceZ_.set(newValue);
            break;
        }
        default:
        case 3: {
            vec3 newPos = planePosition_.get() +
                          static_cast<float>(shift) / 100.0f * glm::normalize(planeNormal_.get());
            newPos = glm::clamp(newPos, vec3(0.0f), vec3(1.0f));
            planePosition_.set(newPos);
            break;
        }
    }
}

void VolumeSliceGL::setVolPosFromScreenPos(vec2 pos) {
    if (!posPicking_.get()) return;  // position mode not enabled

    vec3 planePos = convertScreenPosToVolume(pos);

    invalidateMesh();
    planePosition_.set(vec3(planePos));
}

vec2 VolumeSliceGL::getScreenPosFromVolPos() {
    vec2 pos(inverseSliceRotation_ * vec4(planePosition_.get(), 1.0f));
    return pos;
}

vec3 VolumeSliceGL::convertScreenPosToVolume(const vec2& screenPos, bool clamp) const {
    vec2 pos = vec2(glm::translate(vec3(0.5f, 0.5f, 0.0f)) *
                    glm::translate(vec3(-0.5f, -0.5f, 0.0f)) * vec4(screenPos, 0.0f, 1.0f));

    if (clamp) {
        if ((pos.x < 0.0f) || (pos.x > 1.0f) || (pos.y < 0.0f) || (pos.y > 1.0f)) {
            pos = glm::clamp(pos, vec2(0.0f), vec2(1.0f));
        }
    }

    vec4 newpos(inverseSliceRotation_ * vec4(planePosition_.get(), 1.0f));
    newpos.x = pos.x;
    newpos.y = pos.y;
    newpos = sliceRotation_ * newpos;

    if (clamp) {
        newpos = glm::clamp(newpos, vec4(0.0f), vec4(1.0f));
    }
    return vec3(newpos);
}

void VolumeSliceGL::updateMaxSliceNumber() {
    if (!inport_.hasData()) {
        return;
    }
    NetworkLock lock(this);

    const size3_t dims{inport_.getData()->getDimensions()};
    if (static_cast<int>(dims.x) != sliceX_.getMaxValue()) {
        sliceX_.setMaxValue(static_cast<int>(dims.x));
        sliceX_.set(static_cast<int>(dims.x) / 2);
    }
    if (static_cast<int>(dims.y) != sliceY_.getMaxValue()) {
        sliceY_.setMaxValue(static_cast<int>(dims.y));
        sliceY_.set(static_cast<int>(dims.y) / 2);
    }
    if (static_cast<int>(dims.z) != sliceZ_.getMaxValue()) {
        sliceZ_.setMaxValue(static_cast<int>(dims.z));
        sliceZ_.set(static_cast<int>(dims.z) / 2);
    }

    mat4 texToWorld(inport_.getData()->getCoordinateTransformer().getTextureToWorldMatrix());

    vec3 max(texToWorld * vec4(1.0f));
    vec3 min(texToWorld * vec4(0.0f, 0.0f, 0.0f, 1.0f));
    // Rotation/mirroring may change the sign so apply min/max
    worldPosition_.setMaxValue(glm::max(min, max));
    worldPosition_.setMinValue(glm::min(min, max));
}

void VolumeSliceGL::eventShiftSlice(Event* event) {
    auto wheelEvent = static_cast<WheelEvent*>(event);
    int steps = static_cast<int>(wheelEvent->delta().y);
    shiftSlice(steps);
    event->markAsUsed();
}

void VolumeSliceGL::eventSetMarker(Event* event) {
    auto mouseEvent = static_cast<MouseEvent*>(event);
    setVolPosFromScreenPos(vec2(mouseEvent->posNormalized()));
    event->markAsUsed();
}

void VolumeSliceGL::eventStepSliceUp(Event* event) {
    shiftSlice(1);
    event->markAsUsed();
}

void VolumeSliceGL::eventStepSliceDown(Event* event) {
    shiftSlice(-1);
    event->markAsUsed();
}

void VolumeSliceGL::eventGestureShiftSlice(Event* event) {
    GestureEvent* gestureEvent = static_cast<GestureEvent*>(event);
    if (gestureEvent->deltaPos().y < 0) {
        shiftSlice(1);
        event->markAsUsed();

    } else if (gestureEvent->deltaPos().y > 0) {
        shiftSlice(-1);
        event->markAsUsed();
    }
}

void VolumeSliceGL::eventUpdateMousePos(Event* event) {
    if (!sampleQuery_.isChecked()) return;

    if (!inport_.hasData()) {
        return;
    }

    auto volume = inport_.getData();

    // if position indicator is active, update volume sample from indicator position instead of
    // mouse position (this is done in process())
    if (!posPicking_.get()) {
        auto mouseEvent = static_cast<MouseEvent*>(event);

        auto volPos = convertScreenPosToVolume(vec2(mouseEvent->posNormalized()), false);
        // convert normalized volume position to voxel coords
        const mat4 textureToIndex(volume->getCoordinateTransformer().getTextureToIndexMatrix());
        const vec4 texturePos(volPos, 1.0);
        ivec3 indexPos(ivec3(textureToIndex * texturePos));

        const ivec3 volDim(volume->getDimensions());

        bool outOfBounds = glm::any(glm::greaterThanEqual(indexPos, volDim)) ||
                           glm::any(glm::lessThan(indexPos, ivec3(0)));
        if (outOfBounds) {
            normalizedSample_.set(vec4(-std::numeric_limits<float>::infinity()));
            volumeSample_.set(vec4(-std::numeric_limits<float>::infinity()));
        } else {
            // sample input volume at given index position
            const auto volumeRAM = volume->getRepresentation<VolumeRAM>();
            normalizedSample_.set(volumeRAM->getAsNormalizedDVec4(indexPos));
            volumeSample_.set(volumeRAM->getAsDVec4(indexPos));
        }
        event->markAsUsed();
    }
}

void VolumeSliceGL::sliceChange() {
    if (!inport_.hasData() || updating_) return;
    util::KeepTrueWhileInScope guard(&updating_);

    const mat4 indexToTexture(
        inport_.getData()->getCoordinateTransformer().getIndexToTextureMatrix());
    const ivec4 indexPos(sliceX_.get() - 1, sliceY_.get() - 1, sliceZ_.get() - 1, 1.0);
    const vec3 texturePos(vec3(indexToTexture * vec4(indexPos)));

    const mat4 indexToWorld(inport_.getData()->getCoordinateTransformer().getIndexToWorldMatrix());
    const vec3 worldPos =
        vec3(indexToWorld * vec4(sliceX_.get(), sliceY_.get(), sliceZ_.get(), 1.0f));

    {
        NetworkLock lock(this);
        planePosition_.set(texturePos);
        worldPosition_.set(worldPos);
    }
}

void VolumeSliceGL::positionChange() {
    if (!inport_.hasData() || updating_) return;
    util::KeepTrueWhileInScope guard(&updating_);

    const mat4 textureToIndex(
        inport_.getData()->getCoordinateTransformer().getTextureToIndexMatrix());
    const vec4 texturePos(planePosition_.get(), 1.0);
    const ivec3 indexPos(ivec3(textureToIndex * texturePos) + ivec3(1));

    const mat4 textureToWorld(
        inport_.getData()->getCoordinateTransformer().getTextureToWorldMatrix());
    const vec3 worldPos = vec3(textureToWorld * vec4(planePosition_.get(), 1.0f));

    {
        NetworkLock lock(this);
        sliceX_.set(indexPos.x);
        sliceY_.set(indexPos.y);
        sliceZ_.set(indexPos.z);
        worldPosition_.set(worldPos);
    }

    invalidateMesh();
}

void VolumeSliceGL::rotationModeChange() {
    switch (rotationAroundAxis_.get()) {
        case 0:
            imageRotation_.setVisible(false);
            imageRotation_.set(glm::radians(0.f));
            break;
        case 1:
            imageRotation_.setVisible(false);
            imageRotation_.set(glm::radians(90.f));
            break;
        case 2:
            imageRotation_.setVisible(false);
            imageRotation_.set(glm::radians(180.f));
            break;
        case 3:
            imageRotation_.setVisible(false);
            imageRotation_.set(glm::radians(270.f));
            break;
        case 4:
        default:
            imageRotation_.setVisible(true);
            break;
    }
}

void VolumeSliceGL::updateFromWorldPosition() {

    if (!inport_.hasData() || updating_) return;

    if (inport_.hasData()) {
        util::KeepTrueWhileInScope guard(&updating_);

        const mat4 worldToTexture(
            inport_.getData()->getCoordinateTransformer().getWorldToTextureMatrix());
        const vec3 texturePos = vec3(worldToTexture * vec4(worldPosition_.get(), 1.0f));

        const mat4 worldToIndex(
            inport_.getData()->getCoordinateTransformer().getWorldToIndexMatrix());
        const ivec3 indexPos(ivec3(worldToIndex * vec4(worldPosition_.get(), 1.0f)) + ivec3(1));

        NetworkLock lock(this);
        sliceX_.set(indexPos.x);
        sliceY_.set(indexPos.y);
        sliceZ_.set(indexPos.z);

        planePosition_.set(texturePos);
        invalidateMesh();
    }
}

// override to do member renaming.
void VolumeSliceGL::deserialize(Deserializer& d) {
    util::renamePort(d, {{&inport_, "volume.inport"}, {&outport_, "image.outport"}});
    Processor::deserialize(d);
}

}  // namespace inviwo
