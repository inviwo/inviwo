/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2026 Inviwo Foundation
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

#include <inviwo/core/ports/dataoutport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorinfo.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/util/staticstring.h>
#include <inviwo/dataframe/datastructures/dataframe.h>
#include <inviwo/dataframe/io/csvreader.h>
#include <inviwo/dataframe/properties/columnmetadatalistproperty.h>
#include <inviwo/dataframe/properties/filterlistproperty.h>
#include <inviwo/dataframe/util/filters.h>

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <fmt/core.h>

namespace inviwo {
class Deserializer;

class IVW_MODULE_DATAFRAME_API CSVSource : public Processor {
public:
    CSVSource(const std::filesystem::path& file = "");
    virtual ~CSVSource() = default;

    virtual void process() override;

    virtual const ProcessorInfo& getProcessorInfo() const override;
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
