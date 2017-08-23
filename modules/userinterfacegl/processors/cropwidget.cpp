/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2017 Inviwo Foundation
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

#include <modules/userinterfacegl/processors/cropwidget.h>

#include <inviwo/core/datastructures/geometry/basicmesh.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/util/volumeutils.h>
#include <inviwo/core/util/colorconversion.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/pickingevent.h>
#include <inviwo/core/interaction/events/pickingevent.h>
#include <inviwo/core/network/networklock.h>
#include <modules/opengl/geometry/meshgl.h>
#include <modules/opengl/buffer/buffergl.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/openglutils.h>
#include <modules/assimp/assimpreader.h>

#include <glm/gtc/epsilon.hpp>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo CropWidget::processorInfo_{
    "org.inviwo.CropWidget",  // Class identifier
    "Crop Widget",            // Display name
    "UI",                     // Category
    CodeState::Stable,        // Code state
    "GL, UI, Clipping",       // Tags
};
const ProcessorInfo CropWidget::getProcessorInfo() const { return processorInfo_; }

CropWidget::CropWidget()
    : Processor()
    , inport_("inport")
    , volume_("volume")
    , outport_("outport")
    , meshOut_("mesh")

    , uiSettings_("uiSettings", "UI Settings")
    , showWidget_("showWidget", "Show Widget", true)
    , showCropPlane_("showClipPlane", "Crop Plane Visible", true)
    , handleColor_("handleColor", "Handle Color", vec4(0.8f, 0.4f, 0.1f, 1.0f), vec4(0.0f),
                   vec4(1.0f))
    , cropLineColor_("cropLineColor", "Crop Line Color", vec4(vec3(0.8f), 1.0f), vec4(0.0f),
                     vec4(1.0f))
    , lineWidth_("lineWidth", "Line Width (pixel)", 2.5f, 0.0f, 50.0f, 0.1f)
    , offset_("offset", "Offset", 0.0f, -1.0f, 1.0f)
    , scale_("scale", "Scale", 0.15f, 0.001f, 2.0f, 0.05f)

    , axis_("axis", "Active Axis",
            {
                {"x", "X", CartesianCoordinateAxis::X},
                {"y", "Y", CartesianCoordinateAxis::Y},
                {"z", "Z", CartesianCoordinateAxis::Z},
            })
    , clipRanges_({{{"cropX", "Crop X", 0, 256, 0, 256, 1, 1},
                    {"cropY", "Crop Y", 0, 256, 0, 256, 1, 1},
                    {"cropZ", "Crop Z", 0, 256, 0, 256, 1, 1}}})
    , camera_("camera", "Camera")

    , lightingProperty_("internalLighting", "Lighting", &camera_)
    , trackball_(&camera_)
    , picking_(this, numInteractionWidgets, [&](PickingEvent *p) { objectPicked(p); })
    , shader_("meshrenderer.vert", "meshrenderer.frag", false)
    , lineShader_("linerenderer.vert", "linerenderer.geom", "linerenderer.frag", false)
    , isMouseBeingPressedAndHold_(false)
    , lastState_(-1)
    , volumeBasis_(1.0f)
    , volumeOffset_(-0.5f) {

    addPort(volume_);
    addPort(inport_);
    addPort(outport_);
    addPort(meshOut_);

    inport_.setOptional(true);

    addProperty(axis_);
    for (auto &elem : clipRanges_) {
        addProperty(elem);
    }

    handleColor_.setSemantics(PropertySemantics::Color);
    cropLineColor_.setSemantics(PropertySemantics::Color);

    // brighten up ambient color
    lightingProperty_.ambientColor_.set(vec3(0.6f));

    uiSettings_.setCollapsed(true);
    uiSettings_.addProperty(showWidget_);
    uiSettings_.addProperty(handleColor_);
    uiSettings_.addProperty(offset_);
    uiSettings_.addProperty(scale_);
    uiSettings_.addProperty(showCropPlane_);
    uiSettings_.addProperty(cropLineColor_);
    uiSettings_.addProperty(lineWidth_);
    uiSettings_.addProperty(lightingProperty_);
    addProperty(uiSettings_);

    camera_.setCollapsed(true);

    addProperty(camera_);
    addProperty(trackball_);

    setAllPropertiesCurrentStateAsDefault();

    // Since the clips depend on the input volume dimensions, we make sure to always
    // serialize them so we can do a proper renormalization when we load new data.
    for (auto &elem : clipRanges_) {
        elem.setSerializationMode(PropertySerializationMode::All);
    }

    volume_.onChange([this]() { updateAxis(); });
    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
    lineShader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });

    std::array<InteractionElement, 3> elem = {
        InteractionElement::LowerBound, InteractionElement::UpperBound, InteractionElement::Middle};
    for (int i = 0; i < numInteractionWidgets; ++i) {
        pickingIDs_[i] = {picking_.getPickingId(i), elem[i]};
    }
}

