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

#include <inviwo/dataframe/processors/dataframefilter.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/stringproperty.h>

#include <inviwo/dataframe/util/dataframeutil.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo DataFrameFilter::processorInfo_{
    "org.inviwo.DataFrameFilter",                 // Class identifier
    "DataFrame Filter",                           // Display name
    "DataFrame",                                  // Category
    CodeState::Experimental,                      // Code state
    Tag::CPU | Tag("DataFrame") | Tag("Filter"),  // Tags
};
const ProcessorInfo DataFrameFilter::getProcessorInfo() const { return processorInfo_; }

DataFrameFilter::DataFrameFilter()
    : Processor()
    , inport_("inport")
    , brushing_("brushing", {{{BrushingTarget::Row},
                              BrushingModification::Filtered,
                              InvalidationLevel::InvalidOutput}})
    , outport_("outport")
    , enabled_("enabled", "Enabled", true)
    , brushingMode_("brushingMode", "Brushing and Linking",
                    {{"none", "None", BrushingMode::None},
                     {"filterOnly", "Filter Only", BrushingMode::FilterOnly},
                     {"applyOnly", "Apply Only", BrushingMode::ApplyOnly},
                     {"filterApply", "Filter & Apply", BrushingMode::FilterApply}},
                    1)
    , includeFilters_("includeFilters", "Include Filters", true,
                      FilterType::StringItem | FilterType::IntItem | FilterType::IntRange |
                          FilterType::DoubleItem | FilterType::DoubleRange)
    , excludeFilters_("excludeFilters", "Exclude Filters", true,
                      FilterType::StringItem | FilterType::IntItem | FilterType::IntRange |
                          FilterType::DoubleItem | FilterType::DoubleRange) {

    addPort(inport_);
    addPort(brushing_);
    addPort(outport_);

    addProperties(enabled_, brushingMode_, includeFilters_, excludeFilters_);

    includeFilters_.setCollapsed(true);
    excludeFilters_.setCollapsed(true);

    // make this processor a sink if brushing and linking filtering is selected
    isSink_.setUpdate([&]() {
        return (brushingMode_ == BrushingMode::FilterOnly ||
                brushingMode_ == BrushingMode::FilterApply);
    });
    brushingMode_.onChange([&]() { isSink_.update(); });
}

void DataFrameFilter::process() {
    auto df = inport_.getData();

    if (!enabled_) {
        outport_.setData(df);
        brushing_.filter(getIdentifier(), BitSet());
        return;
    }

    if (brushingMode_.isModified()) {
        if ((brushingMode_ == BrushingMode::None) || (brushingMode_ == BrushingMode::FilterOnly)) {
            // unset any previous local filtering
            brushing_.filter(getIdentifier(), BitSet());
        }
    }

    auto rows = dataframe::selectRows(*df, createFilters());

    if ((brushingMode_ == BrushingMode::ApplyOnly) ||
        (brushingMode_ == BrushingMode::FilterApply)) {
        BitSet b(rows);
        b.flipRange(0, static_cast<std::uint32_t>(inport_.getData()->getNumberOfRows()));
        brushing_.filter(getIdentifier(), b);
    }

    if ((brushingMode_ == BrushingMode::FilterOnly) ||
        (brushingMode_ == BrushingMode::FilterApply)) {
        BitSet b(rows);
        b -= brushing_.getFilteredIndices();
        rows = b.toVector();
    }

    outport_.setData(std::make_shared<DataFrame>(*df, rows));
}

namespace detail {

template <typename T>
decltype(std::declval<T&>().get()) getValue(PropertyOwner* parent, std::string_view identifier) {
    if (auto p = dynamic_cast<T*>(parent->getPropertyByIdentifier(identifier))) {
        return p->get();
    } else {
        throw Exception(IVW_CONTEXT_CUSTOM("DataFrameFilter::createFilters"),
                        "Invalid filter property '{}', missing sub-property '{}'",
                        parent->getIdentifier(), identifier);
    }
}

}  // namespace detail

dataframefilters::Filters DataFrameFilter::createFilters() const {
    auto startsWith = [](std::string_view str, std::string_view prefix) {
        return str.substr(0, prefix.size()) == prefix;
    };

    auto parse = [&](const auto& filterProp, auto& itemFilters) {
        for (auto prop : filterProp) {
            if (auto cp = dynamic_cast<BoolCompositeProperty*>(prop)) {
                if (!cp->isChecked()) continue;

                const auto& identifier = cp->getIdentifier();

                if (startsWith(identifier, "stringItem")) {
                    itemFilters.push_back(dataframefilters::stringMatch(
                        detail::getValue<IntProperty>(cp, "column"),
                        detail::getValue<TemplateOptionProperty<filters::StringComp>>(cp, "comp"),
                        detail::getValue<StringProperty>(cp, "match")));
                } else if (startsWith(identifier, "intItem")) {
                    itemFilters.push_back(dataframefilters::intMatch(
                        detail::getValue<IntProperty>(cp, "column"),
                        detail::getValue<TemplateOptionProperty<filters::NumberComp>>(cp, "comp"),
                        detail::getValue<Int64Property>(cp, "value")));
                } else if (startsWith(identifier, "doubleItem")) {
                    itemFilters.push_back(dataframefilters::doubleMatch(
                        detail::getValue<IntProperty>(cp, "column"),
                        detail::getValue<TemplateOptionProperty<filters::NumberComp>>(cp, "comp"),
                        detail::getValue<DoubleProperty>(cp, "value"),
                        detail::getValue<DoubleProperty>(cp, "epsilon")));
                } else if (startsWith(identifier, "intRangeItem")) {
                    const auto& range = detail::getValue<Int64MinMaxProperty>(cp, "range");
                    itemFilters.push_back(dataframefilters::intRange(
                        detail::getValue<IntProperty>(cp, "column"), range.x, range.y));
                } else if (startsWith(identifier, "doubleRangeItem")) {
                    const auto& range = detail::getValue<DoubleMinMaxProperty>(cp, "range");
                    itemFilters.push_back(dataframefilters::doubleRange(
                        detail::getValue<IntProperty>(cp, "column"), range.x, range.y));
                }
            }
        }
    };

    dataframefilters::Filters filters;
    parse(includeFilters_, filters.include);
    parse(excludeFilters_, filters.exclude);

    return filters;
}

}  // namespace inviwo
