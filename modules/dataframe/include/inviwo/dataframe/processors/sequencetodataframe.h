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

#pragma once

#include <inviwo/dataframe/dataframemoduledefine.h>

#include <inviwo/core/datastructures/datasequence.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>
#include <inviwo/core/util/zip.h>

#include <inviwo/dataframe/datastructures/dataframe.h>

#include <ranges>

namespace inviwo {

template <typename T>
struct SequenceToDataFrameTraits;

template <>
struct SequenceToDataFrameTraits<Volume> {
    static auto header(DataFrame& df) {
        return std::tuple{df.addColumn<int>("X"),
                          df.addColumn<int>("Y"),
                          df.addColumn<int>("Z"),
                          df.addCategoricalColumn("Format"),
                          df.addColumn<int>("Channels"),
                          df.addColumn<double>("Data Min"),
                          df.addColumn<double>("Data Max"),
                          df.addColumn<double>("Value Min"),
                          df.addColumn<double>("Value Max"),
                          df.addCategoricalColumn("Axis"),
                          df.addCategoricalColumn("Unit"),
                          df.addCategoricalColumn("Filename")};
    }
    static void add(const Volume& volume, auto& cols) {
        auto& [x, y, z, format, channels, dMin, dMax, vMin, vMax, axis, unit, file] = cols;

        x->add(static_cast<int>(volume.getDimensions().x));
        y->add(static_cast<int>(volume.getDimensions().y));
        z->add(static_cast<int>(volume.getDimensions().z));

        format->add(volume.getDataFormat()->getString());
        channels->add(static_cast<int>(volume.getDataFormat()->getComponents()));

        dMin->add(volume.dataMap.dataRange.x);
        dMax->add(volume.dataMap.dataRange.y);
        vMin->add(volume.dataMap.valueRange.x);
        vMax->add(volume.dataMap.valueRange.y);

        axis->add(volume.dataMap.valueAxis.name);
        unit->add(fmt::to_string(volume.dataMap.valueAxis.unit));

        if (const auto* filename = volume.getMetaData<StringMetaData>("filename")) {
            file->add(filename->get());
        } else {
            file->add("");
        }
    }
};

template <>
struct SequenceToDataFrameTraits<Layer> {
    static auto header(DataFrame& df) {
        return std::tuple{df.addColumn<int>("X"),
                          df.addColumn<int>("Y"),
                          df.addCategoricalColumn("Format"),
                          df.addColumn<int>("Channels"),
                          df.addColumn<double>("Data Min"),
                          df.addColumn<double>("Data Max"),
                          df.addColumn<double>("Value Min"),
                          df.addColumn<double>("Value Max"),
                          df.addCategoricalColumn("Axis"),
                          df.addCategoricalColumn("Unit"),
                          df.addCategoricalColumn("Filename")};
    }
    static void add(const Layer& layer, auto& cols) {
        auto& [x, y, format, channels, dMin, dMax, vMin, vMax, axis, unit, file] = cols;

        x->add(static_cast<int>(layer.getDimensions().x));
        y->add(static_cast<int>(layer.getDimensions().y));

        format->add(layer.getDataFormat()->getString());
        channels->add(static_cast<int>(layer.getDataFormat()->getComponents()));

        dMin->add(layer.dataMap.dataRange.x);
        dMax->add(layer.dataMap.dataRange.y);
        vMin->add(layer.dataMap.valueRange.x);
        vMax->add(layer.dataMap.valueRange.y);

        axis->add(layer.dataMap.valueAxis.name);
        unit->add(fmt::to_string(layer.dataMap.valueAxis.unit));

        if (const auto* filename = layer.getMetaData<StringMetaData>("filename")) {
            file->add(filename->get());
        } else {
            file->add("");
        }
    }
};

template <>
struct SequenceToDataFrameTraits<Image> {
    static auto header(DataFrame& df) {
        return std::tuple{df.addColumn<int>("X"), df.addColumn<int>("Y"),
                          df.addCategoricalColumn("Format"), df.addColumn<int>("Channels"),
                          df.addCategoricalColumn("Filename")};
    }
    static void add(const Image& image, auto& cols) {
        auto& [x, y, format, channels, file] = cols;

        x->add(static_cast<int>(image.getDimensions().x));
        y->add(static_cast<int>(image.getDimensions().y));

        format->add(image.getDataFormat()->getString());
        channels->add(static_cast<int>(image.getDataFormat()->getComponents()));

        if (const auto* filename = image.getMetaData<StringMetaData>("filename")) {
            file->add(filename->get());
        } else {
            file->add("");
        }
    }
};

template <>
struct SequenceToDataFrameTraits<Mesh> {
    static auto header(DataFrame& df) {
        return std::tuple{df.addColumn<int>("Buffers"), df.addColumn<int>("IndexBuffers"),
                          df.addCategoricalColumn("Filename")};
    }
    static void add(const Mesh& mesh, auto& cols) {
        auto& [buffers, indexBuffers, file] = cols;

        buffers->add(static_cast<int>(mesh.getNumberOfBuffers()));
        indexBuffers->add(static_cast<int>(mesh.getNumberOfIndices()));

        if (const auto* filename = mesh.getMetaData<StringMetaData>("filename")) {
            file->add(filename->get());
        } else {
            file->add("");
        }
    }
};

template <>
struct SequenceToDataFrameTraits<BufferBase> {
    static auto header(DataFrame& df) {
        return std::tuple{df.addColumn<int>("Size"), df.addCategoricalColumn("Format"),
                          df.addColumn<int>("Components")};
    }
    static void add(const BufferBase& buffer, auto& cols) {
        auto& [size, format, components] = cols;
        size->add(static_cast<int>(buffer.getSize()));
        format->add(buffer.getDataFormat()->getString());
        components->add(static_cast<int>(buffer.getDataFormat()->getComponents()));
    }
};

template <typename T>
class SequenceToDataFrame : public Processor {
public:
    SequenceToDataFrame();

