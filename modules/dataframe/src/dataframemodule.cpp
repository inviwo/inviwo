/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2026 Inviwo Foundation
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

#include <inviwo/dataframe/dataframemodule.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/util/moduleutils.h>
#include <inviwo/core/io/datareader.h>
#include <inviwo/core/io/datawriter.h>
#include <inviwo/core/ports/outportiterable.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/propertyconverter.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/foreacharg.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/staticstring.h>
#include <inviwo/core/util/stringconversion.h>
#include <modules/base/processors/filecache.h>
#include <modules/base/processors/inputselector.h>
#include <inviwo/dataframe/datastructures/dataframe.h>
#include <inviwo/dataframe/io/json/dataframepropertyjsonconverter.h>  // IWYU pragma: keep
#include <inviwo/dataframe/io/csvreader.h>
#include <inviwo/dataframe/io/csvwriter.h>
#include <inviwo/dataframe/io/jsondataframereader.h>
#include <inviwo/dataframe/io/jsondataframewriter.h>
#include <inviwo/dataframe/io/xmlwriter.h>
#include <inviwo/dataframe/processors/csvsource.h>
#include <inviwo/dataframe/processors/dataframeexporter.h>
#include <inviwo/dataframe/processors/dataframefilter.h>
#include <inviwo/dataframe/processors/dataframefloat32converter.h>
#include <inviwo/dataframe/processors/dataframejoin.h>
#include <inviwo/dataframe/processors/dataframemetadata.h>
#include <inviwo/dataframe/processors/dataframesource.h>
#include <inviwo/dataframe/processors/dataframetobuffer.h>
#include <inviwo/dataframe/processors/dataframetomesh.h>
#include <inviwo/dataframe/processors/filelist.h>
#include <inviwo/dataframe/processors/imagetodataframe.h>
#include <inviwo/dataframe/processors/syntheticdataframe.h>
#include <inviwo/dataframe/processors/tffromdataframecolumn.h>
#include <inviwo/dataframe/processors/volumesequencetodataframe.h>
#include <inviwo/dataframe/processors/volumetodataframe.h>
#include <inviwo/dataframe/processors/dataframetovector.h>
#include <inviwo/dataframe/properties/colormapproperty.h>
#include <inviwo/dataframe/properties/columnmetadatalistproperty.h>
#include <inviwo/dataframe/properties/columnmetadataproperty.h>
#include <inviwo/dataframe/properties/columnoptionproperty.h>
#include <inviwo/dataframe/properties/filterlistproperty.h>
#include <inviwo/dataframe/properties/optionconverter.h>             // IWYU pragma: keep
#include <inviwo/dataframe/util/filters.h>
#include <inviwo/dataframe/jsondataframeconversion.h>                // IWYU pragma: keep
#include <modules/json/jsonmodule.h>

#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include <fmt/core.h>
#include <glm/common.hpp>

namespace inviwo {

enum class OptionRegEnumInt : int {};
enum class OptionRegEnumUInt : unsigned int {};

DataFrameModule::DataFrameModule(InviwoApplication* app)
    : InviwoModule(app, "DataFrame")
    , JSONSupplier<Inport, PortTraits>{util::getModuleByTypeOrThrow<JSONModule>(app)
                                           .getRegistry<Inport>()}
    , JSONSupplier<Property, PropertyTraits>{
          util::getModuleByTypeOrThrow<JSONModule>(app).getRegistry<Property>()} {
    // Register objects that can be shared with the rest of inviwo here:

    // Processors
    registerProcessor<CSVSource>();
    registerProcessor<DataFrameFilter>();
    registerProcessor<DataFrameJoin>();
    registerProcessor<DataFrameSource>();
    registerProcessor<DataFrameExporter>();
    registerProcessor<DataFrameFloat32Converter>();
    registerProcessor<DataFrameMetaData>();
    registerProcessor<DataFrameToBuffer>();
    registerProcessor<DataFrameToMesh>();
    registerProcessor<DataFrameToVector>();
    registerProcessor<FileList>();
    registerProcessor<ImageToDataFrame>();
    registerProcessor<SyntheticDataFrame>();
    registerProcessor<TFFromDataFrameColumn>();
    registerProcessor<VolumeToDataFrame>();
    registerProcessor<VolumeSequenceToDataFrame>();

    registerProcessor<InputSelector<MultiDataInport<DataFrame>, DataOutport<DataFrame>>>();
    registerProcessor<FileCache<DataFrame>>();

    registerDefaultsForDataType<DataFrame>();
    // Properties
    registerProperty<ColormapProperty>();
    registerProperty<ColumnOptionProperty>();
    registerProperty<ColumnMetaDataProperty>();
    registerProperty<ColumnMetaDataListProperty>();
    registerProperty<FilterListProperty>();
    registerProperty<OptionProperty<filters::StringComp>>();
    registerProperty<OptionProperty<filters::NumberComp>>();

    // Readers and writes
    registerDataReader(std::make_unique<CSVReader>());
    registerDataReader(std::make_unique<JSONDataFrameReader>());

    registerDataWriter(std::make_unique<CSVWriter>());
    registerDataWriter(std::make_unique<XMLWriter>());
    registerDataWriter(std::make_unique<JSONDataFrameWriter>());

    // Data converters
    registerPropertyConverter(std::make_unique<OptionToStringConverter<ColumnOptionProperty>>());

    registerJSONConverter<ColumnOptionProperty>();

    registerJSONConverter<DataFrameInport>();

    using OptionTypes = std::tuple<unsigned int, int, size_t, float, double, std::string>;
    util::for_each_type<OptionTypes>{}([&]<typename T>() {
        registerPropertyConverter(
            std::make_unique<ColumnOptionToOptionConverter<OptionProperty<T>>>());
        registerPropertyConverter(
            std::make_unique<OptionToColumnOptionConverter<OptionProperty<T>>>());
    });

    using OptionEnumTypes = std::tuple<OptionRegEnumInt, OptionRegEnumUInt>;
    util::for_each_type<OptionEnumTypes>{}([&]<typename T>() {
        registerPropertyConverter(
            std::make_unique<ColumnOptionToOptionConverter<OptionProperty<T>>>());
        registerPropertyConverter(
            std::make_unique<OptionToColumnOptionConverter<OptionProperty<T>>>());
    });
}

}  // namespace inviwo
