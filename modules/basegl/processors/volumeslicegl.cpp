/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include "volumeslicegl.h"
#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/image/layergl.h>
#include <modules/opengl/rendering/meshrenderer.h>
#include <modules/opengl/glwrap/shader.h>
#include <modules/opengl/glwrap/textureunit.h>
#include <modules/opengl/textureutils.h>
#include <modules/opengl/volumeutils.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/geometry/plane.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <inviwo/core/interaction/events/keyboardevent.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/gestureevent.h>
#include <limits>

namespace inviwo {

ProcessorClassIdentifier(VolumeSliceGL, "org.inviwo.VolumeSliceGL");
ProcessorDisplayName(VolumeSliceGL, "Volume Slice");
ProcessorTags(VolumeSliceGL, Tags::GL);
ProcessorCategory(VolumeSliceGL, "Volume Operation");
ProcessorCodeState(VolumeSliceGL, CODE_STATE_STABLE);

VolumeSliceGL::VolumeSliceGL()
    : Processor()
    , inport_("volume.inport")
    , outport_("image.outport", COLOR_ONLY)
    , trafoGroup_("trafoGroup", "Transformations")
    , pickGroup_("pickGroup", "Position Selection")
    , tfGroup_("tfGroup", "Transfer Function")
    , sliceAlongAxis_("sliceAxis", "Slice along axis")
    , sliceX_("sliceX", "X Volume Position", 128, 1, 256, 1, VALID)
    , sliceY_("sliceY", "Y Volume Position", 128, 1, 256, 1, VALID)
    , sliceZ_("sliceZ", "Z Volume Position", 128, 1, 256, 1, VALID)
    , worldPosition_("worldPosition_", "World Position", vec3(0.0f), vec3(-10.0f), vec3(10.0f),
                     vec3(0.01f), VALID)
    , planeNormal_("planeNormal", "Plane Normal", vec3(1.f, 0.f, 0.f), vec3(-1.f, -1.f, -1.f),
                   vec3(1.f, 1.f, 1.f), vec3(0.01f, 0.01f, 0.01f))
    , planePosition_("planePosition", "Plane Position", vec3(0.5f), vec3(0.0f), vec3(1.0f))
    , imageScale_("imageScale", "Scale", 1.0f, 0.1f, 10.0f)
    , rotationAroundAxis_("rotation", "Rotation (ccw)", VALID)
    , imageRotation_("imageRotation", "Angle", 0, 0, glm::radians(360.f))
    , flipHorizontal_("flipHorizontal", "Flip Horizontal View", false)
    , flipVertical_("flipVertical", "Flip Vertical View", false)
    