CropWidget::~CropWidget() = default;

void CropWidget::process() {
    if (!interactionHandleMesh_) {
        initMesh();
    }
    if (volume_.isChanged()) {
        updateBoundingCube();
    }
    if (axis_.isModified()) {
        updateAxis();
    }

    if (inport_.isConnected()) {
        utilgl::activateTargetAndCopySource(outport_, inport_);
    } else {
        utilgl::activateAndClearTarget(outport_, ImageType::ColorDepthPicking);
    }

    if (showWidget_.get()) {
        utilgl::GlBoolState depthTest(GL_DEPTH_TEST, true);
        utilgl::BlendModeState blending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        shader_.activate();

        utilgl::setShaderUniforms(shader_, camera_, "camera_");
        utilgl::setShaderUniforms(shader_, lightingProperty_, "light_");

        auto axisInfo = getAxis();
        currentAxisInfo_ = axisInfo;
        auto &property = clipRanges_[static_cast<int>(axis_.get())];
        float range = static_cast<float>(property.getRangeMax() - property.getRangeMin());
        float lowerBound = (property.get().x - property.getRangeMin()) / range;
        float upperBound = (property.get().y - property.getRangeMin()) / range;

        // draw the interaction handles
        {
            shader_.setUniform("overrideColor_", handleColor_.get());

            // apply custom transformation
            const mat4 m = glm::scale(vec3(scale_.get()));

            // lower bound
            mat4 worldMatrix(glm::translate(axisInfo.pos + axisInfo.axis * lowerBound) * m *
                             axisInfo.rotMatrix);
            mat3 normalMatrix(glm::inverseTranspose(worldMatrix));
            shader_.setUniform("geometry_.dataToWorld", worldMatrix);
            shader_.setUniform("geometry_.dataToWorldNormalMatrix", normalMatrix);
            shader_.setUniform("pickColor_", picking_.getColor(0));
            interactionHandleDrawer_->draw();

            // upper bound
            worldMatrix =
                glm::translate(axisInfo.pos + axisInfo.axis * upperBound) * m * axisInfo.flipMatrix;
            normalMatrix = mat3(glm::inverseTranspose(worldMatrix));
            shader_.setUniform("geometry_.dataToWorld", worldMatrix);
            shader_.setUniform("geometry_.dataToWorldNormalMatrix", normalMatrix);
            shader_.setUniform("pickColor_", picking_.getColor(1));
            interactionHandleDrawer_->draw();
        }
        shader_.setUniform("pickColor_", vec3(-1.0f));

        {
            if (showCropPlane_.get()) {
                bool drawLowerPlane = (property.get().x != property.getRangeMin());
                bool drawUpperPlane = (property.get().y != property.getRangeMax());

                if (drawLowerPlane || drawUpperPlane) {
                    if (cropLineColor_.isModified()) {
                        createLineStripMesh();
                    }

                    Shader &shader = lineShader_;
                    shader.activate();

                    MeshDrawerGL::DrawObject drawStrip(linestrip_->getRepresentation<MeshGL>(),
                                                       linestrip_->getDefaultMeshInfo());

                    utilgl::DepthFuncState depthFunc(GL_LEQUAL);

                    shader.setUniform("screenDim", vec2(outport_.getDimensions()));
                    utilgl::setUniforms(shader, camera_, lineWidth_);

                    // rotate clip plane from [0, 0, -1] to match the currently selected clip axis
                    mat4 scale(volumeBasis_);
                    mat4 rotMatrix(1.0f);
                    if (axis_.get() != CartesianCoordinateAxis::Z) {
                        vec3 v1(0.0f, 0.0f, -1.0f);
                        vec3 v2(glm::normalize(axisInfo.axis));
                        rotMatrix = glm::rotate(glm::half_pi<float>(), glm::cross(v1, v2));
                    }
                    rotMatrix = scale * rotMatrix;

                    if (drawLowerPlane) {
                        // draw lower clip plane
                        mat4 worldMatrix(
                            glm::translate(volumeOffset_ + axisInfo.axis * lowerBound) * rotMatrix);
                        mat3 normalMatrix(glm::inverseTranspose(worldMatrix));
                        shader.setUniform("geometry.dataToWorld", worldMatrix);
                        shader.setUniform("geometry.dataToWorldNormalMatrix", normalMatrix);
                        LGL_ERROR;
                        drawStrip.draw();
                        LGL_ERROR;
                    }

                    if (drawUpperPlane) {
                        // draw upper clip plane
                        mat4 worldMatrix(
                            glm::translate(volumeOffset_ + axisInfo.axis * upperBound) * rotMatrix);
                        linestrip_->setModelMatrix(worldMatrix);
                        mat3 normalMatrix(glm::inverseTranspose(worldMatrix));
                        shader.setUniform("geometry.dataToWorld", worldMatrix);
                        shader.setUniform("geometry.dataToWorldNormalMatrix", normalMatrix);
                        LGL_ERROR;
                        drawStrip.draw();
                        LGL_ERROR;
                    }
                    LGL_ERROR;
                }
            }
        }
    }
    utilgl::deactivateCurrentTarget();

    meshOut_.setData(linestrip_);
}