    virtual void process() override;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    static constexpr std::string_view identifierSuffix() { return ".sequence.to.dataframe"; }

private:
    DataInport<DataSequence<T>> inport_;
    DataOutport<DataFrame> outport_;
};

template <typename T>
const ProcessorInfo& SequenceToDataFrame<T>::getProcessorInfo() const {
    static const ProcessorInfo info{ProcessorTraits<SequenceToDataFrame<T>>::getProcessorInfo()};
    return info;
}

template <typename T>
struct ProcessorTraits<SequenceToDataFrame<T>> {
    static ProcessorInfo getProcessorInfo() {

        const auto name = fmt::format("{} Sequence To DataFrame", DataTraits<T>::dataName());
        const auto cid = fmt::format("{}{}", DataTraits<T>::classIdentifier(),
                                     SequenceToDataFrame<T>::identifierSuffix());

        const auto doc =
            fmt::format("Select a specific {0} out of a sequence", DataTraits<T>::dataName());

        return {
            cid,                // Class identifier
            name,               // Display name
            "Data Convert",     // Category
            CodeState::Stable,  // Code state
            Tags::CPU,          // Tags
            Document{doc},
            true  // Visible
        };
    }
};

template <typename T>
SequenceToDataFrame<T>::SequenceToDataFrame()
    : Processor{}
    , inport_{"inport", "DataSequence to brush"_help}
    , outport_{"outport", "Brushed DataSequence"_help} {
    addPorts(inport_, outport_);
}

template <typename T>
void SequenceToDataFrame<T>::process() {
    const auto sequence = inport_.getData();

    auto df = std::make_shared<DataFrame>();
    auto index = df->addColumn<int>("index");
    auto cols = SequenceToDataFrameTraits<T>::header(*df);

    for (auto [i, item] : util::enumerate(*sequence)) {
        index->add(static_cast<int>(i));
        SequenceToDataFrameTraits<T>::add(*item, cols);
    }

    df->updateIndexBuffer();

    outport_.setData(df);
}

}  // namespace inviwo