    , volumeWrapping_("volumeWrapping", "Volume Texture Wrapping")
    , posPicking_("posPicking", "Enable Picking", false)
    , showIndicator_("showIndicator", "Show Position Indicator", true)
    , indicatorColor_("indicatorColor", "Indicator Color", vec4(1.0f, 0.8f, 0.1f, 0.8f), vec4(0.0f),
                      vec4(1.0f), vec4(0.01f), INVALID_OUTPUT, PropertySemantics::Color)
    , tfMappingEnabled_("tfMappingEnabled", "Enable Transfer Function", true, INVALID_RESOURCES)
    , transferFunction_("transferFunction", "Transfer function", TransferFunction(), &inport_)
    , tfAlphaOffset_("alphaOffset", "Alpha Offset", 0.0f, 0.0f, 1.0f, 0.01f)
    , handleInteractionEvents_("handleEvents", "Handle interaction events", true, VALID)
    , mouseShiftSlice_(
          "mouseShiftSlice", "Mouse Slice Shift",
          new MouseEvent(MouseEvent::MOUSE_BUTTON_NONE, InteractionEvent::MODIFIER_NONE,
                         MouseEvent::MOUSE_STATE_WHEEL, MouseEvent::MOUSE_WHEEL_ANY),
          new Action(this, &VolumeSliceGL::eventShiftSlice))
    , mouseSetMarker_("mouseSetMarker", "Mouse Set Marker",
                      new MouseEvent(MouseEvent::MOUSE_BUTTON_LEFT, InteractionEvent::MODIFIER_NONE,
                                     MouseEvent::MOUSE_STATE_PRESS | MouseEvent::MOUSE_STATE_MOVE),
                      new Action(this, &VolumeSliceGL::eventSetMarker))
    , stepSliceUp_(
          "stepSliceUp", "Key Slice Up",
          new KeyboardEvent('W', InteractionEvent::MODIFIER_NONE, KeyboardEvent::KEY_STATE_PRESS),
          new Action(this, &VolumeSliceGL::eventStepSliceUp))
    , stepSliceDown_(
          "stepSliceDown", "Key Slice Down",
          new KeyboardEvent('S', InteractionEvent::MODIFIER_NONE, KeyboardEvent::KEY_STATE_PRESS),
          new Action(this, &VolumeSliceGL::eventStepSliceDown))
    , gestureShiftSlice_("gestureShiftSlice", "Gesture Slice Shift",
                         new GestureEvent(GestureEvent::PAN, GestureEvent::GESTURE_STATE_ANY, 3),
                         new Action(this, &VolumeSliceGL::eventGestureShiftSlice))
    , shader_(NULL)
    , indicatorShader_(NULL)
    , meshCrossHair_(NULL)
    , meshBox_(NULL)
    , meshDirty_(true)
    , inverseSliceRotation_(1.0f)
    , volumeDimensions_(8u)
    , ratioSource_(1.0f)
    , scaleMat_(1.0f) {
    addPort(inport_);
    addPort(outport_);

    inport_.onChange(this, &VolumeSliceGL::updateMaxSliceNumber);
    sliceAlongAxis_.addOption("x", "y-z plane (X axis)", CoordinateEnums::X);
    sliceAlongAxis_.addOption("y", "z-x plane (Y axis)", CoordinateEnums::Y);
    sliceAlongAxis_.addOption("z", "x-y plane (Z axis)", CoordinateEnums::Z);
    sliceAlongAxis_.addOption("p", "plane equation", 3);
    sliceAlongAxis_.set(CoordinateEnums::X);
    sliceAlongAxis_.setCurrentStateAsDefault();
    sliceAlongAxis_.onChange(this, &VolumeSliceGL::modeChange);
    addProperty(sliceAlongAxis_);

    addProperty(sliceX_);
    addProperty(sliceY_);
    addProperty(sliceZ_);
    // Invalidate selected voxel cursor when current slice changes
    sliceX_.onChange(this, &VolumeSliceGL::sliceXChange);
    sliceY_.onChange(this, &VolumeSliceGL::sliceYChange);
    sliceZ_.onChange(this, &VolumeSliceGL::sliceZChange);

    addProperty(planeNormal_);
    addProperty(planePosition_);
    
    planePosition_.onChange(this, &VolumeSliceGL::invalidateMesh);
    planeNormal_.onChange(this, &VolumeSliceGL::planeSettingsChanged);
    


    // Transformations
    rotationAroundAxis_.addOption("0", "0 deg", 0);
    rotationAroundAxis_.addOption("90", "90 deg", 1);
    rotationAroundAxis_.addOption("180", "180 deg", 2);
    rotationAroundAxis_.addOption("270", "270 deg", 3);
    rotationAroundAxis_.addOption("free", "Free rotation", 4);
    rotationAroundAxis_.set(0.f);
    rotationAroundAxis_.setCurrentStateAsDefault();
    
    volumeWrapping_.addOption("0", "Use incoming wrapping", 0);
    volumeWrapping_.addOption("1", "Clamp", GL_CLAMP);
    volumeWrapping_.addOption("2", "Clamp to invisible border", GL_CLAMP_TO_BORDER);
    volumeWrapping_.addOption("3", "Clamp to edge", GL_CLAMP_TO_EDGE);
    volumeWrapping_.addOption("4", "Mirrored repeat", GL_MIRRORED_REPEAT);
    volumeWrapping_.addOption("5", "Repeat", GL_REPEAT);
    volumeWrapping_.set(0);
    volumeWrapping_.setCurrentStateAsDefault();

    imageRotation_.setVisible(false);

    trafoGroup_.addProperty(rotationAroundAxis_);
    trafoGroup_.addProperty(imageRotation_);
    trafoGroup_.addProperty(imageScale_);
    trafoGroup_.addProperty(flipHorizontal_);
    trafoGroup_.addProperty(flipVertical_);
    trafoGroup_.addProperty(volumeWrapping_);

    rotationAroundAxis_.onChange(this ,&VolumeSliceGL::rotationModeChange);
    imageRotation_.onChange(this, &VolumeSliceGL::planeSettingsChanged);
    imageScale_.onChange(this, &VolumeSliceGL::planeSettingsChanged);    
    flipHorizontal_.onChange(this, &VolumeSliceGL::planeSettingsChanged);
    flipVertical_.onChange(this, &VolumeSliceGL::planeSettingsChanged);

    addProperty(trafoGroup_);

    // Position Selection
    pickGroup_.addProperty(posPicking_);
    pickGroup_.addProperty(showIndicator_);
    pickGroup_.addProperty(indicatorColor_);

    posPicking_.onChange(this, &VolumeSliceGL::modeChange);
    indicatorColor_.onChange(this, &VolumeSliceGL::invalidateMesh);
    showIndicator_.setReadOnly(posPicking_.get());
    indicatorColor_.setSemantics(PropertySemantics::Color);

    addProperty(pickGroup_);

    // Transfer Function
    tfGroup_.addProperty(tfMappingEnabled_);
    // Make sure that opacity does not affect the mapped color.
    if (transferFunction_.get().getNumPoints() > 0) {
        transferFunction_.get().getPoint(0)->setA(1.f);
    }
    transferFunction_.setCurrentStateAsDefault();
    tfGroup_.addProperty(transferFunction_);
    tfGroup_.addProperty(tfAlphaOffset_);
    addProperty(tfGroup_);

    worldPosition_.setReadOnly(true);
    addProperty(worldPosition_);

    addProperty(handleInteractionEvents_);

    mouseShiftSlice_.setVisible(false);
    mouseShiftSlice_.setCurrentStateAsDefault();
    addProperty(mouseShiftSlice_);

    addProperty(stepSliceUp_);
    addProperty(stepSliceDown_);
    addProperty(mouseSetMarker_);

    gestureShiftSlice_.setVisible(false);
    gestureShiftSlice_.setCurrentStateAsDefault();
    addProperty(gestureShiftSlice_);
}

VolumeSliceGL::~VolumeSliceGL() {}

void VolumeSliceGL::initialize() {
    Processor::initialize();
    shader_ = new Shader("standard.vert", "volumeslice.frag", false);
    indicatorShader_ = new Shader("standard.vert", "standard.frag", true);
    updateMaxSliceNumber();
    initializeResources();
}

void VolumeSliceGL::initializeResources() {
    if (tfMappingEnabled_.get()) {
        shader_->getFragmentShaderObject()->addShaderDefine("TF_MAPPING_ENABLED");
        transferFunction_.setVisible(true);
        tfAlphaOffset_.setVisible(true);
    } else {
        shader_->getFragmentShaderObject()->removeShaderDefine("TF_MAPPING_ENABLED");
        transferFunction_.setVisible(false);
        tfAlphaOffset_.setVisible(false);
    }
    shader_->build();
    planeSettingsChanged();
}

void VolumeSliceGL::deinitialize() {
    delete meshBox_;
    delete meshCrossHair_;
    delete shader_;
    delete indicatorShader_;
    Processor::deinitialize();
}

void VolumeSliceGL::invokeInteractionEvent(Event* event) {
    if (!handleInteractionEvents_) return;
    Processor::invokeInteractionEvent(event);
}

void VolumeSliceGL::modeChange() {
    disableInvalidation();

    sliceX_.setReadOnly(true);
    sliceY_.setReadOnly(true);
    sliceZ_.setReadOnly(true);
    planePosition_.setReadOnly(true);
    planeNormal_.setReadOnly(true);

    switch (sliceAlongAxis_.get()) {
        case CoordinateEnums::X:
            sliceX_.setReadOnly(false);
            sliceXChange();
            break;
        case CoordinateEnums::Y:
            sliceY_.setReadOnly(false);
            sliceYChange();
            break;
        case CoordinateEnums::Z:
            sliceZ_.setReadOnly(false);
            sliceZChange();
            break;
        case 3: // General plane
        default:
            planePosition_.setReadOnly(false);
            planeNormal_.setReadOnly(false);
            break;
    }

    showIndicator_.setReadOnly(!posPicking_.get());

    enableInvalidation();
    planeSettingsChanged();
}

void VolumeSliceGL::planeSettingsChanged() {
    if(!inport_.hasData()) return;

    vec3 normal = glm::normalize(planeNormal_.get());
    // Make sure we keep the aspect of the input data.
    mat4 texToWorld(inport_.getData()->getCoordinateTransformer().getTextureToWorldMatrix());

    mat4 boxrotation = 
        glm::toMat4(glm::rotation(vec3(0.0f, 0.0f, 1.0f), normal))
        * glm::rotate(imageRotation_.get(), normal);
             
    Plane plane(planePosition_.get(), glm::normalize(planeNormal_.get()));

    std::vector<IntersectionResult> points;
    points.reserve(12);

    points.push_back(plane.getSegmentIntersection(vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f)));
    points.push_back(plane.getSegmentIntersection(vec3(1.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 0.0f)));
    points.push_back(plane.getSegmentIntersection(vec3(1.0f, 1.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f)));
    points.push_back(plane.getSegmentIntersection(vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f)));
                                                                                                 
    points.push_back(plane.getSegmentIntersection(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f)));
    points.push_back(plane.getSegmentIntersection(vec3(1.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 1.0f)));
    points.push_back(plane.getSegmentIntersection(vec3(1.0f, 1.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f)));
    points.push_back(plane.getSegmentIntersection(vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 1.0f, 1.0f)));
                                                                                                 
    points.push_back(plane.getSegmentIntersection(vec3(0.0f, 0.0f, 1.0f), vec3(1.0f, 0.0f, 1.0f)));
    points.push_back(plane.getSegmentIntersection(vec3(1.0f, 0.0f, 1.0f), vec3(1.0f, 1.0f, 1.0f)));
    points.push_back(plane.getSegmentIntersection(vec3(1.0f, 1.0f, 1.0f), vec3(0.0f, 1.0f, 1.0f)));
    points.push_back(plane.getSegmentIntersection(vec3(0.0f, 1.0f, 1.0f), vec3(0.0f, 0.0f, 1.0f)));

    vec2 xrange(std::numeric_limits<float>::max(), std::numeric_limits<float>::min());
    vec2 yrange(std::numeric_limits<float>::max(), std::numeric_limits<float>::min());
    for (std::vector<IntersectionResult>::iterator it = points.begin(); it!=points.end(); ++it) {
        if (it->intersects_) {

            vec4 corner = vec4(it->intersection_,1.0f);
            corner = boxrotation*texToWorld*corner;

            xrange[0] = std::min(xrange[0], corner.x);
            xrange[1] = std::max(xrange[1], corner.x);
            yrange[0] = std::min(yrange[0], corner.y);
            yrange[1] = std::max(yrange[1], corner.y);
        }
    }

    ratioSource_ = glm::abs((xrange[1] - xrange[0]) / (yrange[1] - yrange[0]));

    // Goal: define a transformation that maps the view 2D texture coordinates into 
    // 3D texture coordinates at at some plane in the volume.

    mat4 flipMatX(-1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1);
    mat4 flipMatY(1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1);

    mat4 rotation = glm::translate(vec3(0.5f))
        * glm::toMat4(glm::rotation(vec3(0.0f, 0.0f, 1.0f), normal))
        * glm::rotate(imageRotation_.get(), vec3(0.0f, 0.0f, 1.0f))
        * glm::scale(vec3(imageScale_*std::sqrt(ratioSource_), imageScale_/ std::sqrt(ratioSource_), 1.0f))
        * glm::translate(vec3(-0.5f));

    if (flipHorizontal_) rotation *= flipMatX;
    if (flipVertical_) rotation *= flipMatY;

    // Save the inverse rotation.
    inverseSliceRotation_ = glm::inverse(rotation);

    // Set all the uniforms 
    if (shader_) {
        shader_->activate();
        shader_->setUniform("sliceRotation_", rotation);     
        shader_->deactivate();
    }

    invalidateMesh();
    return;
}

