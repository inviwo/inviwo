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
#pragma once

#include <modules/basegl/baseglmoduledefine.h>

#include <inviwo/core/datastructures/datatraits.h>
#include <inviwo/core/datastructures/spatialdata.h>
#include <inviwo/core/datastructures/geometry/plane.h>
#include <inviwo/core/interaction/pickingmapper.h>
#include <inviwo/core/util/callback.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/glmmat.h>

#include <modules/basegl/interaction/gizmoconfig.h>

#include <glm/geometric.hpp>
#include <glm/gtx/transform.hpp>

#include <string_view>
#include <numbers>
#include <cstdint>
#include <optional>
#include <functional>

namespace inviwo {

class Shader;
class Camera;
class ImageOutport;
class Processor;
class Event;
class Camera;
class PickingEvent;
class MeshGL;
class Mesh;

enum class GizmoType : std::uint8_t { None, Move, Rotate, MoveRotate };
enum class GizmoDrawState : std::uint8_t { Hidden, Visible, Interaction };
enum class GizmoComponent : std::uint8_t {
    MoveX,
    MoveY,
    MoveZ,
    RotateX,
    RotateY,
    RotateZ,
    RotateTrackball,
    None
};
enum class GizmoAxis : std::uint8_t { X, Y, Z, Invalid };

constexpr std::string_view format_as(GizmoType type) {
    switch (type) {
        using enum GizmoType;
        case None:       return "None";
        case Move:       return "Move";
        case Rotate:     return "Rotate";
        case MoveRotate: return "Move & Rotate";
    }
    throw Exception(SourceContext{}, "Invalid GizmoType {}", std::to_underlying(type));
}

constexpr std::string_view format_as(GizmoDrawState state) {
    switch (state) {
        using enum GizmoDrawState;
        case Hidden:      return "Hidden";
        case Visible:     return "Visible";
        case Interaction: return "Interaction";
    }
    throw Exception(SourceContext{}, "Invalid GizmoDrawState {}", std::to_underlying(state));
}

constexpr std::string_view format_as(GizmoComponent component) {
    switch (component) {
        using enum GizmoComponent;
        case MoveX:           return "MoveX";
        case MoveY:           return "MoveY";
        case MoveZ:           return "MoveZ";
        case RotateX:         return "RotateX";
        case RotateY:         return "RotateY";
        case RotateZ:         return "RotateZ";
        case RotateTrackball: return "RotateTrackball";
        case None:            return "None";
    }
    throw Exception(SourceContext{}, "Invalid GizmoComponent {}", std::to_underlying(component));
}

constexpr std::string_view format_as(GizmoAxis axis) {
    switch (axis) {
        using enum GizmoAxis;
        case X:       return "X";
        case Y:       return "Y";
        case Z:       return "Z";
        case Invalid: return "Invalid";
    }
    throw Exception(SourceContext{}, "Invalid GizmoAxis {}", std::to_underlying(axis));
}

struct IVW_MODULE_BASEGL_API GizmoData {
    dmat4 basis{1.0};   // gizmo transform in world coordinates
    dvec3 origin{0.0};  // initial basis offset before interaction started
    GizmoType type = GizmoType::None;
    GizmoDrawState state = GizmoDrawState::Hidden;
    GizmoComponent hovered = GizmoComponent::None;
    GizmoComponent active = GizmoComponent::None;
    std::uint32_t globalPickingId = 0u;
    std::optional<std::array<dvec3, 2>> rotationIndicators = std::nullopt;
};

class IVW_MODULE_BASEGL_API GizmoResources {
public:
    Shader& flatShader() const;
    Shader& pointShader() const;
    Shader& lineShader() const;

