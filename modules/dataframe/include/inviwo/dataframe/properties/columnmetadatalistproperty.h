/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2023 Inviwo Foundation
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

#include <inviwo/dataframe/dataframemoduledefine.h>  // for IVW_MODULE_DATAFRAME_API

#include <inviwo/core/properties/invalidationlevel.h>   // for InvalidationLevel, InvalidationLe...
#include <inviwo/core/properties/listproperty.h>        // for ListProperty
#include <inviwo/core/properties/property.h>            // for OverwriteState
#include <inviwo/core/properties/propertysemantics.h>   // for PropertySemantics, PropertySemant...
#include <inviwo/dataframe/datastructures/dataframe.h>  // for DataFrameInport, DataFrame

#include <cstddef>      // for size_t
#include <functional>   // for function
#include <memory>       // for shared_ptr
#include <string>       // for string
#include <string_view>  // for string_view

namespace inviwo {
class ColumnMetaDataProperty;

/**
 * \ingroup properties
 * List and override column metadata of a DataFrame
 */
class IVW_MODULE_DATAFRAME_API ColumnMetaDataListProperty : public ListProperty {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    ColumnMetaDataListProperty(
        std::string_view identifier, std::string_view displayName,
        InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
        PropertySemantics semantics = PropertySemantics::Default);
    ColumnMetaDataListProperty(
        std::string_view identifier, std::string_view displayName, DataFrameInport& port,
        InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
        PropertySemantics semantics = PropertySemantics::Default);

    ColumnMetaDataListProperty(const ColumnMetaDataListProperty& rhs);
    virtual ColumnMetaDataListProperty* clone() const override;

    virtual ~ColumnMetaDataListProperty() = default;

    /**
     * Populate the property with the DataFrame columns owned by \p port. The onChange event of \p
     * port will trigger updating the column metadata.
     *
     * @note A reference to the port will be kept, so the port must outlive the scope of the
     * property.
     */
    void setPort(DataFrameInport& inport);

    void updateForNewDataFrame(const DataFrame& dataFrame, util::OverwriteState overwrite);

    void updateDataFrame(DataFrame& dataFrame) const;

private:
    ColumnMetaDataProperty& meta(size_t i);
    const ColumnMetaDataProperty& meta(size_t i) const;

    DataFrameInport* inport_ = nullptr;
    std::shared_ptr<std::function<void()>> onChangeCallback_;
};

}  // namespace inviwo
