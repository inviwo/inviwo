/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2023 Inviwo Foundation
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

#include <modules/basegl/baseglmoduledefine.h>  // for IVW_MODULE_BASEGL_API

#include <inviwo/core/properties/compositeproperty.h>                  // for CompositeProperty
#include <inviwo/core/properties/invalidationlevel.h>                  // for InvalidationLevel
#include <inviwo/core/properties/optionproperty.h>                     // for OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>                    // for FloatProperty
#include <inviwo/core/properties/propertysemantics.h>                  // for PropertySemantics
#include <inviwo/core/util/staticstring.h>                             // for operator+
#include <modules/basegl/datastructures/stipplingsettingsinterface.h>  // for StipplingSettingsI...

#include <functional>   // for __base
#include <string>       // for operator==, string
#include <string_view>  // for operator==, string...
#include <vector>       // for operator!=, vector

namespace inviwo {
class Shader;

class IVW_MODULE_BASEGL_API StipplingProperty : public CompositeProperty,
                                                public StipplingSettingsInterface {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    StipplingProperty(std::string_view identifier, std::string_view displayName,
                      InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                      PropertySemantics semantics = PropertySemantics::Default);
    StipplingProperty(const StipplingProperty& rhs);
    virtual StipplingProperty* clone() const override;
    virtual ~StipplingProperty() = default;

    // StipplingSettingsInterface
    virtual Mode getMode() const override { return mode_.get(); };
    virtual float getLength() const override { return length_.get(); };
    virtual float getSpacing() const override { return spacing_.get(); };
    virtual float getOffset() const override { return offset_.get(); };
    virtual float getWorldScale() const override { return worldScale_.get(); };

    OptionProperty<StipplingSettingsInterface::Mode> mode_;
    FloatProperty length_;
    FloatProperty spacing_;
    FloatProperty offset_;
    FloatProperty worldScale_;
};

namespace utilgl {

IVW_MODULE_BASEGL_API void addShaderDefines(Shader& shader, const StipplingProperty& property);
IVW_MODULE_BASEGL_API void addShaderDefines(Shader& shader, const StipplingProperty::Mode& mode);
IVW_MODULE_BASEGL_API void setShaderUniforms(Shader& shader, const StipplingProperty& property,
                                             const std::string& name);

}  // namespace utilgl

}  // namespace inviwo
