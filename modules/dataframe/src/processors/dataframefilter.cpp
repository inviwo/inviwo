/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2025 Inviwo Foundation
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

#include <inviwo/core/datastructures/bitset.h>                         // for BitSet
#include <inviwo/core/processors/processor.h>                          // for Processor
#include <inviwo/core/processors/processorinfo.h>                      // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                     // for CodeState, CodeSta...
#include <inviwo/core/processors/processortags.h>                      // for Tag, Tag::CPU, Tags
#include <inviwo/core/properties/boolcompositeproperty.h>              // for BoolCompositeProperty
#include <inviwo/core/properties/boolproperty.h>                       // for BoolProperty
#include <inviwo/core/properties/invalidationlevel.h>                  // for InvalidationLevel
#include <inviwo/core/properties/minmaxproperty.h>                     // for DoubleMinMaxProperty
#include <inviwo/core/properties/optionproperty.h>                     // for OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>                    // for IntProperty, Doubl...
#include <inviwo/core/properties/propertyowner.h>                      // for PropertyOwner
#include <inviwo/core/properties/stringproperty.h>                     // for StringProperty
#include <inviwo/core/util/exception.h>                                // for Exception
#include <inviwo/core/util/sourcecontext.h>                            // for SourceContext
#include <inviwo/core/util/statecoordinator.h>                         // for StateCoordinator
#include <inviwo/core/util/staticstring.h>                             // for operator+
#include <inviwo/dataframe/datastructures/dataframe.h>                 // for DataFrame, DataFra...
#include <inviwo/dataframe/properties/filterlistproperty.h>            // for FilterType, Filter...
#include <inviwo/dataframe/util/dataframeutil.h>                       // for selectRows
#include <inviwo/dataframe/util/filters.h>                             // for Filters, doubleMatch
#include <modules/brushingandlinking/brushingandlinkingmanager.h>      // for BrushingTargetsInv...
#include <modules/brushingandlinking/datastructures/brushingaction.h>  // for BrushingTarget
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>  // for BrushingAndLinking...

#include <cstdint>      // for uint32_t
#include <memory>       // for make_shared, share...
#include <type_traits>  // for enable_if<>::type
#include <utility>      // for declval

#include <flags/flags.h>  // for operator|, flags
#include <glm/vec2.hpp>   // for vec, vec<>::(anony...

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo DataFrameFilter::processorInfo_{
    "org.inviwo.DataFrameFilter",                  // Class identifier
    "DataFrame Filter",                            // Display name
    "DataFrame",                                   // Category
    CodeState::Experimental,                       // Code state
    Tags::CPU | Tag("DataFrame") | Tag("Filter"),  // Tags
    R"(Creates a new DataFrame by filtering the input DataFrame using either filters or Brushing and
    Linking. Filtered rows are optionally propagated using Brushing and Linking.)"_unindentHelp,
};
const ProcessorInfo& DataFrameFilter::getProcessorInfo() const { return processorInfo_; }

DataFrameFilter::DataFrameFilter()
    : Processor()
    , inport_("inport", "filters are applied to this source DataFrame"_help)
    , brushing_("brushing", "inport for brushing & linking filtering"_help,
                {{{BrushingTarget::Row},
                  BrushingModification::Filtered,
                  InvalidationLevel::InvalidOutput}})
    , outport_("outport", "filtered DataFrame"_help)
    , enabled_("enabled", "Enabled", true)
    , brushingMode_("brushingMode", "Brushing and Linking",
                    R"(Determines how Brushing and Linking is considered when filtering.
        - **None**         no brushing and linking
        - **FilterOnly**   filtered rows are propagated to B&L, B&L filter state is not considered
        - **ApplyOnly**    apply B&L filter state to DataFrame, no brushing actions are sent
        - **FilterApply**  propagate filtered rows and apply B&L filter state)"_unindentHelp,
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

    // Make this processor a sink if the brushing mode is set to filtering. Otherwise brushing
    // actions will not be triggered as long as the outport of this processor is not connected.
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
        throw Exception(SourceContext{}, "Invalid filter property '{}', missing sub-property '{}'",
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
                        detail::getValue<OptionProperty<filters::StringComp>>(cp, "comp"),
                        detail::getValue<StringProperty>(cp, "match")));
                } else if (startsWith(identifier, "intItem")) {
                    itemFilters.push_back(dataframefilters::intMatch(
                        detail::getValue<IntProperty>(cp, "column"),
                        detail::getValue<OptionProperty<filters::NumberComp>>(cp, "comp"),
                        detail::getValue<Int64Property>(cp, "value")));
                } else if (startsWith(identifier, "doubleItem")) {
                    itemFilters.push_back(dataframefilters::doubleMatch(
                        detail::getValue<IntProperty>(cp, "column"),
                        detail::getValue<OptionProperty<filters::NumberComp>>(cp, "comp"),
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
