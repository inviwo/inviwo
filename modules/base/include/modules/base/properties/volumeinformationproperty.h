/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2026 Inviwo Foundation
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

#include <modules/base/basemoduledefine.h>

#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/invalidationlevel.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/propertysemantics.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/stringsproperty.h>
#include <inviwo/core/datastructures/image/imagetypes.h>

#include <array>
#include <string>
#include <string_view>

namespace inviwo {
class Volume;

/**
 * @ingroup properties
 * A CompositeProperty holding properties to show a information about a volume
 */
class IVW_MODULE_BASE_API VolumeInformationProperty : public BoolCompositeProperty {
public:
    virtual std::string_view getClassIdentifier() const override;
    static constexpr std::string_view classIdentifier{"org.inviwo.VolumeInformationProperty"};
    VolumeInformationProperty(
        std::string_view identifier, std::string_view displayName,
        InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
        PropertySemantics semantics = PropertySemantics::Default);
    VolumeInformationProperty(const VolumeInformationProperty& rhs);
    virtual VolumeInformationProperty* clone() const override;
    virtual ~VolumeInformationProperty() = default;

    void updateForNewVolume(const Volume& volume, util::OverwriteState overwrite);
    void updateVolume(Volume& volume);
    // Read only used to show information

    IntSize3Property dimensions;
    StringProperty format;
    IntSizeTProperty channels;
    IntSizeTProperty numVoxels;

    // read / write
    DoubleMinMaxProperty dataRange;
    DoubleMinMaxProperty valueRange;
    StringProperty valueName;
    StringProperty valueUnit;

    OptionProperty<InterpolationType> interpolation;

    StringsProperty<3> axesNames;
    StringsProperty<3> axesUnits;

    std::array<OptionProperty<Wrapping>, 3> wrapping;
};

}  // namespace inviwo