void CropWidget::initializeResources() {
    // shading defines
    utilgl::addShaderDefines(shader_, lightingProperty_);
    shader_.build();

    // initialize shader uniform containing all picking colors, the mesh will fetch the appropriate
    // one using the object ID stored in the second texture coordinate set
    shader_.activate();
    std::vector<vec3> pickcolors;
    pickcolors.resize(3);

    // fill vector with picking colors
    for (int i = 0; i < 3; ++i) {
        pickcolors[i] = picking_.getColor(i);
    }
    shader_.setUniform("pickColors", pickcolors);
    // set up default colors for the mesh (horizontal: red, vertical: green, center: light gray,
    // roll: light blue, zoom: light gray)
    std::array<vec4, 4> color = {vec4(1.0f, 0.7f, 0.7f, 1.0f), vec4(0.7f, 1.0f, 0.7f, 1.0f),
                                 vec4(vec3(0.76f), 1.0f), vec4(0.7f, 0.9f, 1.0f, 1.0f)};
    shader_.setUniform("meshColors_", color);
    shader_.deactivate();

    lineShader_.getGeometryShaderObject()->addShaderDefine("ENABLE_ADJACENCY", "1");
    lineShader_.getFragmentShaderObject()->addShaderDefine("ENABLE_ROUND_DEPTH_PROFILE");
    lineShader_.build();

    lineShader_.activate();
    lineShader_.setUniform("antialiasing", 1.0f);
    lineShader_.setUniform("miterLimit", 1.0f);
    lineShader_.deactivate();
}

void CropWidget::initMesh() {
    auto module = InviwoApplication::getPtr()->getModuleByIdentifier("UserInterfaceGL");
    if (!module) {
        throw Exception("Could not locate module 'UserInterfaceGL'");
    }

    std::string basePath(module->getPath(ModulePath::Data));
    basePath += "/meshes/";
    std::string meshFilename = basePath + "arrow-single.fbx";

    AssimpReader meshReader;
    meshReader.setLogLevel(AssimpLogLevel::Error);
    meshReader.setFixInvalidDataFlag(false);

    // interaction handles
    interactionHandleMesh_ = meshReader.readData(meshFilename);
    interactionHandleDrawer_ = std::make_shared<MeshDrawerGL>(interactionHandleMesh_.get());

    createLineStripMesh();
}

