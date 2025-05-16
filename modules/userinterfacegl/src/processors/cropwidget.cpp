/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2025 Inviwo Foundation
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

#include <inviwo/core/common/inviwoapplication.h>                       // for InviwoApplication
#include <inviwo/core/common/inviwomodule.h>                            // for InviwoModule, Mod...
#include <inviwo/core/datastructures/buffer/buffer.h>                   // for Buffer, IndexBuffer
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>       // for BufferRAMPrecision
#include <inviwo/core/datastructures/camera/camera.h>                   // for Camera
#include <inviwo/core/datastructures/geometry/geometrytype.h>           // for CartesianCoordina...
#include <inviwo/core/datastructures/geometry/mesh.h>                   // for Mesh, Mesh::MeshInfo
#include <inviwo/core/datastructures/image/imagetypes.h>                // for ImageType, ImageT...
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/interaction/cameratrackball.h>                    // for CameraTrackball
#include <inviwo/core/interaction/events/pickingevent.h>                // for PickingEvent
#include <inviwo/core/interaction/pickingmapper.h>                      // for PickingMapper
#include <inviwo/core/interaction/pickingstate.h>                       // for PickingPressItem
#include <inviwo/core/io/datareader.h>                                  // for DataReaderType
#include <inviwo/core/io/datareaderfactory.h>                           // for DataReaderFactory
#include <inviwo/core/network/networklock.h>                            // for NetworkLock
#include <inviwo/core/network/processornetwork.h>                       // for ProcessorNetwork
#include <inviwo/core/ports/imageport.h>                                // for ImageInport, Imag...
#include <inviwo/core/ports/volumeport.h>                               // for VolumeInport
#include <inviwo/core/processors/processor.h>                           // for Processor
#include <inviwo/core/processors/processorinfo.h>                       // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                      // for CodeState, CodeSt...
#include <inviwo/core/processors/processortags.h>                       // for Tags
#include <inviwo/core/properties/boolproperty.h>                        // for BoolProperty
#include <inviwo/core/properties/cameraproperty.h>                      // for CameraProperty
#include <inviwo/core/properties/compositeproperty.h>                   // for CompositeProperty
#include <inviwo/core/properties/invalidationlevel.h>                   // for InvalidationLevel
#include <inviwo/core/properties/minmaxproperty.h>                      // for IntMinMaxProperty
#include <inviwo/core/properties/ordinalproperty.h>                     // for FloatVec4Property
#include <inviwo/core/properties/propertysemantics.h>                   // for PropertySemantics
#include <inviwo/core/properties/simplelightingproperty.h>              // for SimpleLightingPro...
#include <inviwo/core/properties/valuewrapper.h>                        // for PropertySerializa...
#include <inviwo/core/util/exception.h>                                 // for Exception
#include <inviwo/core/util/glmmat.h>                                    // for mat4, mat3
#include <inviwo/core/util/glmvec.h>                                    // for vec3, vec4, vec2
#include <inviwo/core/util/logcentral.h>                                // for LogVerbosity, Log...
#include <inviwo/core/util/sourcecontext.h>                             // for SourceContext
#include <inviwo/core/util/stringconversion.h>                          // for toString
#include <inviwo/core/util/volumeutils.h>                               // for getVolumeDimensions
#include <modules/opengl/geometry/meshgl.h>                             // for MeshGL
#include <modules/opengl/inviwoopengl.h>                                // for GL_DEPTH_TEST
#include <modules/opengl/openglutils.h>                                 // for BlendModeState
#include <modules/opengl/rendering/meshdrawergl.h>                      // for MeshDrawerGL::Dra...
#include <modules/opengl/shader/shader.h>                               // for Shader, Shader::B...
#include <modules/opengl/shader/shaderobject.h>                         // for ShaderObject
#include <modules/opengl/shader/shadertype.h>                           // for ShaderType, Shade...
#include <modules/opengl/shader/shaderutils.h>                          // for setShaderUniforms
#include <modules/opengl/texture/textureutils.h>                        // for activateAndClearT...

#include <algorithm>      // for min, sort, max
#include <cmath>          // for fabs
#include <cstddef>        // for size_t
#include <functional>     // for __base, function
#include <map>            // for map
#include <string>         // for operator+, string
#include <string_view>    // for string_view
#include <type_traits>    // for remove_extent_t
#include <unordered_map>  // for unordered_map
#include <unordered_set>  // for unordered_set
#include <vector>         // for vector

