/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <modules/hdf5/processors/hdf5dataframesource.h>
#include <modules/hdf5/datastructures/hdf5handle.h>
#include <modules/hdf5/hdf5read.h>
#include <modules/hdf5/datastructures/hdf5path.h>
#include <modules/hdf5/hdf5utils.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/dataframe/datastructures/dataframe.h>

#include <algorithm>
#include <numeric>

namespace inviwo::hdf5 {

const ProcessorInfo HDF5ToDataFrame::processorInfo_{
    "org.inviwo.hdf5.ToDataFrame",               // Class identifier
    "HDF5 To DataFrame",                         // Display name
    "Data Input",                                // Category
    CodeState::Stable,                           // Code state
    Tags::CPU | Tag{"HDF5"} | Tag{"DataFrame"},  // Tags
    "Load a DataFrame from an HDF5 file handle. "
    "Each enabled 1-D dataset becomes one column."_help,
};
const ProcessorInfo& HDF5ToDataFrame::getProcessorInfo() const { return processorInfo_; }

HDF5ToDataFrame::HDF5ToDataFrame()
    : Processor()
    , inport_("inport")
    , outport_("outport")
    , automaticEvaluation_("automaticEvaluation", "Automatic loading", true,
                           InvalidationLevel::Valid)
    , evaluate_("evaluate", "Load")
    , columns_("columns", "Columns")
    , dirty_(false) {

    addPort(inport_);
    addPort(outport_);

    inport_.onChange([this]() { onDataChange(); });

    automaticEvaluation_.onChange([this]() { evaluate_.setReadOnly(automaticEvaluation_); });
    evaluate_.onChange([this]() { dirty_ = true; });

    columns_.setCollapsed(false);
    columns_.onChange([this]() {
        if (automaticEvaluation_) {
            dirty_ = true;
            this->invalidate(InvalidationLevel::InvalidOutput);
        }
    });

    addProperties(automaticEvaluation_, evaluate_, columns_);
}

HDF5ToDataFrame::~HDF5ToDataFrame() = default;

void HDF5ToDataFrame::process() try {
    if (dirty_) {
        dirty_ = false;
        makeDataFrame();
        deserialized_ = false;
    }
    if (dataFrame_) {
        outport_.setData(dataFrame_);
    }
} catch (H5::Exception& e) {
    throw Exception(SourceContext{}, "Error reading HDF5 data: {}", e.getDetailMsg());
}

void HDF5ToDataFrame::onDataChange() {
    if (inport_.hasData()) {
        const auto data = inport_.getData();
        const std::vector<DataSetInfo> metadata = util::getDataSets(*data);

        dataMatches_.clear();
        std::ranges::copy_if(metadata, std::back_inserter(dataMatches_),
                             [](const DataSetInfo& meta) {
                                 const auto dims = meta.getColumnMajorDimensions();
                                 return dims.size() == 1ull && dims[0] > 0ull;
                             });

        rebuildColumnProperties();
    } else {
        dataMatches_.clear();
        rebuildColumnProperties();
    }

    dirty_ = true;
}

void HDF5ToDataFrame::rebuildColumnProperties() {
    const NetworkLock lock{this};

    // Remove all previously added dynamic properties.
    for (auto& prop : columnProps_) {
        columns_.removeProperty(prop.get());
    }
    columnProps_.clear();

    // Add one BoolProperty per 1-D dataset found.
    for (size_t i = 0; i < dataMatches_.size(); ++i) {
        const auto& meta = dataMatches_[i];
        // Use a sanitised version of the path as identifier to stay unique.
        const std::string id = "col" + std::to_string(i);
        const std::string displayName = util::dataSetDescription(meta);

        auto prop = std::make_unique<BoolProperty>(id, displayName, true);
        columns_.addProperty(prop.get(), false);
        columnProps_.push_back(std::move(prop));
    }

    columns_.setCurrentStateAsDefault();
}

void HDF5ToDataFrame::makeDataFrame() {
    if (!inport_.hasData() || dataMatches_.empty()) return;

    const auto data = inport_.getData();

    // Collect the enabled datasets.
    std::vector<size_t> enabledIndices;
    for (size_t i = 0; i < columnProps_.size(); ++i) {
        if (columnProps_[i]->get()) {
            enabledIndices.push_back(i);
        }
    }

    if (enabledIndices.empty()) {
        dataFrame_ = std::make_shared<DataFrame>();
        return;
    }

    // Verify all enabled datasets have the same number of elements.
    const size_t expectedRows = dataMatches_[enabledIndices[0]].getColumnMajorDimensions()[0];

    for (const size_t idx : enabledIndices) {
        const size_t rows = dataMatches_[idx].getColumnMajorDimensions()[0];
        if (rows != expectedRows) {
            throw Exception(SourceContext{},
                            "Cannot create DataFrame: dataset '{}' has {} rows but '{}' has {}",
                            dataMatches_[enabledIndices[0]].path_.toString(), expectedRows,
                            dataMatches_[idx].path_.toString(), rows);
        }
    }

    auto df = std::make_shared<DataFrame>();

    for (const size_t idx : enabledIndices) {
        const DataSetInfo& meta = dataMatches_[idx];
        const size_t rows = meta.getColumnMajorDimensions()[0];

        // Build a full-range selection for this 1-D dataset.
        const std::vector<Selection> selection{{0, rows, 1}};

        auto buffer = getBufferAtPathAsType(*data + meta.path_, selection, nullptr);

        // Use the last path component as column header.
        const std::string pathStr = meta.path_.toString();
        const auto slash = pathStr.rfind('/');
        const std::string header =
            (slash != std::string::npos) ? pathStr.substr(slash + 1) : pathStr;

        // Read unit from HDF5 attribute if present.
        Unit unit{};
        try {
            auto dataset = data->getGroup().openDataSet(meta.path_);
            if (dataset.attrExists("units")) {
                const auto attr = dataset.openAttribute("units");
                if (attr.getDataType().getClass() == H5T_STRING) {
                    std::string unitStr;
                    attr.read(attr.getStrType(), unitStr);
                    unit = units::unit_from_string(unitStr);
                }
            }
        } catch (...) {
            // Attribute reading is best-effort; ignore failures.
        }

        df->addColumnFromBuffer(header, buffer, unit);
    }

    df->updateIndexBuffer();
    dataFrame_ = std::move(df);
}

void HDF5ToDataFrame::deserialize(Deserializer& d) {
    Processor::deserialize(d);
    deserialized_ = true;
    dirty_ = true;
}

}  // namespace inviwo::hdf5