void VolumeSliceGL::process() {
    if (volumeDimensions_ != inport_.getData()->getDimensions()) {
        volumeDimensions_ = inport_.getData()->getDimensions();
        updateMaxSliceNumber();
        modeChange();
    }
    
    TextureUnit transFuncUnit, volUnit;
    utilgl::bindTexture(transferFunction_, transFuncUnit);
    utilgl::bindTexture(inport_, volUnit);

    GLint wrapS, wrapT, wrapR;
    if(volumeWrapping_.get() > 0){
        glGetTexParameteriv(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, &wrapS);
        glGetTexParameteriv(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, &wrapT);
        glGetTexParameteriv(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, &wrapR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, volumeWrapping_.get());
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, volumeWrapping_.get());
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, volumeWrapping_.get());
        if(volumeWrapping_.get() == GL_CLAMP_TO_BORDER)
            glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(vec4(0.f)));
    }
    TextureUnit::setZeroUnit();

    utilgl::activateAndClearTarget(outport_);
    shader_->activate();

    if (tfMappingEnabled_.get()) {
        shader_->setUniform("transferFunc_", transFuncUnit.getUnitNumber());
        shader_->setUniform("alphaOffset_", tfAlphaOffset_.get());
    }

    utilgl::setShaderUniforms(shader_, inport_, "volumeParameters_");
    shader_->setUniform("volume_", volUnit.getUnitNumber());
    shader_->setUniform("slice_", (inverseSliceRotation_ * vec4(planePosition_.get(),1.0f)).z);
    
    float ratioTarget = (float)outport_.getDimensions().x / (float)outport_.getDimensions().y;
    if (ratioTarget < ratioSource_) {
        scaleMat_ = glm::scale(glm::vec3(1.0f, ratioTarget / ratioSource_, 1.0f));
    } else {
        scaleMat_ = glm::scale(glm::vec3(ratioSource_ / ratioTarget, 1.0f, 1.0f));
    }
    shader_->setUniform("dataToClip_", scaleMat_);

    utilgl::singleDrawImagePlaneRect();
    shader_->deactivate();

    if (posPicking_.get() && showIndicator_.get()) renderPositionIndicator();
    
    utilgl::deactivateCurrentTarget();

    if(volumeWrapping_.get() > 0){
        volUnit.activate();
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, wrapS);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, wrapT);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, wrapR);
        TextureUnit::setZeroUnit();
    }
}

