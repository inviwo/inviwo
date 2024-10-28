/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2024 Inviwo Foundation
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

#include <modules/base/basemoduledefine.h>  // for IVW_MODULE_BASE_API

#include <inviwo/core/properties/compositeproperty.h>  // for CompositeProperty
#include <inviwo/core/properties/invalidationlevel.h>  // for InvalidationLevel, InvalidationLev...
#include <inviwo/core/properties/ordinalproperty.h>    // for DoubleProperty, IntSize2Property
#include <inviwo/core/properties/propertysemantics.h>  // for PropertySemantics, PropertySemanti...
#include <inviwo/core/properties/stringproperty.h>     // for StringProperty

#include <string>       // for string
#include <string_view>  // for string_view
#include <tuple>        // for tie

namespace inviwo {

class Image;

/**
 * \ingroup properties
 * \brief A CompositeProperty holding properties to show a information about an image
 */
class IVW_MODULE_BASE_API ImageInformationProperty : public CompositeProperty {
public:
    virtual std::string_view getClassIdentifier() const override;
    static const std::string classIdentifier;

    ImageInformationProperty(
        std::string_view identifier, std::string_view displayName,
        InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
        PropertySemantics semantics = PropertySemantics::Default);
    ImageInformationProperty(const ImageInformationProperty& rhs);
    virtual ImageInformationProperty* clone() const override;
    virtual ~ImageInformationProperty() = default;

    void updateForNewImage(const Image& image);

    // Read-only used to show information
    IntSize2Property dimensions_;
    DoubleProperty aspectRatio_;
    StringProperty imageType_;
    IntSizeTProperty numColorLayers_;
    CompositeProperty layers_;

private:
    auto props() { return std::tie(dimensions_, aspectRatio_, imageType_, numColorLayers_); }
    auto props() const { return std::tie(dimensions_, aspectRatio_, imageType_, numColorLayers_); }
};

}  // namespace inviwo
