/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <modules/basegl/interaction/gizmo.h>

#include <inviwo/core/datastructures/camera/camera.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/geometry/plane.h>
#include <inviwo/core/datastructures/geometry/simplemeshcreator.h>
#include <inviwo/core/datastructures/spatialdata.h>
#include <inviwo/core/interaction/events/pickingevent.h>
#include <inviwo/core/interaction/pickingmanager.h>
#include <inviwo/core/interaction/pickingmapper.h>
#include <inviwo/core/interaction/pickingstate.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/glm.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/singleton.h>

#include <modules/base/algorithm/meshutils.h>
#include <modules/opengl/image/imagegl.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/openglutils.h>

#include <fmt/format.h>
#include <glm/gtx/quaternion.hpp>

#include <numbers>
#include <ranges>
#include <algorithm>

namespace inviwo {

std::string format_as(const GizmoData& data) {
    return fmt::format(
        "basis: {::.3g}, origin: {::.3g},\ntype: {}, state: {}, hovered: {}, active: {}, picking "
        "ID: {}",
        data.basis, data.origin, data.type, data.state, data.hovered, data.active,
        data.globalPickingId);
}

Document DataTraits<GizmoData>::info(const GizmoData& data) {

    using P = Document::PathComponent;
    using H = utildoc::TableBuilder::Header;
    Document doc;
    doc.append("b", "GizmoData", {{"style", "color:white;"}});
    utildoc::TableBuilder tb(doc.handle(), P::end());

    tb(H("Gizmo Type"), fmt::format("{}", data.type));
    tb(H("Gizmo State"), fmt::format("{}", data.state));
    tb(H("Basis"), fmt::format("{}", data.basis));

    return doc;
}

namespace {

Line getLineFromViewDirection(const Camera& camera, dvec3 ndc) {
    return {.point = camera.getLookFrom(),
            .direction = camera.getLookFrom() - camera.getWorldPosFromNormalizedDeviceCoords(ndc)};
}

constexpr GizmoComponent pickingIdToComponent(size_t id) {
    switch (id) {
        using enum GizmoComponent;
        case static_cast<size_t>(MoveX):           return MoveX;
        case static_cast<size_t>(MoveY):           return MoveY;
        case static_cast<size_t>(MoveZ):           return MoveZ;
        case static_cast<size_t>(RotateX):         return RotateX;
        case static_cast<size_t>(RotateY):         return RotateY;
        case static_cast<size_t>(RotateZ):         return RotateZ;
        case static_cast<size_t>(RotateTrackball): return RotateTrackball;
        default:                                   return None;
    }
}

GizmoAxis gizmoComponentToAxis(GizmoComponent comp) {
    switch (comp) {
        using enum GizmoComponent;
        case MoveX:   return GizmoAxis::X;
        case MoveY:   return GizmoAxis::Y;
        case MoveZ:   return GizmoAxis::Z;
        case RotateX: return GizmoAxis::X;
        case RotateY: return GizmoAxis::Y;
        case RotateZ: return GizmoAxis::Z;
        default:      return GizmoAxis::Invalid;
    }
}

GizmoType getGizmoInteractionType(GizmoComponent comp) {
    switch (comp) {
        using enum GizmoComponent;
        case MoveX:           [[fallthrough]];
        case MoveY:           [[fallthrough]];
        case MoveZ:           return GizmoType::Move;
        case RotateX:         [[fallthrough]];
        case RotateY:         [[fallthrough]];
        case RotateZ:         [[fallthrough]];
        case RotateTrackball: return GizmoType::Rotate;
        default:              return GizmoType::None;
    }
}

constexpr std::array components = {GizmoComponent::MoveX,          GizmoComponent::MoveY,
                                   GizmoComponent::MoveZ,          GizmoComponent::RotateX,
                                   GizmoComponent::RotateY,        GizmoComponent::RotateZ,
                                   GizmoComponent::RotateTrackball};

constexpr std::array moveComponents = {GizmoComponent::MoveX, GizmoComponent::MoveY,
                                       GizmoComponent::MoveZ};

constexpr std::array rotateComponents = {GizmoComponent::RotateX, GizmoComponent::RotateY,
                                         GizmoComponent::RotateZ};

}  // namespace

Gizmo::Gizmo(Processor* processor, CameraCallback getCamera)
    : space_{CoordinateSpace::World}
    , type_{GizmoType::Move}
    , gizmoBasis_{1.0}
    , picking_{processor, 7, [this](PickingEvent* e) { picking(e); }}
    , getCamera_{std::move(getCamera)} {

    if (!getCamera_) {
        throw Exception{"getCamera callback not set"};
    }
}

void Gizmo::setType(GizmoType type) { type_ = type; }

GizmoType Gizmo::getType() const { return type_; }

void Gizmo::setBasis(dmat4 basis) {
    // don't accept changes to the basis while an interaction is ongoing
    if (interaction_) return;

    space_ = CoordinateSpace::World;
    gizmoBasis_ = basis;
    onInvalidateCallback_.invokeAll();
}

void Gizmo::setBasis(dmat4 basis, const SpatialEntity& entity, CoordinateSpace space) {
    // don't accept changes to the basis while an interaction is ongoing
    if (interaction_) return;

    space_ = space;

    if (space_ != CoordinateSpace::World) {
        const auto& ct = entity.getCoordinateTransformer();
        gizmoBasis_ = ct.getMatrix(space, CoordinateSpace::World) * basis;
    } else {
        gizmoBasis_ = basis;
    }

    onInvalidateCallback_.invokeAll();
}

dmat4 Gizmo::getBasis() const { return gizmoBasis_; }

dmat4 Gizmo::getBasis(CoordinateSpace space, const SpatialEntity& entity) const {
    if (space == CoordinateSpace::World) {
        return gizmoBasis_;
    } else {
        const auto& ct = entity.getCoordinateTransformer();
        return ct.getMatrix(CoordinateSpace::World, space) * gizmoBasis_;
    }
}

CoordinateSpace Gizmo::getSpace() const { return space_; }

GizmoData Gizmo::getData() const {
    const dvec3 origin{
        interaction_.transform([](auto& state) { return dvec4{state.initialBasis[3]}; })
            .value_or(gizmoBasis_[3])};

    const auto activeComponent = interaction_.transform([](auto& state) { return state.component; })
                                     .value_or(GizmoComponent::None);

    return GizmoData{
        // consider only basis translation in world space
        .basis = (space_ == CoordinateSpace::World ? glm::translate(dvec3{gizmoBasis_[3]})
                                                   : gizmoBasis_),
        .origin = origin,
        .type = type_,
        .state = interaction_.has_value() ? GizmoDrawState::Interaction : GizmoDrawState::Visible,
        .hovered = highlight_.value_or(GizmoComponent::None),
        .active = activeComponent,
        .globalPickingId = static_cast<std::uint32_t>(picking_.getPickingId(0)),
        .rotationIndicators =
            std::ranges::contains(rotateComponents, activeComponent)
                ? interaction_.transform([](auto& state) { return state.rotationPositions; })
                : std::nullopt,
    };
}

void Gizmo::picking(PickingEvent* e) {
    bool invalidate = false;
    const util::OnScopeExit invalidationCheck{[&]() {
        if (invalidate) {
            onInvalidateCallback_.invokeAll();
        }
    }};

    if (e->getHoverState() == PickingHoverState::Enter ||
        (e->getGlobalPickingId() != e->getPreviousGlobalPickingId())) {
        // start highlight / increased alpha
        highlight_ = pickingIdToComponent(e->getPickedId());
        invalidate = true;
    } else if (e->getHoverState() == PickingHoverState::Exit) {
        // stop highlight
        highlight_ = std::nullopt;
        invalidate = true;
    }

    if (e->getPressState() != PickingPressState::None) {
        if (e->getPressState() == PickingPressState::Press &&
            e->getPressItem() & PickingPressItem::Primary) {
            startInteraction(e);
            invalidate = true;
        } else if (e->getPressState() == PickingPressState::Press &&
                   e->getPressItem() != PickingPressItem::Primary) {
            cancelInteraction();
        } else if (e->getPressState() == PickingPressState::Move &&
                   e->getPressItems() & PickingPressItem::Primary && interaction_) {
            dragInteraction(e);
            invalidate = true;
        } else if (e->getPressState() == PickingPressState::Release &&
                   e->getPressItem() & PickingPressItem::Primary) {
            interaction_ = std::nullopt;
            invalidate = true;
        }

        e->markAsUsed();
    }
}

void Gizmo::cancelInteraction() {
    if (interaction_) {
        gizmoBasis_ = interaction_->initialBasis;
        interaction_ = std::nullopt;
        onInvalidateCallback_.invokeAll();
    }
}

void Gizmo::startInteraction(PickingEvent* e) {
    const auto id = e->getPickedId();
    const auto pickedComponent = pickingIdToComponent(id);
    const GizmoType interactionType = getGizmoInteractionType(pickedComponent);

    // initial activation with button press
    const Camera& camera = getCamera_();
    const auto viewDir{glm::normalize(camera.getDirection())};
    constexpr dmat4 worldMat{1.0};

    const auto axis = gizmoComponentToAxis(pickedComponent);
    const auto axisId = std::to_underlying(axis);
    const auto axisDir{glm::normalize(
        space_ == CoordinateSpace::World ? dvec3{worldMat[axisId]} : dvec3{gizmoBasis_[axisId]})};

    if (interactionType == GizmoType::Move) {
        const dvec3 normal{glm::cross(axisDir, glm::cross(axisDir, viewDir))};

        const Plane p{dvec3{gizmoBasis_[3]}, normal};
        auto pos = p.getIntersection(getLineFromViewDirection(camera, e->getNDC()));
        if (!pos) {
            throw Exception{"No intersection"};
        }

        interaction_ = InteractionState{
            .component = pickedComponent,
            .initialBasis = gizmoBasis_,
            .interactionPlane = p,
            .axis = Line{.point = *pos, .direction = axisDir},
            .offset = *pos - dvec3{gizmoBasis_[3]},
        };
    } else if (interactionType == GizmoType::Rotate) {
        if (pickedComponent == GizmoComponent::RotateTrackball) {
            interaction_ = InteractionState{
                .component = pickedComponent,
                .initialBasis = gizmoBasis_,
            };
        } else {
            // start trafo on selected axis, project mouse positions onto rotation axis plane
            Plane p{dvec3{gizmoBasis_[3]}, axisDir};
            if (std::abs(glm::dot(viewDir, axisDir)) < 1.0e-8) {
                // view direction and rotation axis are orthogonal, use view plane instead
                p.setNormal(-viewDir);
            }

            auto startPos = p.getIntersection(getLineFromViewDirection(camera, e->getNDC()));
            if (!startPos) {
                throw Exception{"No intersection"};
            }

            const dvec3 dir{glm::normalize(*startPos - p.getPoint())};
            interaction_ = InteractionState{
                .component = pickedComponent,
                .initialBasis = gizmoBasis_,
                .interactionPlane = p,
                .axis = Line{},
                .offset = *startPos,
                .rotationPositions = {dir, dir},
            };
        }
    }
}

void Gizmo::dragInteraction(PickingEvent* e) {
    const auto id = e->getPickedId();
    const auto pickedComponent = pickingIdToComponent(id);
    const GizmoType interactionType = getGizmoInteractionType(pickedComponent);

    // create an interaction plane in line with the selected axis pointing toward the camera
    const Plane& p = interaction_->interactionPlane;
    auto pos = p.getIntersection(getLineFromViewDirection(getCamera_(), e->getNDC()));
    if (!pos) {
        throw Exception{"No intersection"};
    }
    if (interactionType == GizmoType::Move) {
        // project position onto axis and adjust basis
        gizmoBasis_[3] = vec4{interaction_->axis.projectPoint(*pos) - interaction_->offset, 1.0f};
    } else if (interactionType == GizmoType::Rotate) {
        if (pickedComponent == GizmoComponent::RotateTrackball) {
            // based on Trackball::rotateArc()
            const Camera& camera = getCamera_();
            const dvec3 center{gizmoBasis_[3]};

            const auto rot = std::numbers::pi * 0.5 * (e->getPressedNDC() - e->getNDC());
            const auto v1 = glm::normalize(camera.getLookFrom() - center);
            const auto v2 = glm::rotate(glm::rotate(v1, rot.y, glm::cross(v1, camera.getLookUp())),
                                        rot.x, camera.getLookUp());

            const dvec4 translation{interaction_->initialBasis[3]};
            gizmoBasis_ = glm::toMat3(glm::dquat(v2, v1)) * dmat3{interaction_->initialBasis};
            gizmoBasis_[3] = translation;
        } else {
            const dvec3 v1{glm::normalize(interaction_->offset - p.getPoint())};
            const dvec3 v2{glm::normalize(*pos - p.getPoint())};
            const dvec4 translation{interaction_->initialBasis[3]};
            gizmoBasis_ = dmat3{util::rotation(v1, v2)} * dmat3{interaction_->initialBasis};
            gizmoBasis_[3] = translation;

            // update second point position
            interaction_->rotationPositions[1] = v2;
        }
    }
}

const BaseCallBack* Gizmo::onInvalidate(std::function<void()> callback) {
    return onInvalidateCallback_.addLambdaCallback(callback);
}

std::shared_ptr<std::function<void()>> Gizmo::onInvalidateScoped(std::function<void()> callback) {
    return onInvalidateCallback_.addLambdaCallbackRaii(std::move(callback));
}

void Gizmo::removeOnInvalidate(const BaseCallBack* callback) {
    onInvalidateCallback_.remove(callback);
}

namespace {

std::shared_ptr<Mesh> circle(vec3 center, vec3 normal, size_t segments) {
    const vec3 orth = util::orthvec(normal);
    const double angle = 2.0 * std::numbers::pi / static_cast<double>(segments);

    auto positions = std::views::iota(0uz, segments + 1uz) | std::views::transform([&](auto i) {
                         return center + glm::rotate(orth, static_cast<float>(i * angle), normal);
                     });
    auto indices =
        std::views::iota(0uz, segments + 3uz) | std::views::transform([segments](auto v) {
            return static_cast<std::uint32_t>(v == 0uz ? segments - 1uz : (v - 1uz) % segments);
        });

    auto mesh = std::make_shared<Mesh>();
    mesh->addBuffer(BufferType::PositionAttrib,
                    util::makeBuffer(positions | std::ranges::to<std::vector>()));
    mesh->addIndices(Mesh::MeshInfo{.dt = DrawType::Lines, .ct = ConnectivityType::StripAdjacency},
                     util::makeIndexBuffer(indices | std::ranges::to<std::vector>()));
    return mesh;
}

std::shared_ptr<Mesh> semicircle(vec3 center, vec3 normal, size_t segments) {
    const vec3 orth = util::orthvec(normal);
    const double angle = std::numbers::pi / static_cast<double>(segments);
    const size_t numPoints = segments + 1uz;

    auto positions =
        std::views::iota(0uz, numPoints) | std::views::transform([&](auto i) {
            return center + glm::rotate(orth,
                                        static_cast<float>(i * angle - std::numbers::pi * 0.5),
                                        normal);
        });
    auto indices =
        std::views::iota(0uz, numPoints + 2uz) | std::views::transform([numPoints](auto v) {
            return static_cast<std::uint32_t>(std::clamp(v, 1uz, numPoints) - 1uz);
        });

    auto mesh = std::make_shared<Mesh>();
    mesh->addBuffer(BufferType::PositionAttrib,
                    util::makeBuffer(positions | std::ranges::to<std::vector>()));
    mesh->addIndices(Mesh::MeshInfo{.dt = DrawType::Lines, .ct = ConnectivityType::StripAdjacency},
                     util::makeIndexBuffer(indices | std::ranges::to<std::vector>()));
    return mesh;
}

struct GizmoResources {
    static Shader& flatShader();
    static Shader& pointShader();
    static Shader& lineShader();