void VolumeSliceGL::renderPositionIndicator() {
    if (meshDirty_) {
        vec4 pos(static_cast<float>(sliceX_.get()), static_cast<float>(sliceY_.get()),
            static_cast<float>(sliceZ_.get()), 1.0f);

        mat4 trans = inport_.getData()->getCoordinateTransformer().getTextureToWorldMatrix();

        worldPosition_.set(vec3(trans*vec4(planePosition_.get(),1.0f)));

        updateIndicatorMesh();
    }

    MeshRenderer renderer(meshCrossHair_);
    MeshRenderer rendererBox(meshBox_);

    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float s_sizes[2];
    float width = 2.5f;
    glGetFloatv(GL_SMOOTH_LINE_WIDTH_RANGE, s_sizes);
    width = std::max(width, s_sizes[0]);
    width = std::min(width, s_sizes[1]);
    glLineWidth(width);

    indicatorShader_->activate();
    indicatorShader_->setUniform("dataToClip_", scaleMat_);

    glDepthFunc(GL_ALWAYS);
    renderer.render();
    rendererBox.render();
    glDepthFunc(GL_LESS);
    glDisable(GL_BLEND);
    glDisable(GL_LINE_SMOOTH);
    indicatorShader_->deactivate();

    glLineWidth(1.0f);
}

