/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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
#include <inviwo/core/properties/listproperty.h>

#include <flags/flags.h>

namespace inviwo {

enum class FilterType { Rows, StringItem, IntItem, DoubleItem, IntRange, DoubleRange };

ALLOW_FLAGS_FOR_ENUM(FilterType)
using FilterTypes = flags::flags<FilterType>;

/**
 * @brief A list property with different sub-properties for defining row and item filters.
 *
 * This property has a number of prefab objects for adding row and item filter properties
 * (BoolCompositeProperty). A row filter is applied to the entire row of a dataset. Item filters, in
 * contrast, are applied to the individual data items. The distinction between row and item filters
 * is based on the property identifiers.
 *
 * Identifiers of the properties for row filters begin with the following names and hold the listed
 * sub-properties:
 *  - @c emptyLines: Matches empty lines
 *  - @c rowBegin:   StringProperty @c match for matching the beginning of a row
 *  - @c lineRange:  IntMinMaxProperty @c range for a line range (inclusive)
 *
 * Row filter properties also own a BoolProperty @c filterOnHeader. If true, the filter should be
 * applied to all rows including headers where applicable (for example in the CSVReader).
 *
 * Item filters own an IntProperty @c column defining the target column. Identifiers begin with:
 *  - @c stringItem:      StringProperty @c match for matching strings and a
 *                        OptionProperty<filters::StringComp> @c comp for the comparison
 *  - @c intItem:         Int64Property @c value, OptionProperty<filters::NumberComp>
 *                        @c comp for the comparison
 *  - @c doubleItem:      DoubleProperty @c value and DoubleProperty @c epsilon,
 *                        OptionProperty<filters::NumberComp> @c comp for the comparison
 *  - @c intRangeItem:    Int64MinMaxProperty @c range for an inclusive range [min, max]
 *  - @c doubleRangeItem: DoubleMinMaxProperty @c range for an inclusive range [min, max]
 *
 * @see CSVReader
 */
class IVW_MODULE_DATAFRAME_API FilterListProperty : public ListProperty {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    FilterListProperty(std::string_view identifier, std::string_view displayName,
                       bool supportsFilterOnHeader = false,
                       FilterTypes supportedFilters = FilterTypes(flags::any),
                       size_t maxNumberOfElements = 0,
                       ListPropertyUIFlags uiFlags = ListPropertyUIFlag::Add |
                                                     ListPropertyUIFlag::Remove,
                       InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                       PropertySemantics semantics = PropertySemantics::Default);
    virtual ~FilterListProperty() = default;

    virtual FilterListProperty* clone() const override;
};

}  // namespace inviwo
