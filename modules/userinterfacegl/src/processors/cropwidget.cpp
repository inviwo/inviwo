/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2026 Inviwo Foundation
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

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/algorithm/boundingbox.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <inviwo/core/datastructures/camera/camera.h>
#include <inviwo/core/datastructures/geometry/geometrytype.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/image/imagetypes.h>
#include <inviwo/core/interaction/events/pickingevent.h>
#include <inviwo/core/interaction/pickingmapper.h>
#include <inviwo/core/interaction/pickingstate.h>
#include <inviwo/core/io/datareader.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/glmmat.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/volumeutils.h>
#include <inviwo/core/util/zip.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/texture/textureutils.h>

#include <algorithm>
#include <cmath>
#include <string_view>
#include <array>

#include <flags/flags.h>
#include <fmt/base.h>
#include <glm/gtx/transform.hpp>
#include <glm/ext/scalar_constants.hpp>

namespace inviwo {

namespace {

struct AxisParams {
    dvec3 start;
    dvec3 stop;
    dvec3 tickDir;
};

std::array<AxisParams, 3> findAxisPositions(dvec3 viewDirection) {
    constexpr dvec3 center = {0.5, 0.5, 0.5};
    constexpr std::array<dvec3, 8> corners = {
        {{0, 0, 0}, {0, 0, 1}, {0, 1, 0}, {0, 1, 1}, {1, 0, 0}, {1, 0, 1}, {1, 1, 0}, {1, 1, 1}}};
    struct AI {
        std::array<size_t, 2> idx;
        std::array<dvec3, 2> faceNormals;
    };

    constexpr std::array<std::array<AI, 4>, 3> meta = {
        {{AI{.idx = {0, 4}, .faceNormals = {dvec3{0, -1, 0}, dvec3{0, 0, -1}}},
          AI{.idx = {1, 5}, .faceNormals = {dvec3{0, -1, 0}, dvec3{0, 0, 1}}},
          AI{.idx = {2, 6}, .faceNormals = {dvec3{0, 0, -1}, dvec3{0, 1, 0}}},
          AI{.idx = {3, 7}, .faceNormals = {dvec3{0, 1, 0}, dvec3{0, 0, 1}}}},
         {AI{.idx = {0, 2}, .faceNormals = {dvec3{-1, 0, 0}, dvec3{0, 0, -1}}},
          AI{.idx = {1, 3}, .faceNormals = {dvec3{-1, 0, 0}, dvec3{0, 0, 1}}},
          AI{.idx = {4, 6}, .faceNormals = {dvec3{0, 0, -1}, dvec3{1, 0, 0}}},
          AI{.idx = {5, 7}, .faceNormals = {dvec3{1, 0, 0}, dvec3{0, 0, 1}}}},
         {AI{.idx = {0, 1}, .faceNormals = {dvec3{-1, 0, 0}, dvec3{0, -1, 0}}},
          AI{.idx = {2, 3}, .faceNormals = {dvec3{-1, 0, 0}, dvec3{0, 1, 0}}},
          AI{.idx = {4, 5}, .faceNormals = {dvec3{0, -1, 0}, dvec3{1, 0, 0}}},
          AI{.idx = {6, 7}, .faceNormals = {dvec3{1, 0, 0}, dvec3{0, 1, 0}}}}}};

    const auto onEdge = [&](const AI& edge) {
        return glm::sign(glm::dot(edge.faceNormals[0], viewDirection)) !=
               glm::sign(glm::dot(edge.faceNormals[1], viewDirection));
    };

    const auto dist = [&](const AI& edge) {
        return 0.5 * (glm::dot(corners[edge.idx[0]] - center, viewDirection) +
                      glm::dot(corners[edge.idx[1]] - center, viewDirection));
    };

    const auto tickDir = [&](const AI& edge) -> dvec3 {
        const auto normal = glm::dot(edge.faceNormals[0], viewDirection) > 0 ? edge.faceNormals[0]
                                                                             : edge.faceNormals[1];
        const auto axis = corners[edge.idx[1]] - corners[edge.idx[0]];
        auto dir = glm::cross(axis, normal);
        return dir * glm::sign(glm::dot(center - corners[edge.idx[1]], dir));
    };

    const auto find = [&](const std::array<AI, 4>& axis) -> AxisParams {
        const auto& min = *std::ranges::max_element(axis, [&](const AI& a, const AI& b) {
            return (dist(a) + (onEdge(a) ? 0 : -100.0)) < (dist(b) + (onEdge(b) ? 0 : -100.0));
        });

        return {.start = corners[min.idx[0]], .stop = corners[min.idx[1]], .tickDir = tickDir(min)};
    };

    return {find(meta[0]), find(meta[1]), find(meta[2])};
}

dmat4 getTransform(const SpatialEntity& entity, std::optional<dmat4> worldBoundingBox) {
    return worldBoundingBox.value_or(entity.getCoordinateTransformer().getDataToWorldMatrix());
}

}  // namespace

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo CropWidget::processorInfo_{
    "org.inviwo.CropWidget",  // Class identifier
    "Crop Widget",            // Display name
    "UI",                     // Category
    CodeState::Stable,        // Code state
    "GL, UI, Clipping",       // Tags
    "Provides interaction handles for interactively cropping a volume."_help,
};
const ProcessorInfo& CropWidget::getProcessorInfo() const { return processorInfo_; }

CropWidget::CropWidget()
    : Processor()
    , inport_("inport", "input image"_help)
    , volume_("volume", "input volume used to determine the bounding box"_help)
    , outport_("outport",
               "output image with the interaction handles rendered on top of the input image"_help)