void VolumeSliceGL::updateIndicatorMesh() {
    vec2 pos = getScreenPosFromVolPos();

    delete meshCrossHair_;
    delete meshBox_;
    meshCrossHair_ = new Mesh;
    meshCrossHair_->setModelMatrix(mat4(1.0f));
    meshBox_ = new Mesh;
    meshBox_->setModelMatrix(mat4(1.0f));

    uvec2 canvasSize(outport_.getDimensions());
    const vec2 indicatorSize = vec2(4.0f / canvasSize.x, 4.0f / canvasSize.y);
    vec4 color(indicatorColor_.get());

    // add two vertical and two horizontal lines with a gap around the selected position
    Position2dBuffer* posBuf = new Position2dBuffer;
    Position2dBufferRAM* vertices = posBuf->getEditableRepresentation<Position2dBufferRAM>();

    // horizontal
    vertices->add(vec2(-0.5f, pos.y) * 2.0f - 1.0f);
    vertices->add(vec2(pos.x - indicatorSize.x, pos.y) * 2.0f - 1.0f);
    vertices->add(vec2(pos.x + indicatorSize.x, pos.y) * 2.0f - 1.0f);
    vertices->add(vec2(1.5f, pos.y) * 2.0f - 1.0f);
    // vertical
    vertices->add(vec2(pos.x, -0.5f) * 2.0f - 1.0f);
    vertices->add(vec2(pos.x, pos.y - indicatorSize.y) * 2.0f - 1.0f);
    vertices->add(vec2(pos.x, pos.y + indicatorSize.y) * 2.0f - 1.0f);
    vertices->add(vec2(pos.x, 1.5f) * 2.0f - 1.0f);

    ColorBuffer* colorBuf = new ColorBuffer;
    ColorBufferRAM* colors = colorBuf->getEditableRepresentation<ColorBufferRAM>();

    // indices for cross hair lines
    IndexBuffer* indexBuf = new IndexBuffer();
    IndexBufferRAM* indices = indexBuf->getEditableRepresentation<IndexBufferRAM>();
    for (unsigned int i = 0; i < 8; ++i) {
        colors->add(color);
        indices->add(i);
    }
    // clear up existing attribute buffers
    // meshCrossHair_->deinitialize();
    meshCrossHair_->addAttribute(posBuf);
    meshCrossHair_->addAttribute(colorBuf);
    meshCrossHair_->addIndicies(Mesh::AttributesInfo(GeometryEnums::LINES, GeometryEnums::NONE),
                                indexBuf);

    // mesh for center box
    posBuf = new Position2dBuffer;
    vertices = posBuf->getEditableRepresentation<Position2dBufferRAM>();
    colorBuf = new ColorBuffer;
    colors = colorBuf->getEditableRepresentation<ColorBufferRAM>();
    indexBuf = new IndexBuffer();
    indices = indexBuf->getEditableRepresentation<IndexBufferRAM>();
    // box
    vertices->add(vec2(pos.x - indicatorSize.x, pos.y - indicatorSize.y) * 2.0f - 1.0f);
    vertices->add(vec2(pos.x + indicatorSize.x, pos.y - indicatorSize.y) * 2.0f - 1.0f);
    vertices->add(vec2(pos.x + indicatorSize.x, pos.y + indicatorSize.y) * 2.0f - 1.0f);
    vertices->add(vec2(pos.x - indicatorSize.x, pos.y + indicatorSize.y) * 2.0f - 1.0f);
    for (unsigned int i = 0; i < 4; ++i) {
        colors->add(color);
        indices->add(i);
    }
    // clear up existing attribute buffers
    // meshBox_->deinitialize();
    meshBox_->addAttribute(posBuf);
    meshBox_->addAttribute(colorBuf);
    meshBox_->addIndicies(Mesh::AttributesInfo(GeometryEnums::LINES, GeometryEnums::LOOP),
                          indexBuf);

    meshDirty_ = false;
}

