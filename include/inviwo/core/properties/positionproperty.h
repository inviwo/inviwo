/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2024 Inviwo Foundation
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

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/ordinalrefproperty.h>
#include <inviwo/core/datastructures/coordinatetransformer.h>

#include <optional>

namespace inviwo {

/**
 * \ingroup properties
 *
 * \brief Enables Edit a position in world or view space.
 *
 * The default value of the position is the origin (0, 0, 0).
 * @note View space is disabled if a camera is not supplied.
 */
class IVW_CORE_API PositionProperty : public CompositeProperty {
public:
    virtual std::string_view getClassIdentifier() const override;
    static const std::string classIdentifier;

    enum class CoordinateOffset { None, CameraLookAt, Custom };
    enum class ApplyOffset { No, Yes };

    PositionProperty(std::string_view identifier, std::string_view displayName, Document help,
                     const vec3& position = vec3{0.0f},
                     CoordinateSpace coordinateSpace = CoordinateSpace::World,
                     CameraProperty* camera = nullptr,
                     PropertySemantics positionSemantics = PropertySemantics::Default,
                     InvalidationLevel = InvalidationLevel::InvalidResources);

    PositionProperty(std::string_view identifier, std::string_view displayName,
                     const vec3& position = vec3{0.0f},
                     CoordinateSpace coordinateSpace = CoordinateSpace::World,
                     CameraProperty* camera = nullptr,
                     PropertySemantics positionSemantics = PropertySemantics::Default,
                     InvalidationLevel = InvalidationLevel::InvalidResources);
    PositionProperty(const PositionProperty& rhs);
    [[nodiscard]] virtual PositionProperty* clone() const override;

    /**
     * Get the current position in the given coordinate space \p space.
     * @param space    coordinate system of the returned position
     * @return position in \p space
     */
    [[nodiscard]] vec3 get(CoordinateSpace space = CoordinateSpace::World) const;

    using CompositeProperty::set;  // Enable calling CompositeProperty::set(...) functions even
                                   // though overriding with set(const vec3& value)
    /**
     * Set both position \p pos given in the coordinate space \p space and the coordinate space.
     * @param pos    position in \p space coordinates
     * @param space  reference coordinate system of \p pos
     * @param applyOffset   determines whether the current offset is applied to \p pos or \p pos is
     *               considered absolute
     */
    void set(const vec3& pos, CoordinateSpace space, ApplyOffset applyOffset = ApplyOffset::No);

    /**
     * Update the position given \p pos in coordinate space \p sourceSpace. The position will be
     * transformed from \p sourceSpace to the currently set reference coordinate space of the
     * property.
     * @param pos    position in \p space coordinates
     * @param sourceSpace  coordinate space of \p pos
     */
    void updatePosition(const vec3& pos, CoordinateSpace sourceSpace);

    /**
     * Get the offset currently applied to the position in the given coordinate space \p space.
     * @param space    coordinate system of the returned offset
     * @return offset in \p space
     */
    vec3 getOffset(CoordinateSpace space) const;

    [[nodiscard]] CoordinateSpace getCoordinateSpace() const;

    /**
     * Determines the normalized direction from offset to the current position, that is
     * <tt>pos - offset</tt>, in the given coordinate space \p space.
     * @return normalized, directional vector in \p space coordinates
     */
    [[nodiscard]] vec3 getDirection(CoordinateSpace space) const;

    /**
     * Set the property semantics of the position property to \p semantics
     */
    void setPositionSemantics(PropertySemantics semantics);

    virtual void deserialize(Deserializer& d) override;

private:
    [[nodiscard]] vec3 convert(const vec3& pos, CoordinateSpace sourceSpace,
                               CoordinateSpace targetSpace) const;
    [[nodiscard]] vec3 convertFromWorld(const vec3& pos, CoordinateSpace targetSpace) const;
    [[nodiscard]] vec3 convertToWorld(const vec3& pos, CoordinateSpace sourceSpace) const;
    void invalidateOutputProperties();
    void referenceSpaceChanged();

    CameraProperty* camera_;  //< Non-owning reference.
    std::optional<CoordinateSpace> previousReferenceSpace_;

    OptionProperty<CoordinateSpace> referenceSpace_;
    FloatVec3Property position_;

    OptionProperty<CoordinateOffset> coordinateOffsetMode_;
    FloatVec3Property coordinateOffset_;

    CompositeProperty output_;
    FloatVec3RefProperty worldPos_;
    FloatVec3RefProperty viewPos_;
    FloatVec3RefProperty clipPos_;
    FloatVec3RefProperty screenPos_;
};

}  // namespace inviwo
