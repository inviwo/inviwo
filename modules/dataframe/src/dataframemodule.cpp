/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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
#include <inviwo/dataframe/io/json/dataframepropertyjsonconverter.h>
#include <inviwo/dataframe/processors/csvsource.h>
#include <inviwo/dataframe/processors/dataframesource.h>
#include <inviwo/dataframe/processors/dataframeexporter.h>
#include <inviwo/dataframe/processors/imagetodataframe.h>
#include <inviwo/dataframe/processors/syntheticdataframe.h>
#include <inviwo/dataframe/processors/volumetodataframe.h>
#include <inviwo/dataframe/processors/volumesequencetodataframe.h>
#include <inviwo/dataframe/properties/colormapproperty.h>

#include <inviwo/dataframe/io/csvreader.h>
#include <inviwo/dataframe/io/jsonreader.h>

#include <inviwo/core/properties/propertyconverter.h>

#include <modules/json/jsonmodule.h>

namespace inviwo {

DataFrameModule::DataFrameModule(InviwoApplication* app) : InviwoModule(app, "DataFrame") {
    // Register objects that can be shared with the rest of inviwo here:

    // Processors
    registerProcessor<CSVSource>();
    registerProcessor<DataFrameSource>();
    registerProcessor<DataFrameExporter>();
    registerProcessor<ImageToDataFrame>();
    registerProcessor<SyntheticDataFrame>();
    registerProcessor<VolumeToDataFrame>();
    registerProcessor<VolumeSequenceToDataFrame>();

    registerDefaultsForDataType<DataFrame>();
    // Properties
    registerProperty<ColormapProperty>();
    registerProperty<DataFrameColumnProperty>();

    // Readers and writes
    registerDataReader(std::make_unique<CSVReader>());
    registerDataReader(std::make_unique<JSONDataFrameReader>());

    // Data converters
    registerPropertyConverter(std::make_unique<OptionToStringConverter<DataFrameColumnProperty>>());
    app->getModuleByType<JSONModule>()->registerPropertyJSONConverter<DataFrameColumnProperty>();
}

}  // namespace inviwo
