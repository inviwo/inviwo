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

namespace detail {

template <typename Prop, typename... Args>
void addProperty(PropertyOwner& owner, std::string_view identifier, std::string_view displayName,
                 Args&&... args) {
    auto prop = std::make_unique<Prop>(identifier, displayName);
    prop->set(std::forward<Args>(args)...);
    prop->setSemantics(PropertySemantics::Text);
    owner.addProperty(std::move(prop));
}

template <typename Prop>
void addOptionProperty(PropertyOwner& owner, std::string_view identifier,
                       std::string_view displayName,
                       std::vector<OptionPropertyOption<typename Prop::value_type>> options) {
    auto prop = std::make_unique<Prop>(identifier, displayName);
    prop->replaceOptions(options);
    owner.addProperty(std::move(prop));
}

}  // namespace detail

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
        auto p = std::make_unique<BoolCompositeProperty>(identifier, displayName);
        p->getBoolProperty()->set(true);
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
            detail::addProperty<IntMinMaxProperty>(*lineRange, "range", "Line Range", 0, 100, 0,
                                                   std::numeric_limits<int>::max(), 1, 0);

            addPrefab(std::move(lineRange));
        }
    }

    if (supportedFilters.contains(FilterType::StringItem)) {
        auto stringItem = createBoolComposite("stringItem", "String Match");
        detail::addProperty<IntProperty>(*stringItem, "column", "Column", 0, 0,
                                         std::numeric_limits<int>::max(), 1);

        detail::addOptionProperty<TemplateOptionProperty<filters::StringComp>>(
            *stringItem, "comp", "Comparison",
            std::vector<OptionPropertyOption<filters::StringComp>>{
                {"equal", "equal (==)", filters::StringComp::Equal},
                {"notEqual", "Not Equal (!=)", filters::StringComp::NotEqual},
                {"regex", "Regex", filters::StringComp::Regex},
                {"regexPartial", "Regex (Partial)", filters::StringComp::RegexPartial}});
        stringItem->addProperty(std::make_unique<StringProperty>("match", "Matching String", ""));

        addPrefab(std::move(stringItem));
    }
    if (supportedFilters.contains(FilterType::IntItem)) {
        auto intItem = createBoolComposite("intItem", "Integer Comparison");
        detail::addProperty<IntProperty>(*intItem, "column", "Column", 0, 0,
                                         std::numeric_limits<int>::max(), 1);

        detail::addOptionProperty<TemplateOptionProperty<filters::NumberComp>>(
            *intItem, "comp", "Comparison",
            std::vector<OptionPropertyOption<filters::NumberComp>>{
                {"equal", "equal (==)", filters::NumberComp::Equal},
                {"notEqual", "Not Equal (!=)", filters::NumberComp::NotEqual},
                {"less", "Less (<)", filters::NumberComp::Less},
                {"lessEqual", "Less Equal (<=)", filters::NumberComp::LessEqual},
                {"greater", "greater (>)", filters::NumberComp::Greater},
                {"greaterEqual", "Greater Equal (>=)", filters::NumberComp::GreaterEqual}});
        detail::addProperty<Int64Property>(*intItem, "value", "Value", 0,
                                           std::numeric_limits<int64_t>::min(),
                                           std::numeric_limits<int64_t>::max(), 1);

        addPrefab(std::move(intItem));
    }
    if (supportedFilters.contains(FilterType::DoubleItem)) {
        auto doubleItem = createBoolComposite("doubleItem", "Double Comparison");
        detail::addProperty<IntProperty>(*doubleItem, "column", "Column", 0, 0,
                                         std::numeric_limits<int>::max(), 1);

        detail::addOptionProperty<TemplateOptionProperty<filters::NumberComp>>(
            *doubleItem, "comp", "Comparison",
            std::vector<OptionPropertyOption<filters::NumberComp>>{
                {"equal", "equal (==)", filters::NumberComp::Equal},
                {"notEqual", "Not Equal (!=)", filters::NumberComp::NotEqual},
                {"less", "Less (<)", filters::NumberComp::Less},
                {"lessEqual", "Less Equal (<=)", filters::NumberComp::LessEqual},
                {"greater", "greater (>)", filters::NumberComp::Greater},
                {"greaterEqual", "Greater Equal (>=)", filters::NumberComp::GreaterEqual}});
        detail::addProperty<DoubleProperty>(*doubleItem, "value", "Value", 0.0,
                                            std::numeric_limits<double>::lowest(),
                                            std::numeric_limits<double>::max(), 0.1);
        detail::addProperty<DoubleProperty>(*doubleItem, "epsilon", "Epsilon", 0.0, 0.0,
                                            std::numeric_limits<double>::max(), 0.1);

        addPrefab(std::move(doubleItem));
    }
    if (supportedFilters.contains(FilterType::IntRange)) {
        auto intRange = createBoolComposite("intRangeItem", "Int Range");
        detail::addProperty<IntProperty>(*intRange, "column", "Column", 0, 0,
                                         std::numeric_limits<int>::max(), 1);

        detail::addProperty<Int64MinMaxProperty>(*intRange, "range", "Integer Range", 0, 100,
                                                 std::numeric_limits<int64_t>::min(),
                                                 std::numeric_limits<int64_t>::max(), 1, 0);

        addPrefab(std::move(intRange));
    }
    if (supportedFilters.contains(FilterType::DoubleRange)) {
        auto doubleRange = createBoolComposite("doubleRangeItem", "Double Range");
        detail::addProperty<IntProperty>(*doubleRange, "column", "Column", 0, 0,
                                         std::numeric_limits<int>::max(), 1);

        detail::addProperty<DoubleMinMaxProperty>(*doubleRange, "range", "Double Range", 0.0, 100.0,
                                                  std::numeric_limits<double>::lowest(),
                                                  std::numeric_limits<double>::max(), 0.1, 0.0);

        addPrefab(std::move(doubleRange));
    }
}

FilterListProperty* FilterListProperty::clone() const { return new FilterListProperty(*this); }

}  // namespace inviwo
