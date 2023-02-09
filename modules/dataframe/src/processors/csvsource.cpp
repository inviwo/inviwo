/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2022 Inviwo Foundation
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

#include <inviwo/dataframe/processors/csvsource.h>

#include <inviwo/core/ports/dataoutport.h>                           // for DataOutport
#include <inviwo/core/ports/outportiterable.h>                       // for OutportIterableImpl<...
#include <inviwo/core/processors/processor.h>                        // for Processor
#include <inviwo/core/processors/processorinfo.h>                    // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                   // for CodeState, CodeState...
#include <inviwo/core/processors/processortags.h>                    // for Tags
#include <inviwo/core/properties/boolcompositeproperty.h>            // for BoolCompositeProperty
#include <inviwo/core/properties/boolproperty.h>                     // for BoolProperty
#include <inviwo/core/properties/fileproperty.h>                     // for FileProperty
#include <inviwo/core/properties/minmaxproperty.h>                   // for DoubleMinMaxProperty
#include <inviwo/core/properties/optionproperty.h>                   // for OptionProperty, Opti...
#include <inviwo/core/properties/ordinalproperty.h>                  // for IntProperty, DoubleP...
#include <inviwo/core/properties/property.h>                         // for Property, OverwriteS...
#include <inviwo/core/properties/propertyowner.h>                    // for PropertyOwner
#include <inviwo/core/properties/stringproperty.h>                   // for StringProperty
#include <inviwo/core/util/exception.h>                              // for Exception
#include <inviwo/core/util/filesystem.h>                             // for fileExists
#include <inviwo/core/util/logcentral.h>                             // for LogCentral, LogProce...
#include <inviwo/core/util/sourcecontext.h>                          // for IVW_CONTEXT_CUSTOM
#include <inviwo/core/util/statecoordinator.h>                       // for StateCoordinator
#include <inviwo/core/util/staticstring.h>                           // for operator+
#include <inviwo/core/util/stdextensions.h>                          // for ref, any_of
#include <inviwo/dataframe/datastructures/dataframe.h>               // for DataFrame
#include <inviwo/dataframe/io/csvreader.h>                           // for CSVReader::EmptyField
#include <inviwo/dataframe/properties/columnmetadatalistproperty.h>  // for ColumnMetaDataListPr...
#include <inviwo/dataframe/properties/filterlistproperty.h>          // for FilterListProperty
#include <inviwo/dataframe/util/filters.h>                           // for RowFilter, Filters

#include <array>        // for array
#include <type_traits>  // for enable_if<>::type
#include <utility>      // for declval

#include <flags/flags.h>  // for operator|, flags
#include <fmt/core.h>     // for format
#include <glm/vec2.hpp>   // for vec, vec<>::(anonymous)