void CropWidget::createLineStripMesh() {
    auto linestrip = std::make_shared<Mesh>(DrawType::Lines, ConnectivityType::StripAdjacency);
    auto vertices = std::make_shared<Buffer<vec3> >();
    auto colors = std::make_shared<Buffer<vec4> >();
    auto texCoords = std::make_shared<Buffer<vec2> >();

    auto vBuffer = vertices->getEditableRAMRepresentation();
    auto colorBuffer = colors->getEditableRAMRepresentation();
    auto texBuffer = texCoords->getEditableRAMRepresentation();

    vec3 p(0.0f);
    vec2 t(0.0f, 0.0f);
    vec4 color;
    vec3 mask[5] = {{0.0f, 0.0f, 0.0f},
                    {1.0f, 0.0, 0.0f},
                    {1.0f, 1.0f, 0.0f},
                    {0.0f, 1.0f, 0.0f},
                    {0.0f, 0.0f, 0.0f}};
    for (int i = 0; i < 5; ++i) {
        vBuffer->add(p + mask[i]);
        colorBuffer->add(cropLineColor_.get());
        texBuffer->add(t + vec2(i / 4.0f, 0.0f));
    }

    auto indices = std::make_shared<IndexBuffer>();
    auto indexBuffer = indices->getEditableRAMRepresentation();
    indexBuffer->add({3, 0, 1, 2, 3, 4, 1});

    linestrip->addBuffer(BufferType::PositionAttrib, vertices);
    linestrip->addBuffer(BufferType::ColorAttrib, colors);
    linestrip->addBuffer(BufferType::TexcoordAttrib, texCoords);
    linestrip->addIndicies(Mesh::MeshInfo(DrawType::Lines, ConnectivityType::StripAdjacency),
                           indices);

    linestrip_ = linestrip;
}

void CropWidget::updateAxis() {
    if (!volume_.hasData()) return;

    auto dims = util::getVolumeDimensions(volume_.getData());

    if (dims != size3_t(clipRanges_[0].getRangeMax() + 1, clipRanges_[1].getRangeMax() + 1,
                        clipRanges_[2].getRangeMax() + 1)) {
        NetworkLock lock(this);

        for (int i = 0; i < 3; ++i) {
            clipRanges_[i].setRange(ivec2(0, dims[i] - 1));

            // set the new dimensions to default if we were to press reset
            clipRanges_[i].setCurrentStateAsDefault();
        }
    }
}

void CropWidget::updateBoundingCube() {
    if (auto volume = volume_.getData()) {
        volumeBasis_ = volume->getBasis();
        volumeOffset_ = volume->getOffset();
    } else {
        volumeBasis_ = mat3(1.0f);
        volumeOffset_ = vec3(-0.5f);
    }
}

void CropWidget::objectPicked(PickingEvent *p) {
    if (auto me = p->getEventAs<MouseEvent>()) {
        if (me->buttonState() & MouseButton::Left) {
            if (me->state() == MouseState::Press) {
                isMouseBeingPressedAndHold_ = true;
                lastState_ = clipRanges_[static_cast<int>(axis_.get())].get();
            } else if (me->state() == MouseState::Release) {
                isMouseBeingPressedAndHold_ = false;
                lastState_ = ivec2(-1);
            } else if (me->state() == MouseState::Move) {
                switch (p->getPickedId()) {
                    case 0:
                        rangePositionHandlePicked(p, false);
                        break;
                    case 1:
                        rangePositionHandlePicked(p, true);
                        break;
                    case 2:
                        break;
                    default:
                        LogWarn("invalid picking ID");
                        break;
                }
            }
            me->markAsUsed();
        }
    }
}

