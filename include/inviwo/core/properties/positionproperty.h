/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#ifndef IVW_POSITIONPROPERTY_H
#define IVW_POSITIONPROPERTY_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>

namespace inviwo {

/**
 * \class PositionProperty
 *
 * \brief Enables Edit a position in world or view space.
 *
 * The default value of the position is the origin (0, 0, 0).
 * @note View space is disabled if a camera is not supplied.
 */
class IVW_CORE_API PositionProperty : public CompositeProperty {
public:
    InviwoPropertyInfo();
    enum class Space : int { WORLD, VIEW };

    PositionProperty(std::string identifier, std::string displayName
        , FloatVec3Property position = FloatVec3Property("position", "Position", vec3(0.0f, 0.0f, 0.0f), vec3(-10, -10, -10),
        vec3(10, 10, 10))
        , CameraProperty* camera = nullptr
        , InvalidationLevel = InvalidationLevel::InvalidResources
        , PropertySemantics semantics = PropertySemantics::Default);
    PositionProperty(const PositionProperty& rhs);
    PositionProperty& operator=(const PositionProperty& that);
    virtual PositionProperty* clone() const override;
    virtual ~PositionProperty(){}

    /** 
     * \brief Get position in world space.
     * @return vec3 World space position.
     */
    const vec3& get() const;

    using CompositeProperty::set; // Enable calling CompositeProperty::set(...) functions even though overriding with set(const vec3& value)
    /** 
     * \brief Set coordinate in world space.
     * @param const vec3 & worldSpacePos Position in world space.
     */
    void set(const vec3& worldSpacePos);

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    OptionPropertyInt referenceFrame_; //< The space in which the position is specified (world or view).
    FloatVec3Property position_; //< Position in specified space (world or view).
private:
    void referenceFrameChanged();
    void positionChanged();
    void cameraChanged();

    vec3 positionWorldSpace_; //< Used for always keeping track of the current position in world space.
    CameraProperty* camera_; //< Non-owning reference.
};

} // namespace

#endif // IVW_POSITIONPROPERTY_H

