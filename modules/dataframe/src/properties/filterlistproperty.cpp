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

#include <inviwo/dataframe/properties/filterlistproperty.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/optionproperty.h>

#include <inviwo/dataframe/util/filters.h>

#include <limits>
#include <regex>
#include <functional>
#include <algorithm>

namespace inviwo {

const std::string FilterListProperty::classIdentifier = "org.inviwo.FilterListProperty";
std::string FilterListProperty::getClassIdentifier() const { return classIdentifier; }

FilterListProperty::FilterListProperty(std::string_view identifier, std::string_view displayName,
                                       bool supportsFilterOnHeader, FilterTypes supportedFilters,
                                       size_t maxNumberOfElements, ListPropertyUIFlags uiFlags,
                                       InvalidationLevel invalidationLevel,
                                       PropertySemantics semantics)
    : ListProperty(identifier, displayName, maxNumberOfElements, uiFlags, invalidationLevel,
                   semantics) {

    auto onHeaderProp = [enable = supportsFilterOnHeader]() {
        auto p = std::make_unique<BoolProperty>("filterOnHeader", "Filter on Header", enable);
        p->setVisible(enable);
        return p;
    };
    auto createBoolComposite = [](std::string_view identifier, std::string_view displayName) {
        auto p = std::make_unique<BoolCompositeProperty>(identifier, displayName, true);
        p->getBoolProperty()->setSerializationMode(PropertySerializationMode::All);
        return p;
    };

    if (supportedFilters.contains(FilterType::Rows)) {
        {
            auto emptyLines = createBoolComposite("emptyLines", "Empty Lines");
            emptyLines->addProperty(onHeaderProp());

            addPrefab(std::move(emptyLines));
        }
        {
            auto rowBegin = createBoolComposite("rowBegin", "Row Begin");
            rowBegin->addProperty(onHeaderProp());
            rowBegin->addProperty(std::make_unique<StringProperty>("match", "Matching String", ""));

            addPrefab(std::move(rowBegin));
        }
        {
            auto lineRange = createBoolComposite("lineRange", "Line Range");
            lineRange->addProperty(onHeaderProp());
            lineRange->addProperty(std::make_unique<IntMinMaxProperty>(
                "range", "Line Range", 0, 100, 0, std::numeric_limits<int>::max(), 1, 0,
                InvalidationLevel::InvalidOutput, PropertySemantics::Text));

            addPrefab(std::move(lineRange));
        }
    }

    auto columnProp = []() {
        return std::make_unique<IntProperty>(
            "column", "Column", 0, 0, std::numeric_limits<int>::max(), 1,
            InvalidationLevel::InvalidOutput, PropertySemantics::Text);
    };

    if (supportedFilters.contains(FilterType::StringItem)) {
        auto stringItem = createBoolComposite("stringItem", "String Match");
        stringItem->addProperty(columnProp());
        stringItem->addProperty(std::make_unique<TemplateOptionProperty<filters::StringComp>>(
            "comp", "Comparison",
            std::vector<OptionPropertyOption<filters::StringComp>>{
                {"equal", "equal (==)", filters::StringComp::Equal},
                {"notEqual", "Not Equal (!=)", filters::StringComp::NotEqual},
                {"regex", "Regex", filters::StringComp::Regex},
                {"regexPartial", "Regex (Partial)", filters::StringComp::RegexPartial}},
            0));
        stringItem->addProperty(std::make_unique<StringProperty>("match", "Matching String", ""));

        addPrefab(std::move(stringItem));
    }
    if (supportedFilters.contains(FilterType::IntItem)) {
        auto intItem = createBoolComposite("intItem", "Integer Comparison");
        intItem->addProperty(columnProp());
        intItem->addProperty(std::make_unique<TemplateOptionProperty<filters::NumberComp>>(
            "comp", "Comparison",
            std::vector<OptionPropertyOption<filters::NumberComp>>{
                {"equal", "equal (==)", filters::NumberComp::Equal},
                {"notEqual", "Not Equal (!=)", filters::NumberComp::NotEqual},
                {"less", "Less (<)", filters::NumberComp::Less},
                {"lessEqual", "Less Equal (<=)", filters::NumberComp::LessEqual},
                {"greater", "greater (>)", filters::NumberComp::Greater},
                {"greaterEqual", "Greater Equal (>=)", filters::NumberComp::GreaterEqual}},
            0));
        intItem->addProperty(std::make_unique<Int64Property>(
            "value", "Value", 0, std::numeric_limits<int64_t>::min(),
            std::numeric_limits<int64_t>::max(), 1, InvalidationLevel::InvalidOutput,
            PropertySemantics::Text));

        addPrefab(std::move(intItem));
    }
    if (supportedFilters.contains(FilterType::DoubleItem)) {
        auto doubleItem = createBoolComposite("doubleItem", "Double Comparison");
        doubleItem->addProperty(columnProp());
        doubleItem->addProperty(std::make_unique<TemplateOptionProperty<filters::NumberComp>>(
            "comp", "Comparison",
            std::vector<OptionPropertyOption<filters::NumberComp>>{
                {"equal", "equal (==)", filters::NumberComp::Equal},
                {"notEqual", "Not Equal (!=)", filters::NumberComp::NotEqual},
                {"less", "Less (<)", filters::NumberComp::Less},
                {"lessEqual", "Less Equal (<=)", filters::NumberComp::LessEqual},
                {"greater", "greater (>)", filters::NumberComp::Greater},
                {"greaterEqual", "Greater Equal (>=)", filters::NumberComp::GreaterEqual}},
            0));
        doubleItem->addProperty(std::make_unique<DoubleProperty>(
            "value", "Value", 0.0, std::numeric_limits<double>::lowest(),
            std::numeric_limits<double>::max(), 0.1, InvalidationLevel::InvalidOutput,
            PropertySemantics::Text));
        doubleItem->addProperty(std::make_unique<DoubleProperty>(
            "epsilon", "Epsilon", 0.0, 0.0, std::numeric_limits<double>::max(), 0.1,
            InvalidationLevel::InvalidOutput, PropertySemantics::Text));

        addPrefab(std::move(doubleItem));
    }
    if (supportedFilters.contains(FilterType::IntRange)) {
        auto intRange = createBoolComposite("intRangeItem", "Int Range");
        intRange->addProperty(columnProp());
        intRange->addProperty(std::make_unique<Int64MinMaxProperty>(
            "range", "Integer Range", 0, 100, std::numeric_limits<int64_t>::min(),
            std::numeric_limits<int64_t>::max(), 1, 0, InvalidationLevel::InvalidOutput,
            PropertySemantics::Text));

        addPrefab(std::move(intRange));
    }
    if (supportedFilters.contains(FilterType::DoubleRange)) {
        auto doubleRange = createBoolComposite("doubleRangeItem", "Double Range");
        doubleRange->addProperty(columnProp());
        doubleRange->addProperty(std::make_unique<DoubleMinMaxProperty>(
            "range", "Double Range", 0.0, 100.0, std::numeric_limits<double>::lowest(),
            std::numeric_limits<double>::max(), 0.1, 0.0, InvalidationLevel::InvalidOutput,
            PropertySemantics::Text));

        addPrefab(std::move(doubleRange));
    }
}

FilterListProperty* FilterListProperty::clone() const { return new FilterListProperty(*this); }

}  // namespace inviwo