    , showWidget_("showWidget", "Show Widget", true)
    , uiSettings_("uiSettings", "UI Settings",
                  "various properties for adjusting the visual appearance"_help)
    , showCropPlane_("showClipPlane", "Crop Plane Visible", true)
    , handleColor_("handleColor", "Handle Color", util::ordinalColor(vec4(0.8f, 0.4f, 0.1f, 1.0f)))
    , cropLineSettings_("cropLineSettings", "Crop Line Settings")
    , offset_("offset", "Offset", util::ordinalSymmetricVector(0.0, 1.0))
    , scale_("scale", "Scale", util::ordinalScale(0.15, 2.0).setInc(0.001))

    , ranges_({{{"cropX", "Crop X", 0, 256, 0, 256, 1, 1},
                {"cropY", "Crop Y", 0, 256, 0, 256, 1, 1},
                {"cropZ", "Crop Z", 0, 256, 0, 256, 1, 1}}})
    , axisInfo_({{{.axis = CartesianCoordinateAxis::X},
                  {.axis = CartesianCoordinateAxis::Y},
                  {.axis = CartesianCoordinateAxis::Z}}})
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
    , pickingIDs_{}
    , isMouseBeingPressedAndHold_(false)
    , lastState_(-1)
    , interactionHandleMesh_{}
    , linestrip_{}
    , lineRenderer_()
    , getBoundingBox_{util::boundingBox(volume_)} {

    addPorts(volume_, inport_, outport_);

    inport_.setOptional(true);

    addProperties(showWidget_);
    for (auto& elem : ranges_) {
        // Since the clips depend on the input volume dimensions, we make sure to always
        // serialize them so we can do a proper renormalization when we load new data.
        elem.setSerializationMode(PropertySerializationMode::All);
        addProperty(elem);
    }

    cropLineSettings_.lineWidth.set(2.5f);
    cropLineSettings_.defaultColor.set(vec4{0.8f, 0.8f, 0.8f, 1.0f});

    // brighten up ambient color
    lightingProperty_.ambientColor_.set(vec3(0.6f));
    lightingProperty_.setCollapsed(true);
    camera_.setCollapsed(true);
    uiSettings_.setCollapsed(true);
    uiSettings_.addProperties(handleColor_, offset_, scale_, showCropPlane_, cropLineSettings_,
                              lightingProperty_);

    addProperties(relativeRangeAdjustment_, uiSettings_, camera_, trackball_);

    setAllPropertiesCurrentStateAsDefault();

    volume_.onChange([this]() { updateAxisRanges(); });
    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });

    static constexpr std::array<InteractionElement, 3> elem = {
        InteractionElement::LowerBound, InteractionElement::UpperBound, InteractionElement::Middle};
    for (size_t i = 0; i < pickingIDs_.size(); ++i) {
        pickingIDs_[i] = {.id = picking_.getPickingId(i),
                          .element = elem[i % numInteractionWidgets]};
    }
}

