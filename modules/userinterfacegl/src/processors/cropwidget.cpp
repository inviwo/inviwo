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

#include <modules/userinterfacegl/processors/cropwidget.h>

#include <inviwo/core/datastructures/geometry/basicmesh.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/util/volumeutils.h>
#include <inviwo/core/util/colorconversion.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/touchevent.h>
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

    , cropAxes_({{{CartesianCoordinateAxis::X,
                   {"cropAxisX", "Crop X"},
                   {"cropAxisXEnabled", "Enabled", true},
                   {"cropX", "Range", 0, 256, 0, 256, 1, 1},
                   {"cropXOut", "Crop X", 0, 256, 0, 256, 1, 1},
                   AnnotationInfo()},
                  {CartesianCoordinateAxis::Y,
                   {"cropAxisY", "Crop Y"},
                   {"cropAxisYEnabled", "Enabled", true},
                   {"cropY", "Range", 0, 256, 0, 256, 1, 1},
                   {"cropYOut", "Crop Y", 0, 256, 0, 256, 1, 1},
                   AnnotationInfo()},
                  {CartesianCoordinateAxis::Z,
                   {"cropAxisZ", "Crop Z"},
                   {"cropAxisZEnabled", "Enabled", true},
                   {"cropZ", "Range", 0, 256, 0, 256, 1, 1},
                   {"cropZOut", "Crop Z", 0, 256, 0, 256, 1, 1},
                   AnnotationInfo()}}})
    , relativeRangeAdjustment_("relativeRangeAdjustment", "Rel. Adjustment on Range Change", true)
    , outputProps_("outputProperties", "Output")
    , camera_("camera", "Camera")

    , lightingProperty_("internalLighting", "Lighting", &camera_)
    , trackball_(&camera_)
    , picking_(this, 3 * numInteractionWidgets, [&](PickingEvent *p) { objectPicked(p); })
    , shader_("geometrycustompicking.vert", "geometryrendering.frag", false)
    , lineShader_("linerenderer.vert", "linerenderer.geom", "linerenderer.frag", false)
    , isMouseBeingPressedAndHold_(false)
    , lastState_(-1)
    , volumeBasis_(1.0f)
    , volumeOffset_(-0.5f) {

    addPort(volume_);
    addPort(inport_);
    addPort(outport_);

    inport_.setOptional(true);

    for (auto &elem : cropAxes_) {
        // Since the clips depend on the input volume dimensions, we make sure to always
        // serialize them so we can do a proper renormalization when we load new data.
        elem.range.setSerializationMode(PropertySerializationMode::All);

        elem.composite.addProperty(elem.enabled);
        elem.composite.addProperty(elem.range);
        elem.composite.setCollapsed(true);
        addProperty(elem.composite);

        auto updateRange = [&]() {
            // sync ranges including extrema
            const auto rangeExtrema = elem.range.getRange();
            if (elem.enabled.get()) {
                // sync the cropped range
                elem.outputRange.set(ivec2(elem.range.getStart(), elem.range.getEnd()),
                                     rangeExtrema, 1, 1);
            } else {
                // don't sync the crop range, use the full range instead
                elem.outputRange.set(rangeExtrema, rangeExtrema, 1, 1);
            }
        };
        // update status of output ranges
        elem.range.onChange(updateRange);
        elem.range.setReadOnly(!elem.enabled.get());

        elem.enabled.onChange([&]() {
            // range should not be editable if axis is not enabled
            elem.range.setReadOnly(!elem.enabled.get());
            // sync ranges
            if (elem.enabled.get()) {
                // sync the cropped range
                elem.outputRange.set(ivec2(elem.range.getStart(), elem.range.getEnd()));
            } else {
                // don't copy the crop range, use the full range instead
                elem.outputRange.set(elem.range.getRange());
            }
        });

        // set up output crop range properties
        outputProps_.addProperty(elem.outputRange);
        elem.outputRange.setReadOnly(true);
        elem.outputRange.setSemantics(PropertySemantics::Text);
    }
    outputProps_.setCollapsed(true);
    addProperty(outputProps_);
    addProperty(relativeRangeAdjustment_);

    handleColor_.setSemantics(PropertySemantics::Color);
    cropLineColor_.setSemantics(PropertySemantics::Color);

    // brighten up ambient color
    lightingProperty_.ambientColor_.set(vec3(0.6f));
    lightingProperty_.setCollapsed(true);

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

    volume_.onChange([this]() { updateAxisRanges(); });
    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
    lineShader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });

    std::array<InteractionElement, 3> elem = {
        InteractionElement::LowerBound, InteractionElement::UpperBound, InteractionElement::Middle};
    for (int i = 0; i < static_cast<int>(pickingIDs_.size()); ++i) {
        pickingIDs_[i] = {picking_.getPickingId(i), elem[i % numInteractionWidgets]};
    }
}

