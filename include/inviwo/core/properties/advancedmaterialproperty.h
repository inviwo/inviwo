/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#ifndef IVW_ADVANCED_MATERIAL_PROPERTY_H
#define IVW_ADVANCED_MATERIAL_PROPERTY_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/compositeproperty.h>

namespace inviwo {

enum class ShadingFunctionKind {
    HenyeyGreenstein = 0,
    Schlick,
    BlinnPhong,
    Ward,
    CookTorrance,
    AbcMicrofacet,
    Ashikhmin,
    Mix,
    Isotropic,
    None
};

/** \class AdvancedMaterialProperty
 *
 * Adds advanced shading properties to a PropertyOwner (Processor).
 * @see ShadingFunctionEnum
 */
class IVW_CORE_API AdvancedMaterialProperty: public CompositeProperty {
public:
    InviwoPropertyInfo();
    AdvancedMaterialProperty(std::string identifier, std::string displayName,
                             InvalidationLevel=InvalidationLevel::InvalidResources,
                             PropertySemantics semantics = PropertySemantics::Default);
    
    AdvancedMaterialProperty(const AdvancedMaterialProperty& rhs);
    AdvancedMaterialProperty& operator=(const AdvancedMaterialProperty& that);
    virtual AdvancedMaterialProperty* clone() const override;
    virtual ~AdvancedMaterialProperty();

    vec4 getCombinedMaterialParameters() const;
    ShadingFunctionKind getPhaseFunctionEnum() const;

    virtual void phaseFunctionChanged();
    virtual void deserialize(Deserializer& d) override;


    // Material properties
    OptionPropertyString phaseFunctionProp;
    FloatProperty indexOfRefractionProp;
    FloatProperty roughnessProp;
    FloatVec4Property specularColorProp;
    FloatProperty anisotropyProp;
protected:
    float HenyehGreensteinToSchlick(float g) const;

    float roughnessToShininess(float m) const;

    float WardParameterMapping(float x) const;

    vec2 BCParameterMapping(const vec2& bc) const;

    /**
     * Compute index of refraction (IOR) f0 = ((n1-n2)/(n1+n2))^2
     *
     * @param n1 First material
     * @param n2 Second material
     * @return Index of refraction ((n1-n2)/(n1+n2))^2
     */
    float getIndexOfRefractionTerm(float n1, float n2) const;
};

} // namespace

#endif // IVW_ADVANCED_MATERIAL_PROPERTY_H