CropWidget::AnnotationInfo CropWidget::getAxis() {
    auto &cam = camera_.get();
    std::array<vec2, 4> axisSelector = {{{0, 0}, {1, 0}, {1, 1}, {0, 1}}};

    auto viewMatrix(cam.getViewMatrix());
    auto viewprojMatrix = cam.getProjectionMatrix() * cam.getViewMatrix();

    std::array<int, 3> indices;  // encodes the index of the primary axis and the other two axes

    mat4 rotMatrix(1.0f);
    mat4 flipOrientationMat;  // matrix used for the second arrow facing the opposite direction
                              // tilt the arrow mesh so that it is rotated by 45 degree
    switch (axis_.get()) {
        case CartesianCoordinateAxis::X:
            indices = {{0, 1, 2}};
            rotMatrix = glm::rotate(-glm::half_pi<float>(), vec3(0.0f, 1.0f, 0.0f)) *
                        glm::rotate(-glm::quarter_pi<float>(), vec3(0.0f, 0.0f, 1.0f));
            flipOrientationMat = glm::rotate(glm::pi<float>(), vec3(0.0f, 1.0f, -1.0f)) * rotMatrix;
            break;
        case CartesianCoordinateAxis::Y:
            indices = {{1, 0, 2}};
            rotMatrix = glm::rotate(glm::half_pi<float>(), vec3(1.0f, 0.0f, 0.0f)) *
                        glm::rotate(-glm::quarter_pi<float>(), vec3(0.0f, 0.0f, 1.0f));
            flipOrientationMat = glm::rotate(glm::pi<float>(), vec3(1.0f, 0.0f, -1.0f)) * rotMatrix;
            break;
        case CartesianCoordinateAxis::Z:
        default:
            indices = {{2, 0, 1}};
            rotMatrix = glm::rotate(glm::pi<float>(), vec3(0.0f, 1.0f, 0.0f)) *
                        glm::rotate(glm::quarter_pi<float>(), vec3(0.0f, 0.0f, 1.0f));
            flipOrientationMat = glm::rotate(glm::pi<float>(), vec3(1.0f, 1.0f, 0.0f)) * rotMatrix;
            break;
    }

    vec3 axis(volumeBasis_[indices[0]]);

    vec3 volumeCenter =
        volumeOffset_ + (volumeBasis_[0] + volumeBasis_[1] + volumeBasis_[2]) * 0.5f;
    vec4 projCenter(viewprojMatrix * vec4(volumeCenter, 1.0f));
    projCenter /= projCenter.w;

    std::array<vec3, 4> points;      // base points of the four edge candidates
    std::array<vec4, 4> viewPoints;  // edge mid points in device coords
    std::array<vec4, 4> projPoints;  // edge mid points in device coords

    std::array<vec4, 8> projPoints2;  // edge mid points in device coords

    for (int i = 0; i < 4; ++i) {
        points[i] = volumeOffset_ + volumeBasis_[indices[1]] * axisSelector[i].x +
                    volumeBasis_[indices[2]] * axisSelector[i].y;
        // compute the projection of the mid point of the edge
        auto p = vec4(points[i] + axis * 0.5f, 1.0);

        viewPoints[i] = viewMatrix * p;
        viewPoints[i] /= viewPoints[i].w;

        projPoints[i] = viewprojMatrix * p;
        projPoints[i] /= projPoints[i].w;

        p = vec4(points[i], 1.0);
        projPoints2[i] = viewprojMatrix * p;
        projPoints2[i] /= projPoints2[i].w;
        p = vec4(points[i] + axis, 1.0);
        projPoints2[i + 4] = viewprojMatrix * p;
        projPoints2[i + 4] /= projPoints2[i + 4].w;
    }

    // sort edges based on depth
    std::vector<int> index{0, 1, 2, 3};
    std::sort(index.begin(), index.end(),
              [&](int a, int b) { return viewPoints[a].z > viewPoints[b].z; });

    auto isLeftOf = [&](int a, int b) {
        vec2 v = vec2(projPoints[b]) - vec2(projCenter);
        if (glm::dot(vec2(projPoints[a]) - vec2(projPoints[b]), v) < 0.0f) {
            // edge not occluded
            float d = glm::dot(vec2(projPoints[a]) - vec2(projPoints[b]), vec2(-1.0f, -1.0f));
            return d > 0.0f;
        } else {
            return false;  // edge is occluded, not relevant
        }
    };

    const float epsilon = 0.05f;

    bool sameDepth = glm::epsilonEqual(viewPoints[index[0]].z, viewPoints[index[1]].z, epsilon) &&
                     glm::epsilonEqual(viewPoints[index[0]].z, viewPoints[index[2]].z, epsilon);

    int selectedIndex = -1;
    if (sameDepth) {
        // camera is aligned orthogonally to clip axis, pick the left most edge
        auto leftMost = [&](int a, int b) {
            return glm::dot(vec2(projPoints[a]) - vec2(projPoints[b]), vec2(-1.0f, -1.0f)) > 0.0f;
        };

        std::vector<int> indexLeft{0, 1, 2, 3};
        std::sort(indexLeft.begin(), indexLeft.end(), leftMost);

        selectedIndex = indexLeft[0];
    } else {
        vec3 v1, v2;
        if (index[2] == ((index[0] + 1) % 4)) {
            v1 = vec3(projPoints2[index[2]]) - vec3(projPoints2[index[0]]);
            v2 = vec3(projPoints2[index[0] + 4]) - vec3(projPoints2[index[0]]);
        } else {
            v1 = vec3(projPoints2[index[0]]) - vec3(projPoints2[index[2]]);
            v2 = vec3(projPoints2[index[2] + 4]) - vec3(projPoints2[index[2]]);
        }
        vec3 normal = glm::cross(v1, v2);
        bool occluded = normal.z < 0.0f;
        if (!occluded) {
            // third edge not occluded
            selectedIndex = isLeftOf(index[1], index[2]) ? index[1] : index[2];
        } else {
            // choose between first and second edge
            selectedIndex = isLeftOf(index[0], index[1]) ? index[0] : index[1];
        }
    }

    vec3 annotationBase(points[selectedIndex]);

    // adjust rotation matrix for odd axes (rotation along the axis by 90 degree), so that the mesh
    // is always touching the axis on the side
    if ((selectedIndex % 2) == 1) {
        rotMatrix = glm::rotate(glm::half_pi<float>(), axis) * rotMatrix;
        flipOrientationMat = glm::rotate(glm::half_pi<float>(), axis) * flipOrientationMat;
    }

    // offset direction depends on which of the four orthogonal axes was chosen
    vec3 offsetDir((annotationBase + axis * 0.5f) - volumeCenter);
    offsetDir = glm::normalize(offsetDir) * offset_.get();
    annotationBase += offsetDir;

    return {annotationBase,
            axis,
            rotMatrix,
            flipOrientationMat,
            vec3(projPoints2[selectedIndex]),
            vec3(projPoints2[selectedIndex + 4])};
}

