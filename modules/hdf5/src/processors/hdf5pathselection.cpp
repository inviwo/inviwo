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

#include <modules/hdf5/processors/hdf5pathselection.h>
#include <modules/hdf5/datastructures/hdf5metadata.h>

namespace inviwo {

namespace hdf5 {

const ProcessorInfo PathSelection::processorInfo_{
    "org.inviwo.hdf5.PathSelection",  // Class identifier
    "HDF5 Path Selection",            // Display name
    "Data Input",                     // Category
    CodeState::Stable,                // Code state
    Tags::None,                       // Tags
};
const ProcessorInfo PathSelection::getProcessorInfo() const { return processorInfo_; }

PathSelection::PathSelection()
    : Processor(), inport_("inport"), outport_("outport"), selection_("selection", "Select Group") {
    addPort(inport_);
    addPort(outport_);

    addProperty(selection_);
    selection_.setSerializationMode(PropertySerializationMode::All);

    inport_.onChange([this]() { onDataChange(); });
}

void PathSelection::process() {
    if (inport_.hasData()) {
        auto data = inport_.getData();
        outport_.setData(data->getHandleForPath(selection_.getSelectedValue()));
    }
}

void PathSelection::onDataChange() {
    const auto data = inport_.getData();

    std::vector<OptionPropertyStringOption> options;
    for (const auto& meta : util::getMetaData(data->getGroup())) {
        if (meta.type_ == MetaData::HDFType::Group) {
            options.emplace_back(meta.path_, meta.path_, meta.path_);
        }
    }
    selection_.replaceOptions(options);
    selection_.setCurrentStateAsDefault();
}

}  // namespace hdf5

}  // namespace inviwo