CropWidget::~CropWidget() = default;

void CropWidget::process() {
    if (!interactionHandleMesh_[0]) {
        initMesh();
    }

    utilgl::activateTargetAndClearOrCopySource(outport_, inport_);

    if (showWidget_ || showCropPlane_) {
        const utilgl::GlBoolState depthTest{GL_DEPTH_TEST, true};
        const utilgl::BlendModeState blending{GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA};
        const utilgl::CullFaceState culling{GL_BACK};
        shader_.activate();

        utilgl::setShaderUniforms(shader_, camera_, "camera");
        utilgl::setShaderUniforms(shader_, lightingProperty_, "lighting");
        shader_.setUniform("overrideColor", handleColor_.get());

        const auto maybeBB =
            std::optional<dmat4>{getBoundingBox_ ? getBoundingBox_() : std::nullopt}.transform(
                util::minExtentBoundingBox);
        const auto d2w = getTransform(*volume_.getData(), maybeBB);
        const auto nD2W = glm::transpose(glm::inverse(dmat3{d2w}));

        const auto w2ndc = camera_.get().getProjectionMatrix() * camera_.get().getViewMatrix();

        // transform camera to data space since findAxisPositions uses a uniform cube centered at
        // the origin to determine the visible faces
        const dmat4 trafo{glm::inverse(d2w)};
        const auto axes =
            findAxisPositions(glm::normalize(vec3(trafo * dvec4(camera_.getLookFrom(), 1.0)) -
                                             vec3(trafo * dvec4(camera_.getLookTo(), 1.0))));
        for (auto&& [i, axis] : util::enumerate(axes)) {
            const dvec3 center{0.5, 0.5, 0.5};
            const auto offsetDir = glm::normalize(
                dmat3{d2w} * glm::normalize(axis.start - center + axis.stop - center));
            const dvec3 start{dvec3{d2w * dvec4(axis.start, 1)} + offsetDir * offset_.get()};
            const dvec3 stop{dvec3{d2w * dvec4(axis.stop, 1)} + offsetDir * offset_.get()};
            const dvec3 dir{normalize(stop - start)};
            const dvec3 tickDir{glm::normalize(nD2W * axis.tickDir)};

            const dmat4 rot{dmat3{dir, glm::cross(tickDir, dir), tickDir}};
            const dmat4 flip{rot * glm::rotate(glm::pi<double>(), dvec3{0.0, 1.0, 0.0})};

            const auto startNdc = w2ndc * dvec4{start, 1.0};
            axisInfo_[i].startNDC = dvec3{startNdc} / startNdc.w;
            const auto endNdc = w2ndc * dvec4{stop, 1.0};
            axisInfo_[i].endNDC = dvec3{endNdc} / endNdc.w;

            renderAxis(i, start, stop, rot, flip);
        }
    }
    utilgl::deactivateCurrentTarget();
}

void CropWidget::initializeResources() {
    utilgl::addShaderDefines(shader_, lightingProperty_);
    shader_.build();
}

