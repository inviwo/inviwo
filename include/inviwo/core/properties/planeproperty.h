/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#ifndef IVW_PLANEPROPERTY_H
#define IVW_PLANEPROPERTY_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/templateproperty.h>
#include <inviwo/core/properties/boolcompositeproperty.h>

namespace inviwo {

/**
 * \ingroup properties
 *
 * \brief A Property that represents a plane
 *
 * A Plane is represented by a plane normal and a point that is in the plane.
 * The property also holds a color for drawing the plane. A bool for turning it on an and off.
 */
class IVW_CORE_API PlaneProperty : public BoolCompositeProperty {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    PlaneProperty(const std::string& identifier, const std::string& displayName,
                  InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                  PropertySemantics semantics = PropertySemantics::Default);
    PlaneProperty(const std::string& identifier, const std::string& displayName, vec3 position,
                  vec3 normal = vec3(0.0f, 0.0f, 1.0f), vec4 color = vec4(1.0f),
                  InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                  PropertySemantics semantics = PropertySemantics::Default);

    PlaneProperty(const PlaneProperty& rhs);
    virtual PlaneProperty* clone() const override;
    virtual ~PlaneProperty();

    FloatVec3Property position_;
    FloatVec3Property normal_;
    FloatVec4Property color_;
};

}  // namespace inviwo

#endif  // IVW_PLANEPROPERTY_H