#include <flags/flags.h>                 // for operator&, flags
#include <fmt/core.h>                    // for format
#include <glm/detail/qualifier.hpp>      // for tvec2
#include <glm/ext/matrix_transform.hpp>  // for rotate, translate
#include <glm/ext/scalar_constants.hpp>  // for pi
#include <glm/geometric.hpp>             // for dot, normalize
#include <glm/gtc/constants.hpp>         // for half_pi, quarter_pi
#include <glm/gtc/epsilon.hpp>           // for epsilonEqual
#include <glm/gtc/matrix_inverse.hpp>    // for inverseTranspose
#include <glm/gtx/transform.hpp>         // for rotate, translate
#include <glm/vec2.hpp>                  // for vec<>::(anonymous)
#include <glm/vec4.hpp>                  // for operator*, operator+

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo CropWidget::processorInfo_{
    "org.inviwo.CropWidget",  // Class identifier
    "Crop Widget",            // Display name
    "UI",                     // Category
    CodeState::Stable,        // Code state
    "GL, UI, Clipping",       // Tags
    "Processor for providing interaction handles for cropping a volume."_help,
};
const ProcessorInfo& CropWidget::getProcessorInfo() const { return processorInfo_; }

CropWidget::CropWidget()
    : Processor()
    , inport_("inport", "input image"_help)
    , volume_("volume", "input volume used to determine the bounding box"_help)
    , outport_("outport",
               "output image with the interaction handles rendered on top of the input image"_help)

    , uiSettings_("uiSettings", "UI Settings",
                  "various properties for adjusting the visual appearance"_help)
    , showWidget_("showWidget", "Show Widget", true)
    , showCropPlane_("showClipPlane", "Crop Plane Visible", true)
    , handleColor_("handleColor", "Handle Color", util::ordinalColor(vec4(0.8f, 0.4f, 0.1f, 1.0f)))
    , cropLineSettings_("cropLineSettings", "Crop Line Settings")
    , offset_("offset", "Offset", 0.0f, -1.0f, 1.0f)
    , scale_("scale", "Scale", 0.15f, 0.001f, 2.0f, 0.05f)

    , cropAxes_({{{CartesianCoordinateAxis::X,
                   {"cropAxisX", "Crop X", "enable and adjust crop range along the x axis"_help},
                   {"cropAxisXEnabled", "Enabled", true},
                   {"cropX", "Range", 0, 256, 0, 256, 1, 1},
                   {"cropXOut", "Crop X", 0, 256, 0, 256, 1, 1},
                   AnnotationInfo()},
                  {CartesianCoordinateAxis::Y,
                   {"cropAxisY", "Crop Y", "enable and adjust crop range along the y axis"_help},
                   {"cropAxisYEnabled", "Enabled", true},
                   {"cropY", "Range", 0, 256, 0, 256, 1, 1},
                   {"cropYOut", "Crop Y", 0, 256, 0, 256, 1, 1},
                   AnnotationInfo()},
                  {CartesianCoordinateAxis::Z,
                   {"cropAxisZ", "Crop Z", "enable and adjust crop range along the z axis"_help},
                   {"cropAxisZEnabled", "Enabled", true},
                   {"cropZ", "Range", 0, 256, 0, 256, 1, 1},
                   {"cropZOut", "Crop Z", 0, 256, 0, 256, 1, 1},
                   AnnotationInfo()}}})
    , relativeRangeAdjustment_("relativeRangeAdjustment", "Rel. Adjustment on Range Change", true)
    , outputProps_("outputProperties", "Output")
    , camera_("camera", "Camera")

    , lightingProperty_("internalLighting", "Lighting",
                        "Lighting parameters used for shading the handles"_help,
                        LightingConfig{
                            .position = vec3{4.0f, 6.6f, 18.0f},
                            .referenceSpace = CoordinateSpace::View,
                            .ambient = vec3{0.6f},
                        },
                        &camera_)
    , trackball_(&camera_)
    , picking_(this, 3 * numInteractionWidgets, [&](PickingEvent* p) { objectPicked(p); })
    , shader_("geometrycustompicking.vert", "geometryrendering.frag", Shader::Build::No)
    , isMouseBeingPressedAndHold_(false)
    , lastState_(-1)
    , volumeBasis_(1.0f)
    , volumeOffset_(-0.5f)
    , lineRenderer_(&cropLineSettings_) {

    addPort(volume_);
    addPort(inport_);
    addPort(outport_);

    inport_.setOptional(true);

    for (auto& elem : cropAxes_) {
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

    cropLineSettings_.lineWidth.set(2.5f);
    cropLineSettings_.lineWidth.setCurrentStateAsDefault();
    cropLineSettings_.defaultColor.set(vec4{0.8f, 0.8f, 0.8f, 1.0f});
    cropLineSettings_.defaultColor.setCurrentStateAsDefault();

    // brighten up ambient color
    lightingProperty_.ambientColor_.set(vec3(0.6f));
    lightingProperty_.setCollapsed(true);

    uiSettings_.setCollapsed(true);
    uiSettings_.addProperty(showWidget_);

    uiSettings_.addProperty(handleColor_);
    uiSettings_.addProperty(offset_);
    uiSettings_.addProperty(scale_);
    uiSettings_.addProperty(showCropPlane_);
    uiSettings_.addProperty(cropLineSettings_);
    uiSettings_.addProperty(lightingProperty_);
    addProperty(uiSettings_);

    camera_.setCollapsed(true);

    addProperty(camera_);
    addProperty(trackball_);

    setAllPropertiesCurrentStateAsDefault();

    volume_.onChange([this]() { updateAxisRanges(); });
    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });

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

        for (auto& elem : cropAxes_) {
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
}

