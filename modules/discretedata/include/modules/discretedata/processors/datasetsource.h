/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#include <modules/discretedata/discretedatamoduledefine.h>
#include <modules/base/processors/datasource.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <modules/discretedata/ports/datasetport.h>
#include <modules/discretedata/dataset.h>

namespace inviwo {
namespace discretedata {

/** \docpage{org.inviwo.DataSetSource, DataSet Source}
    ![](org.inviwo.DataSetSource.png?classIdentifier=org.inviwo.DataSetSource)

    Loads a DataSet. The underlying connectivity depends on the file loaded.

    ### Outports
      * __Outport__ The loaded dataset.

    ### Properties
      * __File name__ File to load.
*/

/** \class DataSetSource
    \brief Loads a DataSet. Data-driven.
*/
class IVW_MODULE_DISCRETEDATA_API DataSetSource
    : public DataSource<DataSetInitializer, DataSetOutport> {
    // Construction / Deconstruction
public:
    DataSetSource();
    virtual ~DataSetSource() = default;

    // Methods
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    // Update the list of channels loaded to let the user rename them.
    void updateChannelNames(std::shared_ptr<DataSetInitializer> data);

    // Called when we load new data.
    virtual void dataLoaded(std::shared_ptr<DataSetInitializer> data) override {
        updateChannelNames(data);
    }
    // Called when we deserialized old data.
    virtual void dataDeserialized(std::shared_ptr<DataSetInitializer> data) {
        updateChannelNames(data);
    }
    // Change names.
    virtual void process() override;

    // Properties
public:
    CompositeProperty renameChannels_;
};

}  // namespace discretedata
}  // namespace inviwo