CropWidget::~CropWidget() = default;

void CropWidget::process() {
    if (!interactionHandleMesh_[0]) {
        initMesh();
    }
    if (volume_.isChanged()) {
        updateBoundingCube();
    }

    if (inport_.isReady()) {
        utilgl::activateTargetAndCopySource(outport_, inport_);
    } else {
        utilgl::activateAndClearTarget(outport_, ImageType::ColorDepthPicking);
    }

    if (showWidget_ || showCropPlane_) {
        utilgl::GlBoolState depthTest(GL_DEPTH_TEST, true);
        utilgl::BlendModeState blending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        shader_.activate();

        utilgl::setShaderUniforms(shader_, camera_, "camera");
        utilgl::setShaderUniforms(shader_, lightingProperty_, "lighting");
        shader_.setUniform("overrideColor", handleColor_.get());

        for (auto &elem : cropAxes_) {
            if (elem.enabled.get()) {
                // update axis information
                elem.info = getAxis(elem.axis);

                renderAxis(elem);
            }
        }
    }
    utilgl::deactivateCurrentTarget();
}

void CropWidget::initializeResources() {
    // shading defines
    utilgl::addShaderDefines(shader_, lightingProperty_);
    shader_.build();

    lineShader_[ShaderType::Geometry]->addShaderDefine("ENABLE_ADJACENCY", "1");
    lineShader_[ShaderType::Fragment]->addShaderDefine("ENABLE_ROUND_DEPTH_PROFILE");

    // See createLineStripMesh()
    lineShader_[ShaderType::Vertex]->addInDeclaration("in_" + toString(BufferType::PositionAttrib),
                                                      static_cast<int>(BufferType::PositionAttrib),
                                                      "vec3");
    lineShader_[ShaderType::Vertex]->addInDeclaration("in_" + toString(BufferType::ColorAttrib),
                                                      static_cast<int>(BufferType::ColorAttrib),
                                                      "vec4");
    lineShader_[ShaderType::Vertex]->addInDeclaration("in_" + toString(BufferType::TexcoordAttrib),
                                                      static_cast<int>(BufferType::TexcoordAttrib),
                                                      "vec2");
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

    AssimpReader meshReader;
    meshReader.setLogLevel(AssimpLogLevel::Error);
    meshReader.setFixInvalidDataFlag(false);

    // interaction handles
    interactionHandleMesh_[0] = meshReader.readData(basePath + "arrow-single.fbx");
    interactionHandleMesh_[1] = meshReader.readData(basePath + "crop-handle.fbx");

    createLineStripMesh();
}