namespace inviwo {
class Deserializer;

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo CSVSource::processorInfo_{
    "org.inviwo.CSVSource",                   // Class identifier
    "CSV Source",                             // Display name
    "Data Input",                             // Category
    CodeState::Stable,                        // Code state
    "CPU, Plotting, Source, CSV, DataFrame",  // Tags
};
const ProcessorInfo CSVSource::getProcessorInfo() const { return processorInfo_; }

CSVSource::CSVSource(const std::string& file)
    : Processor{}
    , data_{"data"}
    , inputFile_{"inputFile_", "CSV File", file, "dataframe"}
    , firstRowIsHeaders_{"firstRowIsHeaders", "First Row Contains Column Headers",
                         CSVReader::defaultFirstRowHeader}
    , firstColumnIsIndices_{"firstColumnIsIndices", "First Column Contains Indices",
                            CSVReader::defaultFirstColIndices}
    , unitsInHeaders_{"unitsInHeaders", "Look for units in headers",
                      CSVReader::defaultUnitInHeaders}
    , unitRegexp_{"unitRegexp", "Unit regexp", CSVReader::defaultUnitRegexp}
    , delimiters_{"delimiters", "Delimiters", CSVReader::defaultDelimiters}
    , stripQuotes_{"stripQuotes", "Strip surrounding quotes", CSVReader::defaultStripQuotes}
    , doublePrecision_{"doublePrecision", "Double Precision", CSVReader::defaultDoublePrecision}
    , exampleRows_{"exampleRows", "Example Rows", CSVReader::defaultNumberOfExampleRows, 0, 10000}
    , rowComment_{"rowComment", "Comment marker", ""}
    , includeFilters_{"includeFilters", "Include Filters", true,
                      FilterType::Rows | FilterType::StringItem | FilterType::DoubleItem |
                          FilterType::DoubleRange}
    , excludeFilters_{"excludeFilters", "Exclude Filters", true}
    , locale_{"locale", "Locale", CSVReader::defaultLocale}
    , emptyField_{"emptyField",
                  "Missing Data Mode",
                  {{"throw", "Throw Exception", CSVReader::EmptyField::Throw},
                   {"emptyOrZero", "Empty Or Zero", CSVReader::EmptyField::EmptyOrZero},
                   {"nanOrZero", "Nan Or Zero", CSVReader::EmptyField::NanOrZero}}}
    , reloadData_{"reloadData", "Reload Data"}
    , columns_{"columns", "Column MetaData"}
    , loadingFailed_{false}
    , deserialized_{false} {

    emptyField_.setSelectedValue(CSVReader::defaultEmptyField);
    emptyField_.setCurrentStateAsDefault();

    addPort(data_);

    unitsInHeaders_.addProperty(unitRegexp_);
    addProperties(inputFile_, firstRowIsHeaders_, firstColumnIsIndices_, unitsInHeaders_,
                  delimiters_, stripQuotes_, doublePrecision_, exampleRows_, rowComment_, locale_,
                  emptyField_, reloadData_, includeFilters_, excludeFilters_, columns_);

    includeFilters_.setCollapsed(true);
    excludeFilters_.setCollapsed(true);

    isReady_.setUpdate(
        [this]() { return !loadingFailed_ && filesystem::fileExists(inputFile_.get()); });
    for (auto&& item :
         util::ref<Property>(inputFile_, reloadData_, delimiters_, stripQuotes_, firstRowIsHeaders_,
                             firstColumnIsIndices_, unitsInHeaders_, unitRegexp_, doublePrecision_,
                             exampleRows_, rowComment_, locale_, emptyField_)) {
        std::invoke(&Property::onChange, item, [this]() {
            loadingFailed_ = false;
            isReady_.update();
        });
    }

    // make sure that we always process even if not connected
    isSink_.setUpdate([]() { return true; });
}

void CSVSource::process() {
    if (inputFile_.get().empty()) {
        data_.clear();
        return;
    }

    try {
        const auto overwrite = deserialized_ ? util::OverwriteState::No : util::OverwriteState::Yes;
        deserialized_ = false;

        if (util::any_of(
                util::ref<Property>(inputFile_, reloadData_, delimiters_, stripQuotes_,
                                    firstRowIsHeaders_, firstColumnIsIndices_, unitsInHeaders_,
                                    unitRegexp_, doublePrecision_, exampleRows_, rowComment_,
                                    includeFilters_, excludeFilters_, locale_, emptyField_),
                &Property::isModified)) {
            CSVReader reader(delimiters_, firstRowIsHeaders_, doublePrecision_);
            reader.setLocale(locale_)
                .setFirstColIndices(firstColumnIsIndices_)
                .setStripQuotes(stripQuotes_)
                .setNumberOfExampleRows(exampleRows_)
                .setHandleEmptyFields(emptyField_)
                .setUnitsInHeaders(unitsInHeaders_)
                .setUnitRegexp(unitRegexp_);

            reader.setFilters(createFilters());

            loadedData_ = reader.readData(inputFile_.get());
            columns_.updateForNewDataFrame(*loadedData_, overwrite);
        }

        auto dataFrame = std::make_shared<DataFrame>(*loadedData_);
        columns_.updateDataFrame(*dataFrame);
        data_.setData(dataFrame);
    } catch (const Exception& e) {
        LogProcessorError(e.getMessage());
        data_.clear();
        loadingFailed_ = true;
    }
}

void CSVSource::deserialize(Deserializer& d) {
    Processor::deserialize(d);
    deserialized_ = true;
}

namespace detail {

template <typename T>
decltype(std::declval<T&>().get()) getValue(PropertyOwner* parent, std::string_view identifier) {
    if (auto p = dynamic_cast<T*>(parent->getPropertyByIdentifier(identifier))) {
        return p->get();
    } else {
        throw Exception(IVW_CONTEXT_CUSTOM("CSVSource::createFilters"),
                        "Invalid filter property '{}', missing sub-property '{}'",
                        parent->getIdentifier(), identifier);
    }
}

}  // namespace detail

csvfilters::Filters CSVSource::createFilters() const {

    auto startsWith = [](std::string_view str, std::string_view prefix) {
        return str.substr(0, prefix.size()) == prefix;
    };

    auto parse = [&](const auto& filterProp, auto& rowFilters, auto& itemFilters) {
        for (auto prop : filterProp) {
            if (auto cp = dynamic_cast<BoolCompositeProperty*>(prop)) {
                if (!cp->isChecked()) continue;

                const auto& identifier = cp->getIdentifier();

                if (startsWith(identifier, "emptyLine")) {
                    rowFilters.push_back(csvfilters::emptyLines(
                        detail::getValue<BoolProperty>(cp, "filterOnHeader")));
                } else if (startsWith(identifier, "rowBegin")) {
                    rowFilters.push_back(
                        csvfilters::rowBegin(detail::getValue<StringProperty>(cp, "match"),
                                             detail::getValue<BoolProperty>(cp, "filterOnHeader")));
                } else if (startsWith(identifier, "lineRange")) {
                    const auto& range = detail::getValue<IntMinMaxProperty>(cp, "range");
                    rowFilters.push_back(csvfilters::lineRange(
                        range.x, range.y, detail::getValue<BoolProperty>(cp, "filterOnHeader")));
                } else if (startsWith(identifier, "stringItem")) {
                    itemFilters.push_back(csvfilters::stringMatch(
                        detail::getValue<IntProperty>(cp, "column"),
                        detail::getValue<OptionProperty<filters::StringComp>>(cp, "comp"),
                        detail::getValue<StringProperty>(cp, "match")));
                } else if (startsWith(identifier, "intItem")) {
                    itemFilters.push_back(csvfilters::intMatch(
                        detail::getValue<IntProperty>(cp, "column"),
                        detail::getValue<OptionProperty<filters::NumberComp>>(cp, "comp"),
                        detail::getValue<Int64Property>(cp, "value")));
                } else if (startsWith(identifier, "doubleItem")) {
                    itemFilters.push_back(csvfilters::doubleMatch(
                        detail::getValue<IntProperty>(cp, "column"),
                        detail::getValue<OptionProperty<filters::NumberComp>>(cp, "comp"),
                        detail::getValue<DoubleProperty>(cp, "value"),
                        detail::getValue<DoubleProperty>(cp, "epsilon")));
                } else if (startsWith(identifier, "intRangeItem")) {
                    const auto& range = detail::getValue<Int64MinMaxProperty>(cp, "range");
                    itemFilters.push_back(csvfilters::intRange(
                        detail::getValue<IntProperty>(cp, "column"), range.x, range.y));
                } else if (startsWith(identifier, "doubleRangeItem")) {
                    const auto& range = detail::getValue<DoubleMinMaxProperty>(cp, "range");
                    itemFilters.push_back(csvfilters::doubleRange(
                        detail::getValue<IntProperty>(cp, "column"), range.x, range.y));
                }
            }
        }
    };

    csvfilters::Filters filters;
    parse(includeFilters_, filters.includeRows, filters.includeItems);
    parse(excludeFilters_, filters.excludeRows, filters.excludeItems);

    if (!rowComment_.get().empty()) {
        filters.excludeRows.push_back(csvfilters::rowBegin(rowComment_, true));
    }
    return filters;
}

}  // namespace inviwo