void VolumeSliceGL::invalidateMesh() { meshDirty_ = true; }

void VolumeSliceGL::shiftSlice(int shift) {
        vec3 newPos = planePosition_.get() + static_cast<float>(shift)/100.0f * glm::normalize(planeNormal_.get()); 
        newPos = glm::clamp(newPos,vec3(0.0f),vec3(1.0f));
        planePosition_.set(newPos);
}

void VolumeSliceGL::setVolPosFromScreenPos(vec2 pos) {
    if (!posPicking_.get()) return;  // position mode not enabled

    pos = vec2(glm::translate(vec3(0.5f, 0.5f, 0.0f))
        * glm::inverse(scaleMat_) 
        * glm::translate(vec3(-0.5f, -0.5f, 0.0f)) 
        * vec4(pos, 0.0f, 1.0f));

    if ((pos.x < 0.0f) || (pos.x > 1.0f) || (pos.y < 0.0f) || (pos.y > 1.0f)) {
        // invalid position
        return;
    }

    vec4 newpos(inverseSliceRotation_ * vec4(planePosition_.get(),1.0f));
    newpos.x = pos.x;
    newpos.y = pos.y;
    newpos = glm::inverse(inverseSliceRotation_) * newpos;

    invalidateMesh();
    disableInvalidation();
    planePosition_.set(vec3(newpos));
    enableInvalidation();
}