void CropWidget::rangePositionHandlePicked(PickingEvent *p, bool upperLimit) {
    auto currNDC = p->getNDC();
    auto prevNDC = p->getPressedNDC();

    // Use depth of initial press as reference to move in the image plane.
    auto refDepth = p->getPressedDepth();
    currNDC.z = refDepth;
    prevNDC.z = refDepth;

    auto corrWorld = camera_.getWorldPosFromNormalizedDeviceCoords(static_cast<vec3>(currNDC));
    auto prevWorld = camera_.getWorldPosFromNormalizedDeviceCoords(static_cast<vec3>(prevNDC));

    vec3 axis(volumeBasis_[static_cast<int>(axis_.get())]);

    auto viewprojMatrix = camera_.get().getProjectionMatrix() * camera_.get().getViewMatrix();

    // project mouse delta onto axis
    vec2 delta(currNDC - prevNDC);
    vec2 axis2D(currentAxisInfo_.endNDC - currentAxisInfo_.startNDC);
    float dist = glm::dot(delta, glm::normalize(axis2D));
    // LogInfo("dist: " << dist);

    auto &property = clipRanges_[static_cast<int>(axis_.get())];

    auto value = property.get();
    auto range = property.getRange();
    bool modified = false;
    if (upperLimit) {
        int v = lastState_.y + static_cast<int>(dist * (range.y - range.x));
        v = std::max(v, property.getMinSeparation() + lastState_.x);
        modified = (value.y != v);
        value.y = v;
    } else {
        int v = lastState_.x + static_cast<int>(dist * (range.y - range.x));
        v = std::min(v, lastState_.y - property.getMinSeparation());
        modified = (value.x != v);
        value.x = v;
    }
    if (modified) {
        property.set(value);
    }
}

}  // namespace inviwo
