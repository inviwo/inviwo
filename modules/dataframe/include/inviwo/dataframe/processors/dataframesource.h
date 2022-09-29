/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2022 Inviwo Foundation
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

#include <inviwo/dataframe/dataframemoduledefine.h>                  // for IVW_MODULE_DATAFRAME...

#include <inviwo/core/io/datareader.h>                               // for DataReaderType
#include <inviwo/core/io/datareaderexception.h>                      // for DataReaderException
#include <inviwo/core/processors/processorinfo.h>                    // for ProcessorInfo
#include <inviwo/dataframe/datastructures/dataframe.h>               // for DataFrameOutport
#include <inviwo/dataframe/properties/columnmetadatalistproperty.h>  // for ColumnMetaDataListPr...
#include <modules/base/processors/datasource.h>                      // for DataSource

#include <map>                                                       // for map, operator!=
#include <memory>                                                    // for shared_ptr
#include <string>                                                    // for string

namespace inviwo {
class InviwoApplication;

/** \docpage{org.inviwo.DataFrameSource, Data Frame Source}
 * ![](org.inviwo.DataFrameSource.png?classIdentifier=org.inviwo.DataFrameSource)
 * Loads a DataFrame from file.
 *
 * ### Outports
 *   * __Outport__ The loaded DataFrame
 *
 * ### Properties
 *   * __File name__ File to load.
 */
class IVW_MODULE_DATAFRAME_API DataFrameSource : public DataSource<DataFrame, DataFrameOutport> {
public:
    DataFrameSource(InviwoApplication* app, const std::string& file = "");
    virtual ~DataFrameSource() = default;

    virtual void dataLoaded(std::shared_ptr<DataFrame> data) override;
    virtual void dataDeserialized(std::shared_ptr<DataFrame> data) override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    ColumnMetaDataListProperty columns_;
};

}  // namespace inviwo