    static const MeshGL* arrowMeshGL();
    static const MeshGL* pointMeshGL();
    static const MeshGL* sphereMeshGL();
    static const MeshGL* semicircleMeshGL();
    static const MeshGL* circleMeshGL();
};

Shader& GizmoResources::flatShader() {
    static std::unique_ptr<Shader> shader;

    if (!shader) {
        shader =
            std::make_unique<Shader>("gizmohandle.vert", "gizmohandle.frag", Shader::Build::No);
        shader->getVertexShaderObject()->clearInDeclarations();
        shader->getVertexShaderObject()->addInDeclaration(
            "in_Position", static_cast<int>(BufferType::PositionAttrib), "vec3");
        shader->getVertexShaderObject()->addInDeclaration(
            "in_Normal", static_cast<int>(BufferType::NormalAttrib), "vec3");
        shader->getVertexShaderObject()->addInDeclaration(
            "in_Color", static_cast<int>(BufferType::ColorAttrib), "vec4");
        shader->build();
        shader->setLabel("GizmoShader (flat)");
    }
    return *shader;
}

Shader& GizmoResources::pointShader() {
    static std::unique_ptr<Shader> shader;

    if (!shader) {
        shader = std::make_unique<Shader>("pointrenderer.vert", "pointrenderer.geom",
                                          "pointrenderer.frag", Shader::Build::No);
        shader->getVertexShaderObject()->clearInDeclarations();
        shader->getVertexShaderObject()->addInDeclaration(
            "in_Position", static_cast<int>(BufferType::PositionAttrib), "vec3");
        shader->build();
        shader->setLabel("GizmoShader (point)");
    }
    return *shader;
}

Shader& GizmoResources::lineShader() {
    static std::unique_ptr<Shader> shader;

    if (!shader) {
        shader = std::make_unique<Shader>("linerenderer.vert", "linerenderer.geom",
                                          "linerenderer.frag", Shader::Build::No);

        shader->getGeometryShaderObject()->addShaderDefine("ENABLE_ADJACENCY", "1");
        shader->getVertexShaderObject()->clearInDeclarations();
        shader->getVertexShaderObject()->addInDeclaration(
            "in_Position", static_cast<int>(BufferType::PositionAttrib), "vec3");
        shader->build();
        shader->setLabel("GizmoShader (line)");
    }
    return *shader;
}

const MeshGL* GizmoResources::arrowMeshGL() {
    static std::shared_ptr<Mesh> mesh;
    static const MeshGL* meshGL = nullptr;

    if (!mesh) {
        mesh = meshutil::arrow(vec3{0.05f, 0.0f, 0.0f}, vec3{1.0f, 0.0f, 0.0f}, vec4{0.0f}, 0.03f,
                               0.35f, 0.09f, 64);
        meshGL = mesh->getRepresentation<MeshGL>();
    }
    return meshGL;
}

const MeshGL* GizmoResources::pointMeshGL() {
    static std::shared_ptr<Mesh> mesh;
    static const MeshGL* meshGL = nullptr;

    if (!mesh) {
        mesh = std::make_shared<Mesh>(DrawType::Points, ConnectivityType::None);
        mesh->addBuffer(BufferType::PositionAttrib, util::makeBuffer({vec3{0.0f}}));
        meshGL = mesh->getRepresentation<MeshGL>();
    }
    return meshGL;
}

const MeshGL* GizmoResources::sphereMeshGL() {
    static std::shared_ptr<Mesh> mesh;
    static const MeshGL* meshGL = nullptr;

    if (!mesh) {
        mesh = SimpleMeshCreator::sphere(0.99f, 256, 128, vec4{0.5f, 0.5f, 0.5f, 1.0f});
        meshGL = mesh->getRepresentation<MeshGL>();
    }
    return meshGL;
}

const MeshGL* GizmoResources::semicircleMeshGL() {
    static std::shared_ptr<Mesh> mesh;
    static const MeshGL* meshGL = nullptr;

    if (!mesh) {
        mesh = semicircle(vec3{0.0f}, vec3{1.0f, 0.0f, 0.0f}, 128);
        meshGL = mesh->getRepresentation<MeshGL>();
    }
    return meshGL;
}

const MeshGL* GizmoResources::circleMeshGL() {
    static std::shared_ptr<Mesh> mesh;
    static const MeshGL* meshGL = nullptr;

    if (!mesh) {
        mesh = circle(vec3{0.0f}, vec3{1.0f, 0.0f, 0.0f}, 256);
        meshGL = mesh->getRepresentation<MeshGL>();
    }
    return meshGL;
}

float getAlpha(GizmoComponent comp, const GizmoData& gizmo, const GizmoConfig& config) {
    if (gizmo.hovered == comp) {
        return 1.0f;
    }
    return config.alpha;
}

vec4 gizmoPickingColor(size_t globalPickingId, GizmoComponent comp) {
    const auto pickId = globalPickingId + std::to_underlying(comp);
    return vec4{vec3{PickingManager::indexToColor(pickId)} / 255.0f, 1.0f};
}

// transformations for ensuring that the semicircles are rendered in front of the gizmo
dmat4 orientationTransform(dvec3 axisDir, dvec3 axisNormal, dvec3 position, dvec3 viewDir) {
    if (std::abs(glm::dot(viewDir, axisDir)) > 1.0e-6f) {
        const Plane p{position, axisDir};
        const dvec3 viewDirInPlane = normalize(p.projectPoint(position - viewDir) - position);
        return util::rotation(axisNormal, viewDirInPlane);
    }
    return dmat4{1.0f};
}

void renderMoveGizmo(Shader& flatShader, const dmat4& widgetTransform, const Camera& camera,
                     const GizmoData& gizmo, const GizmoConfig& config,
                     GLenum pickingAttachmentId) {
    const auto viewDir{glm::normalize(dvec3{gizmo.basis[3]} - camera.getLookFrom())};
    constexpr dvec3 arrowDir{1.0, 0.0, 0.0};
    const auto* meshGL = GizmoResources::arrowMeshGL();
    MeshDrawerGL::DrawObject drawer{meshGL, meshGL->getDefaultMeshInfo()};

    auto activeComponents = moveComponents | std::views::filter([&](auto i) {
                                if (gizmo.state == GizmoDrawState::Interaction) {
                                    // only draw active component during interactions
                                    return gizmo.active == i;
                                }
                                const auto axisId = std::to_underlying(gizmoComponentToAxis(i));
                                const auto axisDir{normalize(dvec3{gizmo.basis[axisId]})};
                                const auto minAngle = glm::radians(10.0);
                                // draw axes only if they do not lie within 10 degrees of the view
                                // direction
                                return std::acos(std::abs(glm::dot(viewDir, axisDir))) > minAngle;
                            });

    SpatialIdentity trafo{dmat4{1.0}, widgetTransform};

    // render move gizmo into picking buffer with correct depth ordering
    glEnable(GL_DEPTH_TEST);
    glDrawBuffer(pickingAttachmentId);
    for (auto comp : activeComponents) {
        const auto axisId = std::to_underlying(gizmoComponentToAxis(comp));
        const auto axisDir{normalize(dvec3{gizmo.basis[axisId]})};

        trafo.setModelMatrix(util::rotation(arrowDir, axisDir));
        utilgl::setShaderUniforms(flatShader, trafo, "geometry");
        flatShader.setUniform("defaultColor", gizmoPickingColor(gizmo.globalPickingId, comp));
        drawer.draw();
    }

    // render color parts of move gizmo without depth test to maintain blending
    glDisable(GL_DEPTH_TEST);
    glDrawBuffer(gl::GLenum::GL_COLOR_ATTACHMENT0);
    for (auto comp : activeComponents) {
        const auto axisId = std::to_underlying(gizmoComponentToAxis(comp));
        const auto axisDir{normalize(dvec3{gizmo.basis[axisId]})};

        trafo.setModelMatrix(util::rotation(arrowDir, axisDir));
        utilgl::setShaderUniforms(flatShader, trafo, "geometry");
        flatShader.setUniform("defaultColor",
                              vec4{config.getAxisColor(axisId), getAlpha(comp, gizmo, config)});
        drawer.draw();
    }
}

void renderRotateGizmo(const dmat4& widgetTransform, const Camera& camera, const GizmoData& gizmo,
                       const GizmoConfig& config, vec2 dims, GLenum pickingAttachmentId) {
    auto& lineShader = GizmoResources::lineShader();
    const utilgl::Activate activeShader{&lineShader};

    utilgl::setShaderUniforms(lineShader, camera, "camera");
    lineShader.setUniform("screenDim", dims);
    lineShader.setUniform("roundCaps", true);
    lineShader.setUniform("defaultPickId", std::uint32_t{0});

    const dvec3 viewDir{glm::normalize(camera.getDirection())};
    constexpr dvec3 semicircleDir{1.0, 0.0, 0.0};
    constexpr dvec3 semicircleNormal{0.0, 1.0, 0.0};

    const auto* meshGL = GizmoResources::semicircleMeshGL();
    MeshDrawerGL::DrawObject drawer{meshGL, meshGL->getDefaultMeshInfo()};

    // draw rotation handles in reverse order so that x is on top
    for (auto comp : rotateComponents | std::views::reverse) {
        const auto axis = gizmoComponentToAxis(comp);
        const auto axisId = std::to_underlying(axis);
        const auto axisDir{normalize(dvec3{gizmo.basis[axisId]})};

        dmat4 trafo = util::rotation(semicircleDir, axisDir);
        dmat4 o = orientationTransform(axisDir, dmat3{trafo} * semicircleNormal,
                                       dvec3{gizmo.basis[3]}, viewDir);

        utilgl::setShaderUniforms(lineShader, SpatialIdentity{o * trafo, widgetTransform},
                                  "geometry");

        const auto pickId = gizmo.globalPickingId + std::to_underlying(comp);
        const vec4 pickingColor{vec3{PickingManager::indexToColor(pickId)} / 255.0f, 1.0f};

        lineShader.setUniform("defaultColor", pickingColor);
        lineShader.setUniform("antialiasing", 0.0f);
        lineShader.setUniform("lineWidth", config.pickingLineWidth);
        glDrawBuffer(pickingAttachmentId);
        drawer.draw();

        // draw semicircle twice with different line widths for easier picking
        lineShader.setUniform("defaultColor",
                              vec4{config.getAxisColor(axisId), getAlpha(comp, gizmo, config)});
        lineShader.setUniform("antialiasing", 0.5f);
        lineShader.setUniform("lineWidth", config.lineWidth);
        glDrawBuffer(gl::GLenum::GL_COLOR_ATTACHMENT0);
        drawer.draw();
    }
}

void renderInteractionOrigin(const Camera& camera, const GizmoData& gizmo,
                             const GizmoConfig& config, vec2 dims) {
    auto& pointShader = GizmoResources::pointShader();
    const utilgl::Activate activeShader{&pointShader};
    const utilgl::GlBoolState pointSprite(GL_PROGRAM_POINT_SIZE, true);

    utilgl::setShaderUniforms(pointShader, camera, "camera");
    pointShader.setUniform(
        "config.color",
        config.getAxisColor(std::to_underlying(gizmoComponentToAxis(gizmo.active))));
    pointShader.setUniform("config.radius", config.lineWidth * 1.4f);
    pointShader.setUniform("viewport", vec4{0.0f, 0.0f, 2.0f / dims.x, 2.0f / dims.y});
    utilgl::setShaderUniforms(pointShader, SpatialIdentity{glm::translate(gizmo.origin)},
                              "geometry");

    const auto* meshGL = GizmoResources::pointMeshGL();
    MeshDrawerGL::DrawObject drawer{meshGL, meshGL->getDefaultMeshInfo()};
    drawer.draw();
}

void renderRotationInteraction(const dmat4& widgetTransform, const Camera& camera,
                               const GizmoData& gizmo, const GizmoConfig& config, vec2 dims) {
    const auto axis = gizmoComponentToAxis(gizmo.active);
    if (axis == GizmoAxis::Invalid) return;

    const auto axisId = std::to_underlying(axis);
    const auto axisDir{normalize(dvec3{gizmo.basis[axisId]})};

    // rotate cicle mesh into place for currently selected axis
    const dmat4 m = util::rotation(dvec3{1.0, 0.0, 0.0}, axisDir);
    SpatialIdentity trafo{m, widgetTransform};

    {
        auto& lineShader = GizmoResources::lineShader();
        const utilgl::Activate activeShader{&lineShader};

        utilgl::setShaderUniforms(lineShader, camera, "camera");
        lineShader.setUniform("screenDim", dims);
        lineShader.setUniform("roundCaps", false);

        utilgl::setShaderUniforms(lineShader, trafo, "geometry");
        lineShader.setUniform("defaultColor", vec4{config.getAxisColor(axisId), 1.0f});
        lineShader.setUniform("antialiasing", 0.5f);
        lineShader.setUniform("lineWidth", config.lineWidth);

        const auto* meshGL = GizmoResources::circleMeshGL();
        MeshDrawerGL::DrawObject drawer{meshGL, meshGL->getDefaultMeshInfo()};
        drawer.draw();
    }

    if (gizmo.rotationIndicators) {
        auto& pointShader = GizmoResources::pointShader();
        const utilgl::Activate activeShader{&pointShader};
        const utilgl::GlBoolState pointSprite(GL_PROGRAM_POINT_SIZE, true);

        utilgl::setShaderUniforms(pointShader, camera, "camera");

        pointShader.setUniform("config.color", config.getAxisColor(axisId));
        pointShader.setUniform("config.radius", config.lineWidth * 1.4f);
        pointShader.setUniform("viewport", vec4{0.0f, 0.0f, 2.0f / dims.x, 2.0f / dims.y});

        const auto* meshGL = GizmoResources::pointMeshGL();
        MeshDrawerGL::DrawObject drawer{meshGL, meshGL->getDefaultMeshInfo()};

        trafo.setModelMatrix(glm::translate((*gizmo.rotationIndicators)[0]));
        utilgl::setShaderUniforms(pointShader, trafo, "geometry");
        drawer.draw();

        trafo.setModelMatrix(glm::translate((*gizmo.rotationIndicators)[1]));
        utilgl::setShaderUniforms(pointShader, trafo, "geometry");
        drawer.draw();
    }
}

}  // namespace

namespace gizmoutil {

double gizmoScale(const Camera& camera, size2_t screenDim, int pixelWidgetSize,
                  const dmat4& gizmoBasis) {
    const SpatialIdentity spatialEntity{};
    const auto& coordTransform{spatialEntity.getCoordinateTransformer(camera)};

    const dvec3 center{gizmoBasis[3]};
    const auto centerClip = util::transformPos(coordTransform.getWorldToClipMatrix(), center);

    return glm::distance(
        center, util::transformPos(coordTransform.getClipToWorldMatrix(),
                                   centerClip + dvec3{0.0,
                                                      static_cast<double>(pixelWidgetSize) /
                                                          static_cast<double>(screenDim.y),
                                                      0.0}));
}

dmat4 gizmoScaleTransform(const Camera& camera, size2_t screenDim, int pixelWidgetSize,
                          const dmat4& gizmoBasis) {
    const auto size = gizmoScale(camera, screenDim, pixelWidgetSize, gizmoBasis);
    return glm::scale(glm::translate(dvec3{gizmoBasis[3]}), dvec3{size});
}

void renderGizmo(const ImageOutport& outport, const Camera& camera, const GizmoData& gizmo,
                 const GizmoConfig& config) {

    if (gizmo.state == GizmoDrawState::Hidden) return;

    const bool interacting = gizmo.state == GizmoDrawState::Interaction;
    const bool showMove = gizmo.type == GizmoType::Move || gizmo.type == GizmoType::MoveRotate;
    const bool showRotate = gizmo.type == GizmoType::Rotate || gizmo.type == GizmoType::MoveRotate;

    const auto widgetTransform{gizmoutil::gizmoScaleTransform(camera, outport.getDimensions(),
                                                              config.widgetSize, gizmo.basis)};
    const vec2 dims{outport.getDimensions()};
    auto& destImage = *outport.getEditableData()->getEditableRepresentation<ImageGL>();

    const utilgl::CullFaceState cull{gl::GLenum::GL_BACK};
    const utilgl::GlBoolState depthTest{GL_DEPTH_TEST, false};

    auto& shader = GizmoResources::flatShader();
    const utilgl::Activate activeFlatShader{&shader};
    utilgl::setShaderUniforms(shader, camera, "camera");

    // trackball for picking in the background
    if (showRotate && config.trackballVisible) {
        shader.setUniform("defaultColor", gizmoPickingColor(gizmo.globalPickingId,
                                                            GizmoComponent::RotateTrackball));
        utilgl::setShaderUniforms(shader, SpatialIdentity{dmat4{1.0}, widgetTransform}, "geometry");

        const auto* meshGL = GizmoResources::sphereMeshGL();
        MeshDrawerGL::DrawObject drawer{meshGL, meshGL->getDefaultMeshInfo()};
        glDrawBuffer(destImage.getPickingAttachmentID());
        drawer.draw();
    }

    // render move gizmo
    if (showMove) {
        renderMoveGizmo(shader, widgetTransform, camera, gizmo, config,
                        destImage.getPickingAttachmentID());
    }

    // render visible part of trackball only when hovered
    if (!interacting && showRotate && config.trackballVisible &&
        gizmo.hovered == GizmoComponent::RotateTrackball) {
        shader.setUniform("defaultColor", gizmo.hovered == GizmoComponent::RotateTrackball
                                              ? config.neutralColor
                                              : vec4{0.0f});
        utilgl::setShaderUniforms(shader, SpatialIdentity{dmat4{1.0}, widgetTransform}, "geometry");

        const auto* meshGL = GizmoResources::sphereMeshGL();
        MeshDrawerGL::DrawObject drawer{meshGL, meshGL->getDefaultMeshInfo()};
        glDrawBuffer(gl::GLenum::GL_COLOR_ATTACHMENT0);
        drawer.draw();
    }

    // render the initial position of move interactions and trackball rotation
    if (std::ranges::contains(moveComponents, gizmo.active) ||
        gizmo.active == GizmoComponent::RotateTrackball) {
        renderInteractionOrigin(camera, gizmo, config, dims);
    } else if (std::ranges::contains(rotateComponents, gizmo.active)) {
        renderRotationInteraction(widgetTransform, camera, gizmo, config, dims);
    }

    if (!interacting && showRotate) {
        renderRotateGizmo(widgetTransform, camera, gizmo, config, dims,
                          destImage.getPickingAttachmentID());
    }

    // reset draw buffers
    const auto drawBuffers = destImage.getDrawBuffers(ImageType::ColorPicking);
    glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
}

}  // namespace gizmoutil

}  // namespace inviwo
