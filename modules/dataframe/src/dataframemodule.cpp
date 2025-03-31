/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2025 Inviwo Foundation
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

#include <inviwo/core/common/inviwoapplication.h>  // for InviwoApplication
#include <inviwo/core/common/inviwomodule.h>       // for InviwoModule
#include <inviwo/core/util/moduleutils.h>
#include <inviwo/core/io/datareader.h>                 // for DataReader
#include <inviwo/core/io/datawriter.h>                 // for DataWriter
#include <inviwo/core/ports/outportiterable.h>         // for OutportIterable, Out...
#include <inviwo/core/properties/optionproperty.h>     // for OptionProperty
#include <inviwo/core/properties/propertyconverter.h>  // for OptionToStringConverter
#include <inviwo/core/util/exception.h>                // for Exception
#include <inviwo/core/util/foreacharg.h>               // for for_each_type
#include <inviwo/core/util/glmvec.h>                   // for uvec3
#include <inviwo/core/util/staticstring.h>             // for operator+
#include <inviwo/core/util/stringconversion.h>         // for htmlEncode
#include <modules/base/processors/filecache.h>
#include <modules/base/processors/inputselector.h>
#include <inviwo/dataframe/datastructures/dataframe.h>                // for DataFrame
#include <inviwo/dataframe/io/json/dataframepropertyjsonconverter.h>  // IWYU pragma: keep
#include <inviwo/dataframe/io/csvreader.h>                            // for CSVReader
#include <inviwo/dataframe/io/csvwriter.h>                            // for CSVWriter
#include <inviwo/dataframe/io/jsondataframereader.h>
#include <inviwo/dataframe/io/jsondataframewriter.h>
#include <inviwo/dataframe/io/xmlwriter.h>                          // for XMLWriter
#include <inviwo/dataframe/processors/csvsource.h>                  // for CSVSource
#include <inviwo/dataframe/processors/dataframeexporter.h>          // for DataFrameExporter
#include <inviwo/dataframe/processors/dataframefilter.h>            // for DataFrameFilter
#include <inviwo/dataframe/processors/dataframefloat32converter.h>  // for DataFrameFloat32Conv...
#include <inviwo/dataframe/processors/dataframejoin.h>              // for DataFrameJoin
#include <inviwo/dataframe/processors/dataframemetadata.h>          // for DataFrameMetaData
#include <inviwo/dataframe/processors/dataframesource.h>            // for DataFrameSource
#include <inviwo/dataframe/processors/dataframetobuffer.h>
#include <inviwo/dataframe/processors/dataframetomesh.h>
#include <inviwo/dataframe/processors/imagetodataframe.h>           // for ImageToDataFrame
#include <inviwo/dataframe/processors/syntheticdataframe.h>         // for SyntheticDataFrame
#include <inviwo/dataframe/processors/tffromdataframecolumn.h>
#include <inviwo/dataframe/processors/volumesequencetodataframe.h>  // for VolumeSequenceToData...
#include <inviwo/dataframe/processors/volumetodataframe.h>          // for VolumeToDataFrame
#include <inviwo/dataframe/processors/dataframetovector.h>
#include <inviwo/dataframe/properties/colormapproperty.h>            // for ColormapProperty
#include <inviwo/dataframe/properties/columnmetadatalistproperty.h>  // for ColumnMetaDataListPr...
#include <inviwo/dataframe/properties/columnmetadataproperty.h>      // for ColumnMetaDataProperty
#include <inviwo/dataframe/properties/columnoptionproperty.h>        // for ColumnOptionProperty
#include <inviwo/dataframe/properties/filterlistproperty.h>          // for FilterListProperty
#include <inviwo/dataframe/properties/optionconverter.h>             // IWYU pragma: keep
#include <inviwo/dataframe/util/filters.h>                           // for NumberComp, StringComp
#include <inviwo/dataframe/jsondataframeconversion.h>                // IWYU pragma: keep
#include <modules/json/jsonmodule.h>                                 // for JSONModule

#include <cstddef>        // for size_t
#include <functional>     // for __base, function
#include <memory>         // for unique_ptr, make_unique
#include <string>         // for operator+, operator==
#include <tuple>          // for tuple
#include <unordered_map>  // for operator!=
#include <utility>        // for move
#include <vector>         // for vector

#include <fmt/core.h>      // for format, format_to
#include <glm/common.hpp>  // for clamp

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