vec2 VolumeSliceGL::getScreenPosFromVolPos() {
    vec2 pos(inverseSliceRotation_ * vec4(planePosition_.get(),1.0f));
    return pos;
}

void VolumeSliceGL::updateMaxSliceNumber() {
    if (!inport_.hasData()) {
        return;
    }
    disableInvalidation();
    uvec3 dims = inport_.getData()->getDimensions();
    if (dims.x != sliceX_.getMaxValue()) {
        sliceX_.setMaxValue(static_cast<int>(dims.x));
        sliceX_.set(static_cast<int>(dims.x) / 2);
    }
    if (dims.y != sliceY_.getMaxValue()) {
        sliceY_.setMaxValue(static_cast<int>(dims.y));
        sliceY_.set(static_cast<int>(dims.y) / 2);
    }
    if (dims.z != sliceZ_.getMaxValue()) {
        sliceZ_.setMaxValue(static_cast<int>(dims.z));
        sliceZ_.set(static_cast<int>(dims.z) / 2);
    }
    enableInvalidation();
}

void VolumeSliceGL::eventShiftSlice(Event* event){
    MouseEvent* mouseEvent = static_cast<MouseEvent*>(event);
    int steps = mouseEvent->wheelSteps();
    shiftSlice(steps);
}

void VolumeSliceGL::eventSetMarker(Event* event){
    MouseEvent* mouseEvent = static_cast<MouseEvent*>(event);
    vec2 mousePos(mouseEvent->posNormalized());
    setVolPosFromScreenPos(vec2(mousePos.x, 1.0f - mousePos.y));
}

void VolumeSliceGL::eventStepSliceUp(Event*){
    shiftSlice(1);
}

void VolumeSliceGL::eventStepSliceDown(Event*){
    shiftSlice(-1);
}

void VolumeSliceGL::eventGestureShiftSlice(Event* event){
    GestureEvent* gestureEvent = static_cast<GestureEvent*>(event);
    if (gestureEvent->deltaPos().y < 0)
        shiftSlice(1);
    else if (gestureEvent->deltaPos().y > 0)
        shiftSlice(-1);
}

void VolumeSliceGL::sliceXChange() {
    planeNormal_.set(vec3(-1.0f, 0.0f, 0.0f));
    planePosition_.set(
        vec3(static_cast<float>(sliceX_.get() - 1) / static_cast<float>(sliceX_.getMaxValue() - 1),
        planePosition_.get().y, planePosition_.get().z));
}

void VolumeSliceGL::sliceYChange() {
    planeNormal_.set(vec3(0.0f, -1.0f, 0.0f));
    planePosition_.set(
        vec3(planePosition_.get().x,
        static_cast<float>(sliceY_.get() - 1) / static_cast<float>(sliceY_.getMaxValue() - 1),
        planePosition_.get().z));
}

void VolumeSliceGL::sliceZChange() {
    planeNormal_.set(vec3(0.0f, 0.0f, -1.0f));
    planePosition_.set(vec3(
        planePosition_.get().x, planePosition_.get().y,
        static_cast<float>(sliceZ_.get() - 1) / static_cast<float>(sliceZ_.getMaxValue() - 1)));
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

}  // inviwo namespace
