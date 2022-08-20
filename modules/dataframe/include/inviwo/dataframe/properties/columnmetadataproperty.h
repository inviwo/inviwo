/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2022 Inviwo Foundation
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

#include <inviwo/dataframe/dataframemoduledefine.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/valuewrapper.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/dataframe/datastructures/dataframe.h>

#include <string_view>

namespace inviwo {

/**
 * \ingroup properties
 * A property for accessing and overriding column-specific metadata of a DataFrame
 */
class IVW_MODULE_DATAFRAME_API ColumnMetaDataProperty : public BoolCompositeProperty {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    ColumnMetaDataProperty(
        std::string_view identifier, std::string_view displayName, dvec2 range = {0.0, 1.0},
        InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
        PropertySemantics semantics = PropertySemantics::Default);
    ColumnMetaDataProperty(const ColumnMetaDataProperty& rhs);
    ColumnMetaDataProperty& operator=(const ColumnMetaDataProperty& rhs);

    virtual ColumnMetaDataProperty* clone() const override;

    virtual ~ColumnMetaDataProperty() = default;

    const std::string& getHeader() const;
    dvec2 getRange() const;
    Unit getUnit() const;
    bool getDrop() const;
    std::string getType() const;

    void updateForNewColumn(const Column& col, util::OverwriteState overwrite);
    void updateColumn(Column& col) const;

protected:
    StringProperty header_;
    StringProperty type_;
    DoubleMinMaxProperty range_;
    StringProperty unit_;
    BoolProperty drop_;
};

}  // namespace inviwo