void CropWidget::initMesh() {
    auto load = [this](std::string_view file) -> std::shared_ptr<const Mesh> {
        auto* app = getNetwork()->getApplication();
        const auto* module = app->getModuleByIdentifier("UserInterfaceGL");
        if (!module) {
            throw Exception("Could not locate module 'UserInterfaceGL'");
        }

        auto reader =
            app->getDataReaderFactory()->getReaderForTypeAndExtension<Mesh>(LCString{"fbx"});
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

    auto* vBuffer = vertices->getEditableRAMRepresentation();

    const std::array<vec3, 4> mask{{
        {0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 1.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
    }};
    for (const auto& elem : mask) {
        vBuffer->add(elem);
    }

    auto indices = std::make_shared<IndexBuffer>();
    auto* indexBuffer = indices->getEditableRAMRepresentation();
    indexBuffer->add({3, 0, 1, 2, 3, 0, 1});

    linestrip->addBuffer(BufferType::PositionAttrib, vertices);
    linestrip->addIndices(Mesh::MeshInfo(DrawType::Lines, ConnectivityType::StripAdjacency),
                          indices);

    linestrip_ = linestrip;
}

void CropWidget::renderAxis(size_t axisIndex, dvec3 start, dvec3 stop, const dmat4& rotMat,
                            const dmat4& flipMat) {
    // if min separation of the range is smaller, the middle handle is not drawn
    const float minSeparationPercentage = 0.05f;

    const auto& property = ranges_[axisIndex];

    const auto range = static_cast<double>(property.getRangeMax() - property.getRangeMin());
    const auto lowerBound = static_cast<double>(property.get().x - property.getRangeMin()) / range;
    const auto upperBound = static_cast<double>(property.get().y - property.getRangeMin()) / range;

    // draw the interaction handles
    if (showWidget_) {
        const int axisIDOffset =
            static_cast<int>(axisInfo_[axisIndex].axis) * static_cast<int>(numInteractionWidgets);

        shader_.activate();

        // apply custom transformation
        const auto m = glm::scale(dvec3{scale_.get()});

        auto draw = [&](auto& drawObject, unsigned int pickID, double value, const dmat4& rot) {
            const auto worldMatrix{glm::translate(glm::mix(start, stop, value)) * m * rot};
            const auto normalMatrix{glm::inverseTranspose(dmat3{worldMatrix})};
            shader_.setUniform("geometry.dataToWorld", worldMatrix);
            shader_.setUniform("geometry.dataToWorldNormalMatrix", normalMatrix);
            shader_.setUniform("pickId", pickID);

            drawObject.draw();
        };

        const auto globalPickID = static_cast<unsigned int>(picking_.getPickingId(axisIDOffset));

        {
            // lower bound
            auto drawObject = MeshDrawerGL::getDrawObject(interactionHandleMesh_[0].get());
            draw(drawObject, globalPickID, lowerBound, rotMat);

            // upper bound
            draw(drawObject, globalPickID + 1, upperBound, flipMat);
        }

        {
            // middle handle
            if ((property.get().x > property.getRangeMin()) ||
                (property.get().y < property.getRangeMax())) {
                auto drawObject = MeshDrawerGL::getDrawObject(interactionHandleMesh_[1].get());
                if (std::fabs(upperBound - lowerBound) > minSeparationPercentage) {
                    draw(drawObject, globalPickID + 2, (upperBound + lowerBound) * 0.5, rotMat);
                }
            }
        }
        shader_.setUniform("pickId", 0u);
    }

    if (showCropPlane_.get()) {
        const bool drawLowerPlane = (property.get().x != property.getRangeMin());
        const bool drawUpperPlane = (property.get().y != property.getRangeMax());

        if (drawLowerPlane || drawUpperPlane) {
            if (!linestrip_) {
                createLineStripMesh();
            }

            const utilgl::DepthFuncState depthFunc(GL_LEQUAL);

            const auto basis{volume_.getData()->getBasis()};
            const auto offset{volume_.getData()->getOffset()};

            // rotate clip plane from [-1, 0, 0] to match the currently selected clip axis
            dmat4 rotMatrix{basis};
            if (axisInfo_[axisIndex].axis != CartesianCoordinateAxis::X) {
                rotMatrix *=
                    glm::toMat4(glm::rotation(dvec3{-1.0, 0.0, 0.0}, dmat3{1.0}[axisIndex]));
            }

            auto draw = [&](double value) {
                dmat4 worldMatrix{rotMatrix};
                worldMatrix[3] = dvec4{offset + basis[axisIndex] * value, 1.0};
                linestrip_->setWorldMatrix(worldMatrix);

                LineData lineData;
                cropLineSettings_.update(lineData);

                lineRenderer_.render(*linestrip_, camera_.get(), outport_.getDimensions(),
                                     lineData);
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

    size3_t cropDims{};
    for (int i = 0; i < 3; ++i) {
        cropDims[i] = ranges_[i].getRangeMax();
    }

    if (dims != cropDims) {
        const NetworkLock lock(this);

        // crop range should be [0, dims)
        for (int i = 0; i < 3; ++i) {
            if (relativeRangeAdjustment_.get()) {
                ranges_[i].setRangeNormalized(ivec2(0, dims[i]));
            } else {
                ranges_[i].setRange(ivec2(0, dims[i]));
            }

            // set the new dimensions to default if we were to press reset
            ranges_[i].setCurrentStateAsDefault();
        }
    }
}

void CropWidget::objectPicked(PickingEvent* e) {
    const auto axisID = e->getPickedId() / static_cast<size_t>(numInteractionWidgets);
    if (axisID >= ranges_.size()) {
        log::warn("invalid picking ID");
        return;
    }
    if (e->getPressState() != PickingPressState::None) {
        if (e->getPressState() == PickingPressState::Press &&
            e->getPressItem() & PickingPressItem::Primary) {
            // initial activation with button press
            isMouseBeingPressedAndHold_ = true;
            lastState_ = ranges_[axisID].get();
        } else if (e->getPressState() == PickingPressState::Move &&
                   e->getPressItems() & PickingPressItem::Primary) {
            const auto element =
                static_cast<InteractionElement>(e->getPickedId() % numInteractionWidgets);
            rangePositionHandlePicked(axisID, e, element);
        } else if (e->getPressState() == PickingPressState::Release &&
                   e->getPressItem() & PickingPressItem::Primary) {
            isMouseBeingPressedAndHold_ = false;
            lastState_ = ivec2(-1);
        }
        e->markAsUsed();
    }
}

void CropWidget::rangePositionHandlePicked(size_t axisIndex, PickingEvent* p,
                                           InteractionElement element) {
    // project mouse delta onto axis
    const dvec2 delta{p->getNDC() - p->getPressedNDC()};
    const dvec2 axis2D{axisInfo_[axisIndex].endNDC - axisInfo_[axisIndex].startNDC};
    const auto dist = glm::dot(delta, glm::normalize(axis2D)) / glm::length(axis2D);

    auto& property = ranges_[axisIndex];

    auto value = property.get();
    const auto range = property.getRange();
    bool modified = false;
    switch (element) {
        case InteractionElement::UpperBound: {
            int v = lastState_.y + static_cast<int>(dist * static_cast<float>(range.y - range.x));
            v = std::max(v, property.getMinSeparation() + lastState_.x);
            modified = (value.y != v);
            value.y = v;
            break;
        }
        case InteractionElement::LowerBound: {
            int v = lastState_.x + static_cast<int>(dist * static_cast<float>(range.y - range.x));
            v = std::min(v, lastState_.y - property.getMinSeparation());
            modified = (value.x != v);
            value.x = v;
            break;
        }
        case InteractionElement::Middle: {
            // adjust both lower and upper bound
            int v = lastState_.x + static_cast<int>(dist * static_cast<float>(range.y - range.x));
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
