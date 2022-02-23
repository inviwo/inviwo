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

    if (supportedFilters & FilterType::Rows) {
        {
            auto emptyLines =
                std::make_unique<BoolCompositeProperty>("emptyLines", "Empty Lines", true);
            emptyLines->addProperty(onHeaderProp());

            addPrefab(std::move(emptyLines));
        }
        {
            auto rowBegin = std::make_unique<BoolCompositeProperty>("rowBegin", "Row Begin", true);
            rowBegin->addProperty(onHeaderProp());
            rowBegin->addProperty(std::make_unique<StringProperty>("match", "Matching String", ""));

            addPrefab(std::move(rowBegin));
        }
        {
            auto lineRange =
                std::make_unique<BoolCompositeProperty>("lineRange", "Line Range", true);
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

    if (supportedFilters & FilterType::StringItem) {
        auto stringItem =
            std::make_unique<BoolCompositeProperty>("stringItem", "String Match", true);
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
    if (supportedFilters & FilterType::IntItem) {
        auto intItem =
            std::make_unique<BoolCompositeProperty>("intItem", "Integer Comparison", true);
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
        intItem->addProperty(std::make_unique<IntProperty>(
            "value", "Value", 0, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(),
            1, InvalidationLevel::InvalidOutput, PropertySemantics::Text));

        addPrefab(std::move(intItem));
    }
    if (supportedFilters & FilterType::FloatItem) {
        auto floatItem =
            std::make_unique<BoolCompositeProperty>("floatItem", "Float Comparison", true);
        floatItem->addProperty(columnProp());
        floatItem->addProperty(std::make_unique<TemplateOptionProperty<filters::NumberComp>>(
            "comp", "Comparison",
            std::vector<OptionPropertyOption<filters::NumberComp>>{
                {"equal", "equal (==)", filters::NumberComp::Equal},
                {"notEqual", "Not Equal (!=)", filters::NumberComp::NotEqual},
                {"less", "Less (<)", filters::NumberComp::Less},
                {"lessEqual", "Less Equal (<=)", filters::NumberComp::LessEqual},
                {"greater", "greater (>)", filters::NumberComp::Greater},
                {"greaterEqual", "Greater Equal (>=)", filters::NumberComp::GreaterEqual}},
            0));
        floatItem->addProperty(std::make_unique<FloatProperty>(
            "value", "Value", 0.0f, std::numeric_limits<float>::lowest(),
            std::numeric_limits<float>::max(), 0.1f, InvalidationLevel::InvalidOutput,
            PropertySemantics::Text));
        floatItem->addProperty(std::make_unique<FloatProperty>(
            "epsilon", "Epsilon", 0.0f, 0.0f, std::numeric_limits<float>::max(), 0.1f,
            InvalidationLevel::InvalidOutput, PropertySemantics::Text));

        addPrefab(std::move(floatItem));
    }
    if (supportedFilters & FilterType::DoubleItem) {
        auto doubleItem =
            std::make_unique<BoolCompositeProperty>("doubleItem", "Double Comparison", true);
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
    if (supportedFilters & FilterType::IntRange) {
        auto intRange = std::make_unique<BoolCompositeProperty>("intRangeItem", "Int Range", true);
        intRange->addProperty(columnProp());
        intRange->addProperty(std::make_unique<IntMinMaxProperty>(
            "range", "Integer Range", 0, 100, std::numeric_limits<int>::min(),
            std::numeric_limits<int>::max(), 1, 0, InvalidationLevel::InvalidOutput,
            PropertySemantics::Text));

        addPrefab(std::move(intRange));
    }
    if (supportedFilters & FilterType::FloatRange) {
        auto floatRange =
            std::make_unique<BoolCompositeProperty>("floatRangeItem", "Float Range", true);
        floatRange->addProperty(columnProp());
        floatRange->addProperty(std::make_unique<FloatMinMaxProperty>(
            "range", "Float Range", 0.0f, 100.0f, std::numeric_limits<float>::lowest(),
            std::numeric_limits<float>::max(), 0.1f, 0.0f, InvalidationLevel::InvalidOutput,
            PropertySemantics::Text));

        addPrefab(std::move(floatRange));
    }
    if (supportedFilters & FilterType::DoubleRange) {
        auto doubleRange =
            std::make_unique<BoolCompositeProperty>("doubleRangeItem", "Double Range", true);
        doubleRange->addProperty(columnProp());
        doubleRange->addProperty(std::make_unique<DoubleMinMaxProperty>(
            "range", "Double Range", 0.0, 100.0, std::numeric_limits<double>::min(),
            std::numeric_limits<double>::max(), 0.1, 0.0, InvalidationLevel::InvalidOutput,
            PropertySemantics::Text));

        addPrefab(std::move(doubleRange));
    }
}

FilterListProperty* FilterListProperty::clone() const { return new FilterListProperty(*this); }

}  // namespace inviwo