void CropWidget::createLineStripMesh() {
    auto linestrip = std::make_shared<Mesh>(DrawType::Lines, ConnectivityType::StripAdjacency);
    auto vertices = std::make_shared<Buffer<vec3>>();
    auto colors = std::make_shared<Buffer<vec4>>();
    auto texCoords = std::make_shared<Buffer<vec2>>();

    auto vBuffer = vertices->getEditableRAMRepresentation();
    auto colorBuffer = colors->getEditableRAMRepresentation();
    auto texBuffer = texCoords->getEditableRAMRepresentation();

    vec3 p(0.0f);
    vec2 t(0.0f, 0.0f);
    vec3 mask[5] = {{0.0f, 0.0f, 0.0f},
                    {1.0f, 0.0f, 0.0f},
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
    linestrip->addIndices(Mesh::MeshInfo(DrawType::Lines, ConnectivityType::StripAdjacency),
                          indices);

    linestrip_ = linestrip;
}

void CropWidget::renderAxis(const CropAxis &axis) {
    // if min separation of the range is smaller, the middle handle is not drawn
    const float minSeparationPercentage = 0.05f;

    auto &property = axis.range;

    float range = static_cast<float>(property.getRangeMax() - property.getRangeMin());
    float lowerBound = (property.get().x - property.getRangeMin()) / range;
    float upperBound = (property.get().y - property.getRangeMin()) / range;

    // draw the interaction handles
    if (showWidget_) {
        const int axisIDOffset = static_cast<int>(axis.axis) * numInteractionWidgets;

        shader_.activate();

        // apply custom transformation
        const mat4 m = glm::scale(vec3(scale_.get()));

        auto draw = [&](auto &drawObject, unsigned int pickID, float value, const mat4 &rot) {
            mat4 worldMatrix(glm::translate(axis.info.pos + axis.info.axis * value) * m * rot);
            mat3 normalMatrix(glm::inverseTranspose(worldMatrix));
            shader_.setUniform("geometry.dataToWorld", worldMatrix);
            shader_.setUniform("geometry.dataToWorldNormalMatrix", normalMatrix);
            shader_.setUniform("pickId", pickID);

            drawObject.draw();
        };

        const auto globalPickID = static_cast<unsigned int>(picking_.getPickingId(axisIDOffset));

        {
            // lower bound
            auto drawObject = MeshDrawerGL::getDrawObject(interactionHandleMesh_[0].get());
            draw(drawObject, globalPickID, lowerBound, axis.info.rotMatrix);

            // upper bound
            draw(drawObject, globalPickID + 1, upperBound, axis.info.flipMatrix);
        }

        {
            // middle handle
            if ((property.get().x > property.getRangeMin()) ||
                (property.get().y < property.getRangeMax())) {
                auto drawObject = MeshDrawerGL::getDrawObject(interactionHandleMesh_[1].get());
                if (std::fabs(upperBound - lowerBound) > minSeparationPercentage) {
                    draw(drawObject, globalPickID + 2, (upperBound + lowerBound) * 0.5f,
                         axis.info.rotMatrix);
                }
            }
        }
        shader_.setUniform("pickId", 0u);
    }

    if (showCropPlane_.get()) {
        bool drawLowerPlane = (property.get().x != property.getRangeMin());
        bool drawUpperPlane = (property.get().y != property.getRangeMax());

        if (drawLowerPlane || drawUpperPlane) {
            if (cropLineColor_.isModified()) {
                createLineStripMesh();
            }

            utilgl::DepthFuncState depthFunc(GL_LEQUAL);

            lineShader_.activate();
            lineShader_.setUniform("screenDim", vec2(outport_.getDimensions()));
            utilgl::setUniforms(lineShader_, camera_, lineWidth_);

            // rotate clip plane from [0, 0, -1] to match the currently selected clip axis
            mat4 scale(volumeBasis_);
            mat4 rotMatrix(1.0f);
            if (axis.axis != CartesianCoordinateAxis::Z) {
                vec3 v1(0.0f, 0.0f, -1.0f);
                vec3 v2(glm::normalize(axis.info.axis));
                rotMatrix = glm::rotate(glm::half_pi<float>(), glm::cross(v1, v2));
            }
            rotMatrix = scale * rotMatrix;

            MeshDrawerGL::DrawObject drawStrip(linestrip_->getRepresentation<MeshGL>(),
                                               linestrip_->getDefaultMeshInfo());

            auto draw = [&](float value) {
                mat4 worldMatrix(glm::translate(volumeOffset_ + axis.info.axis * value) *
                                 rotMatrix);
                mat3 normalMatrix(glm::inverseTranspose(worldMatrix));
                lineShader_.setUniform("geometry.dataToWorld", worldMatrix);
                lineShader_.setUniform("geometry.dataToWorldNormalMatrix", normalMatrix);
                drawStrip.draw();
            };

            if (drawLowerPlane) {
                draw(lowerBound);
            }

            if (drawUpperPlane) {
                draw(upperBound);
            }
        }
    }
}

void CropWidget::updateAxisRanges() {
    if (!volume_.hasData()) return;

    auto dims = util::getVolumeDimensions(volume_.getData());

    size3_t cropDims;
    for (int i = 0; i < 3; ++i) {
        cropDims[i] = cropAxes_[i].range.getRangeMax() + 1;
    }

    if (dims != cropDims) {
        NetworkLock lock(this);

        // crop range should be [0, dims - 1]
        for (int i = 0; i < 3; ++i) {
            if (relativeRangeAdjustment_.get()) {
                cropAxes_[i].range.setRangeNormalized(ivec2(0, dims[i] - 1));
            } else {
                cropAxes_[i].range.setRange(ivec2(0, dims[i] - 1));
            }

            // set the new dimensions to default if we were to press reset
            cropAxes_[i].range.setCurrentStateAsDefault();
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

void CropWidget::objectPicked(PickingEvent *e) {
    const auto axisID = e->getPickedId() / static_cast<size_t>(numInteractionWidgets);
    if (axisID >= cropAxes_.size()) {
        LogWarn("invalid picking ID");
        return;
    }
    if (e->getPressState() != PickingPressState::None) {
        if (e->getPressState() == PickingPressState::Press &&
            e->getPressItem() & PickingPressItem::Primary) {
            // initial activation with button press
            isMouseBeingPressedAndHold_ = true;
            lastState_ = cropAxes_[axisID].range.get();
        } else if (e->getPressState() == PickingPressState::Move &&
                   e->getPressItems() & PickingPressItem::Primary) {
            InteractionElement element =
                static_cast<InteractionElement>(e->getPickedId() % numInteractionWidgets);
            rangePositionHandlePicked(cropAxes_[axisID], e, element);
        } else if (e->getPressState() == PickingPressState::Release &&
                   e->getPressItem() & PickingPressItem::Primary) {
            isMouseBeingPressedAndHold_ = false;
            lastState_ = ivec2(-1);
        }
        e->markAsUsed();
    }
}

CropWidget::AnnotationInfo CropWidget::getAxis(CartesianCoordinateAxis majorAxis) {
    auto &cam = camera_.get();
    std::array<vec2, 4> axisSelector = {{{0, 0}, {1, 0}, {1, 1}, {0, 1}}};

    auto viewMatrix(cam.getViewMatrix());
    auto viewprojMatrix = cam.getProjectionMatrix() * cam.getViewMatrix();

    std::array<int, 3> indices;  // encodes the index of the primary axis and the other two axes

    mat4 rotMatrix(1.0f);
    mat4 flipOrientationMat;  // matrix used for the second arrow facing the opposite direction
                              // tilt the arrow mesh so that it is rotated by 45 degree
    switch (majorAxis) {
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

void CropWidget::rangePositionHandlePicked(CropAxis &cropAxis, PickingEvent *p,
                                           InteractionElement element) {
    auto currNDC = p->getNDC();
    auto prevNDC = p->getPressedNDC();

    // Use depth of initial press as reference to move in the image plane.
    auto refDepth = p->getPressedDepth();
    currNDC.z = refDepth;
    prevNDC.z = refDepth;

    // project mouse delta onto axis
    vec2 delta(currNDC - prevNDC);
    vec2 axis2D(cropAxis.info.endNDC - cropAxis.info.startNDC);
    float dist = glm::dot(delta, glm::normalize(axis2D));

    auto &property = cropAxis.range;

    auto value = property.get();
    auto range = property.getRange();
    bool modified = false;
    switch (element) {
        case InteractionElement::UpperBound: {
            int v = lastState_.y + static_cast<int>(dist * (range.y - range.x));
            v = std::max(v, property.getMinSeparation() + lastState_.x);
            modified = (value.y != v);
            value.y = v;
            break;
        }
        case InteractionElement::LowerBound: {
            int v = lastState_.x + static_cast<int>(dist * (range.y - range.x));
            v = std::min(v, lastState_.y - property.getMinSeparation());
            modified = (value.x != v);
            value.x = v;
            break;
        }
        case InteractionElement::Middle: {
            // adjust both lower and upper bound
            int v = lastState_.x + static_cast<int>(dist * (range.y - range.x));
            v = std::min(v, range.y - property.getMinSeparation());
            modified = (value.x != v);
            value.x = v;
            value.y = std::min(v + lastState_.y - lastState_.x, range.y);
            break;
        }
        case InteractionElement::None:
        default:
            break;
    }
    if (modified) {
        property.set(value);
    }
}

}  // namespace inviwo