    const MeshGL* arrowMeshGL() const;
    const MeshGL* pointMeshGL() const;
    const MeshGL* sphereMeshGL() const;
    const MeshGL* semicircleMeshGL() const;
    const MeshGL* circleMeshGL() const;

private:
    mutable std::shared_ptr<Shader> flatShader_{};
    mutable std::shared_ptr<Shader> pointShader_{};
    mutable std::shared_ptr<Shader> lineShader_{};
    mutable std::shared_ptr<Mesh> arrowMesh_{};
    mutable std::shared_ptr<Mesh> pointMesh_{};
    mutable std::shared_ptr<Mesh> sphereMesh_{};
    mutable std::shared_ptr<Mesh> semicircleMesh_{};
    mutable std::shared_ptr<Mesh> circleMesh_{};
};

IVW_MODULE_BASEGL_API std::string format_as(const GizmoData& data);

template <>
struct DataTraits<GizmoData> {
    static constexpr std::string_view classIdentifier() { return "org.inviwo.GizmoData"; }
    static constexpr std::string_view dataName() { return "GizmoData"; }
    static constexpr uvec3 colorCode() { return {170, 172, 180}; }
    IVW_MODULE_BASEGL_API static Document info(const GizmoData& data);
};

class IVW_MODULE_BASEGL_API Gizmo {
public:
    using CameraCallback = std::function<const Camera&()>;

    Gizmo(Processor* processor, CameraCallback getCamera);

    void setType(GizmoType type);
    GizmoType getType() const;

    /**
     * Set gizmo basis to @p basis in world coordinates. Has no effect during any interaction.
     */
    void setBasis(dmat4 basis);
    /**
     * Set gizmo basis to @p basis given in @p space coordinates. Uses the coordinate transformer of
     * the spatial entity @p entity. Has no effect during any interaction.
     */
    void setBasis(dmat4 basis, const SpatialEntity& entity,
                  CoordinateSpace space = CoordinateSpace::World);
    /**
     * Current gizmo basis given in world coordinates
     * @return basis in world coordinates
     */
    dmat4 getBasis() const;
    dmat4 getBasis(CoordinateSpace space, const SpatialEntity& entity) const;

    CoordinateSpace getSpace() const;

    GizmoData getData() const;

    void cancelInteraction();

    const BaseCallBack* onInvalidate(std::function<void()> callback);
    std::shared_ptr<std::function<void()>> onInvalidateScoped(std::function<void()> callback);
    void removeOnInvalidate(const BaseCallBack* callback);

private:
    void picking(PickingEvent* e);
    void startInteraction(PickingEvent* e);
    void dragInteraction(PickingEvent* e);

    CoordinateSpace space_;
    GizmoType type_;
    dmat4 gizmoBasis_;  // always in world coordinates

    PickingMapper picking_;

    struct InteractionState {
        GizmoComponent component;
        dmat4 initialBasis{1.0};
        Plane interactionPlane{};
        Line axis{};
        dvec3 offset{0.0};
        std::array<dvec3, 2> rotationPositions{{dvec3{0.0}, dvec3{0.0}}};
    };

    std::optional<GizmoComponent> highlight_;
    std::optional<InteractionState> interaction_;

    CallBackList onInvalidateCallback_;
    CameraCallback getCamera_;
};

namespace gizmoutil {

IVW_MODULE_BASEGL_API double gizmoScale(const Camera& camera, size2_t screenDim,
                                        int pixelWidgetSize, const dmat4& gizmoBasis);

/**
 * Compute the transformation matrix for scaling a gizmo to a given pixel size in screen space at
 * the offset position in basis @p gizmoBasis.
 *
 * @return gizmo scale transformation
 */
IVW_MODULE_BASEGL_API dmat4 gizmoScaleTransform(const Camera& camera, size2_t screenDim,
                                                int pixelWidgetSize, const dmat4& gizmoBasis);

IVW_MODULE_BASEGL_API void renderGizmo(const ImageOutport& outport, const Camera& camera,
                                       const GizmoData& gizmo, const GizmoConfig& config,
                                       const GizmoResources& resources = {});

}  // namespace gizmoutil

}  // namespace inviwo
