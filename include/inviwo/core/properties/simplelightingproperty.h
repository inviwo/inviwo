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

#ifndef IVW_SIMPLELIGHTINGPROPERTY_H
#define IVW_SIMPLELIGHTINGPROPERTY_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/templateproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/optionproperty.h>

namespace inviwo {

/** \class SimpleLightingProperty
 * Add simple light properties, position, ambient, diffuse, specular color. Used for phong shading.
 */

namespace ShadingMode {
    enum Modes {
        None,
        Ambient,
        Diffuse,
        Specular,
        BlinnPhong,
        Phong,
    };
}



class CameraProperty;

class IVW_CORE_API SimpleLightingProperty : public CompositeProperty {
public:
    InviwoPropertyInfo();

    enum class Space : int {WORLD, VIEW};


    SimpleLightingProperty(std::string identifier, std::string displayName,
                           CameraProperty* camera = nullptr,
                           InvalidationLevel = INVALID_RESOURCES,
                           PropertySemantics semantics = PropertySemantics::Default);
    
    SimpleLightingProperty(const SimpleLightingProperty& rhs);
    SimpleLightingProperty& operator=(const SimpleLightingProperty& that);
    virtual SimpleLightingProperty* clone() const;
    virtual ~SimpleLightingProperty();
    
    // Light properties
    OptionPropertyInt shadingMode_;
    OptionPropertyInt referenceFrame_;
    FloatVec3Property lightPosition_;
    FloatVec3Property lightAttenuation_;
    BoolProperty applyLightAttenuation_;

    // Material properties
    // Diffuse color often come from the object 
    // so we neglect it here
    FloatVec3Property ambientColor_;
    FloatVec3Property diffuseColor_; 
    FloatVec3Property specularColor_;
    FloatProperty specularExponent_;
    
    vec3 getTransformedPosition() const;

private:
    CameraProperty* camera_; //< Non-owning reference.
};

} // namespace

#endif // IVW_SIMPLELIGHTINGPROPERTY_H