void CropWidget::initMesh() {

    auto load = [this](std::string_view file) -> std::shared_ptr<const Mesh> {
        auto app = getNetwork()->getApplication();
        auto module = app->getModuleByIdentifier("UserInterfaceGL");
        if (!module) {
            throw Exception("Could not locate module 'UserInterfaceGL'");
        }

        auto reader = app->getDataReaderFactory()->getReaderForTypeAndExtension<Mesh>("fbx");
        if (!reader) {
            throw Exception("Could not fbx mesh reader");
        }
        reader->setOption("FixInvalidData", false);
        reader->setOption("LogLevel", LogVerbosity::Error);

        return reader->readData(module->getPath(ModulePath::Data) / "meshes" / file);
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

    static std::array<std::weak_ptr<const Mesh>, 2> meshes;

    // interaction handles
    interactionHandleMesh_[0] = cache(meshes[0], load, "arrow-single.fbx");
    interactionHandleMesh_[1] = cache(meshes[1], load, "crop-handle.fbx");

    createLineStripMesh();
}

void CropWidget::createLineStripMesh() {
    auto linestrip = std::make_shared<Mesh>(DrawType::Lines, ConnectivityType::StripAdjacency);
    auto vertices = std::make_shared<Buffer<vec3>>();

    auto vBuffer = vertices->getEditableRAMRepresentation();

    vec3 p(0.0f);
    vec3 mask[5] = {{0.0f, 0.0f, 0.0f},
                    {1.0f, 0.0f, 0.0f},
                    {1.0f, 1.0f, 0.0f},
                    {0.0f, 1.0f, 0.0f},
                    {0.0f, 0.0f, 0.0f}};
    for (int i = 0; i < 5; ++i) {
        vBuffer->add(p + mask[i]);
    }

    auto indices = std::make_shared<IndexBuffer>();
    auto indexBuffer = indices->getEditableRAMRepresentation();
    indexBuffer->add({3, 0, 1, 2, 3, 4, 1});

    linestrip->addBuffer(BufferType::PositionAttrib, vertices);
    linestrip->addIndices(Mesh::MeshInfo(DrawType::Lines, ConnectivityType::StripAdjacency),
                          indices);

    linestrip_ = linestrip;
}

void CropWidget::renderAxis(const CropAxis& axis) {
    // if min separation of the range is smaller, the middle handle is not drawn
    const float minSeparationPercentage = 0.05f;

    auto& property = axis.range;

    float range = static_cast<float>(property.getRangeMax() - property.getRangeMin());
    float lowerBound = (property.get().x - property.getRangeMin()) / range;
    float upperBound = (property.get().y - property.getRangeMin()) / range;

    // draw the interaction handles
    if (showWidget_) {
        const int axisIDOffset = static_cast<int>(axis.axis) * numInteractionWidgets;

        shader_.activate();

        // apply custom transformation
        const mat4 m = glm::scale(vec3(scale_.get()));

        auto draw = [&](auto& drawObject, unsigned int pickID, float value, const mat4& rot) {
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
            if (!linestrip_) {
                createLineStripMesh();
            }

            utilgl::DepthFuncState depthFunc(GL_LEQUAL);

            // rotate clip plane from [0, 0, -1] to match the currently selected clip axis
            mat4 scale(volumeBasis_);
            mat4 rotMatrix(1.0f);
            if (axis.axis != CartesianCoordinateAxis::Z) {
                vec3 v1(0.0f, 0.0f, -1.0f);
                vec3 v2(glm::normalize(axis.info.axis));
                rotMatrix = glm::rotate(glm::half_pi<float>(), glm::cross(v1, v2));
            }
            rotMatrix = scale * rotMatrix;

            auto draw = [&](float value) {
                mat4 worldMatrix(glm::translate(volumeOffset_ + axis.info.axis * value) *
                                 rotMatrix);
                linestrip_->setWorldMatrix(worldMatrix);
                mat3 normalMatrix(glm::inverseTranspose(worldMatrix));

                lineRenderer_.render(*linestrip_, camera_.get(), outport_.getDimensions(),
                                     &cropLineSettings_);
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

void CropWidget::objectPicked(PickingEvent* e) {
    const auto axisID = e->getPickedId() / static_cast<size_t>(numInteractionWidgets);
    if (axisID >= cropAxes_.size()) {
        log::warn("invalid picking ID");
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
    auto& cam = camera_.get();
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

void CropWidget::rangePositionHandlePicked(CropAxis& cropAxis, PickingEvent* p,
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

    auto& property = cropAxis.range;

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
