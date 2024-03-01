/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2024 Inviwo Foundation
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

#include <inviwo/dataframe/dataframemoduledefine.h>  // for IVW_MODULE_DATAFRAME...

#include <inviwo/core/ports/dataoutport.h>                           // for DataOutport
#include <inviwo/core/processors/processor.h>                        // for Processor
#include <inviwo/core/processors/processorinfo.h>                    // for ProcessorInfo
#include <inviwo/core/properties/boolcompositeproperty.h>            // for BoolCompositeProperty
#include <inviwo/core/properties/boolproperty.h>                     // for BoolProperty
#include <inviwo/core/properties/buttonproperty.h>                   // for ButtonProperty
#include <inviwo/core/properties/fileproperty.h>                     // for FileProperty
#include <inviwo/core/properties/optionproperty.h>                   // for OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>                  // for IntSizeTProperty
#include <inviwo/core/properties/stringproperty.h>                   // for StringProperty
#include <inviwo/core/util/staticstring.h>                           // for operator+
#include <inviwo/dataframe/datastructures/dataframe.h>               // for DataFrame
#include <inviwo/dataframe/io/csvreader.h>                           // for CSVReader::EmptyField
#include <inviwo/dataframe/properties/columnmetadatalistproperty.h>  // for ColumnMetaDataListPr...
#include <inviwo/dataframe/properties/filterlistproperty.h>          // for FilterListProperty
#include <inviwo/dataframe/util/filters.h>                           // for Filters

#include <functional>   // for __base
#include <memory>       // for shared_ptr
#include <string>       // for operator==, operator+
#include <string_view>  // for operator==
#include <vector>       // for operator!=, vector

#include <fmt/core.h>  // for format

namespace inviwo {
class Deserializer;

/** \docpage{org.inviwo.CSVSource, CSVSource}
 * ![](org.inviwo.CSVSource.png?classIdentifier=org.inviwo.CSVSource)
 * Reads comma separated values (CSV) and converts it into a DataFrame.
 *
 * ### Outports
 *   * __data__  DataFrame representation of the CSV input file
 *
 * ### Properties
 *   * __First Row Headers__   if true, the first row is used as column names in the DataFrame
 *   * __Delimiters__          defines the delimiter between values (default ',')
 */

class IVW_MODULE_DATAFRAME_API CSVSource : public Processor {
public:
    CSVSource(const std::filesystem::path& file = "");
    virtual ~CSVSource() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void deserialize(Deserializer& d) override;

private:
    csvfilters::Filters createFilters() const;

    DataOutport<DataFrame> data_;
    FileProperty inputFile_;
    BoolProperty firstRowIsHeaders_;
    BoolProperty firstColumnIsIndices_;
    BoolCompositeProperty unitsInHeaders_;
    StringProperty unitRegexp_;
    StringProperty delimiters_;
    BoolProperty stripQuotes_;
    BoolProperty doublePrecision_;
    IntSizeTProperty exampleRows_;
    StringProperty rowComment_;
    FilterListProperty includeFilters_;
    FilterListProperty excludeFilters_;
    StringProperty locale_;
    OptionProperty<CSVReader::EmptyField> emptyField_;
    ButtonProperty reloadData_;

    ColumnMetaDataListProperty columns_;
    std::shared_ptr<DataFrame> loadedData_;
    std::string error_;
    bool deserialized_;
};

}  // namespace inviwo
