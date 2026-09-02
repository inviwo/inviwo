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

#include <modules/hdf5/hdf5read.h>
#include <modules/hdf5/hdf5types.h>
#include <modules/hdf5/hdf5exception.h>

#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/formatdispatching.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>

#include <modules/base/algorithm/dataminmax.h>

#include <algorithm>

namespace inviwo::hdf5 {

namespace {

// Read the standard value attributes (units, long_name, missing_value) into a Volume or Layer and
// record the missing value in @p ignore.
template <typename T>
void readValueAttributes(const H5::DataSet& dataset, T& data, IgnoreValues& ignore) {
    if (dataset.attrExists("units")) {
        const auto attr = dataset.openAttribute("units");
        if (attr.getDataType().getClass() == H5T_STRING) {
            std::string units;
            attr.read(attr.getStrType(), units);
            data.dataMap.valueAxis.unit = units::unit_from_string(units);
        }
    }

    if (dataset.attrExists("long_name")) {
        const auto attr = dataset.openAttribute("long_name");
        if (attr.getDataType().getClass() == H5T_STRING) {
            std::string name;
            attr.read(attr.getStrType(), name);
            data.dataMap.valueAxis.name = name;
        }
    }

    if (dataset.attrExists("missing_value")) {
        const auto attr = dataset.openAttribute("missing_value");
        if (attr.getDataType().getClass() == H5T_FLOAT) {
            double missingValue{};
            attr.read(H5::PredType::NATIVE_DOUBLE, &missingValue);
            data.template setMetaData<MetaDataType<double>>("missing_value", missingValue);
            ignore.floatingPoint = missingValue;
        } else if (attr.getDataType().getClass() == H5T_INTEGER) {
            std::int64_t missingValue{};
            attr.read(H5::PredType::NATIVE_INT64, &missingValue);
            data.template setMetaData<MetaDataType<std::int64_t>>("missing_value", missingValue);
            ignore.signedInteger = missingValue;
        }
    }
}

template <size_t N>
struct DimConfig {
    std::vector<hsize_t> start;
    std::vector<hsize_t> count;
    std::vector<hsize_t> stride;
    std::vector<hsize_t> dataDimensions;
    std::vector<hsize_t> selectedDimensions;
};

template <size_t N>
DimConfig<N> getDimConfig(const H5::DataSpace& dataSpace,
                          const std::vector<Selection>& selections) {
    const size_t rank = dataSpace.getSimpleExtentNdims();
    if (selections.size() != rank) {
        throw Exception("Selection not of the same rank as the data");
    }

    DimConfig<N> config;

    config.dataDimensions.resize(rank);
    dataSpace.getSimpleExtentDims(config.dataDimensions.data());

    /*
     * Column major, i.e. the FIRST listed dimension is the fasted changing
     * Inviwo, OpenGL, matlab, Fortran
     *
     * Row major, i.e. the LAST listed dimension is the fasted changing
     * HDF, C/C++, Mathematica, Python
     *
     * Solution reverse all the dimension lists.
     * Row major version of the selection to match the hdf row major dataDimensions.
     */
    for (auto&& selection :
         std::views::zip(selections | std::views::reverse, config.dataDimensions) |
             std::views::transform([](auto&& item) { return std::apply(clamp, item); })) {

        config.start.emplace_back(selection.start);
        config.count.emplace_back(selection.count);
        config.stride.emplace_back(selection.stride);
    }

    config.selectedDimensions.assign_range(config.count |
                                           std::views::filter([](hsize_t c) { return c > 1; }));

    if (config.selectedDimensions.size() > N) {
        throw Exception{SourceContext{}, "Invalid selection, resulting rank {} > {}",
                        config.selectedDimensions.size(), N};
    }

    return config;
}

}  // namespace

std::shared_ptr<Volume> getVolumeAtPathAsType(
    const Handle& handle, std::vector<Selection> selection, const DataFormatBase* type,
    const std::function<std::shared_ptr<Volume>(const VolumeConfig&)>& getVolume) {

    auto dataset = handle.open();
    const H5::DataSpace dataSpace = dataset.getSpace();
    const auto config = getDimConfig<3>(dataSpace, selection);

    dataSpace.selectHyperslab(H5S_SELECT_SET, config.count.data(), config.start.data(),
                              config.stride.data(), nullptr);

    H5::DataSpace memorySpace(3, config.selectedDimensions.data());
    memorySpace.selectAll();

    if (memorySpace.getSelectNpoints() != dataSpace.getSelectNpoints()) {
        throw Exception{
            SourceContext{},
            "Invalid selection, source selection size {} not equal to destination memory size {}",
            memorySpace.getSelectNpoints(), dataSpace.getSelectNpoints()};
    }

    const DataFormatBase* format = type ? type : util::getDataFormatFromDataSet(dataset);

    // Reverse back the Column major
    auto volume =
        getVolume({.dimensions = size3_t{config.selectedDimensions[2], config.selectedDimensions[1],
                                         config.selectedDimensions[0]},
                   .format = format});
    auto* volumeRam = volume->getEditableRepresentation<VolumeRAM>();

    IgnoreValues ignore{};
    readValueAttributes(dataset, *volume, ignore);

    auto minmax = volumeRam->dispatch<std::pair<dvec4, dvec4>, dispatching::filter::Scalars>(
        [&](auto vrprecision) {
            using ValueType = ::inviwo::util::PrecisionValueType<decltype(vrprecision)>;

            ValueType* data = vrprecision->getDataTyped();

            try {
                dataset.read(data, TypeMap<ValueType>::getType(), memorySpace, dataSpace);
            } catch (H5::DataSetIException& e) {
                throw Exception(SourceContext{}, "HDF: unable to read data: {}", e.getDetailMsg());
            }

            return ::inviwo::util::dataMinMax(data, memorySpace.getSelectNpoints(), ignore);
        });

    volume->dataMap.dataRange.x = glm::compMin(minmax.first);
    volume->dataMap.dataRange.y = glm::compMax(minmax.second);
    volume->dataMap.valueRange = volume->dataMap.dataRange;
    volume->discardHistograms();

    log::info(
        "Read HDF Volume: source dims {}, selection dims: {} type: {} data "
        "range: {}, {} file: {}",
        fmt::join(config.dataDimensions, " x "), fmt::join(config.selectedDimensions, " x "),
        volume->getDataFormat()->getString(), minmax.first, minmax.second, dataset.getFileName());

    return volume;
}

std::shared_ptr<Layer> getLayerAtPathAsType(const Handle& handle, std::vector<Selection> selection,
                                            const DataFormatBase* type) {
    auto dataset = handle.open();
    const H5::DataSpace dataSpace = dataset.getSpace();

    const auto config = getDimConfig<2>(dataSpace, selection);
    dataSpace.selectHyperslab(H5S_SELECT_SET, config.count.data(), config.start.data(),
                              config.stride.data(), nullptr);

    H5::DataSpace memorySpace(2, config.selectedDimensions.data());
    memorySpace.selectAll();

    if (memorySpace.getSelectNpoints() != dataSpace.getSelectNpoints()) {
        throw Exception{
            SourceContext{},
            "Invalid selection, source selection size {} not equal to destination memory size {}",
            memorySpace.getSelectNpoints(), dataSpace.getSelectNpoints()};
    }

    const DataFormatBase* format = type ? type : util::getDataFormatFromDataSet(dataset);

    auto layer = std::make_shared<Layer>(LayerConfig{
        .dimensions = size2_t{config.selectedDimensions[1], config.selectedDimensions[0]},
        .format = format,
        .type = LayerType::Color});
    auto* layerRam = layer->getEditableRepresentation<LayerRAM>();

    IgnoreValues ignore{};
    readValueAttributes(dataset, *layer, ignore);

    auto minmax = layerRam->dispatch<std::pair<dvec4, dvec4>, dispatching::filter::Scalars>(
        [&](auto lrprecision) {
            using ValueType = ::inviwo::util::PrecisionValueType<decltype(lrprecision)>;

            ValueType* data = lrprecision->getDataTyped();

            try {
                dataset.read(data, TypeMap<ValueType>::getType(), memorySpace, dataSpace);
            } catch (H5::DataSetIException& e) {
                throw Exception(SourceContext{}, "HDF: unable to read data: {}", e.getDetailMsg());
            }

            return ::inviwo::util::dataMinMax(data, memorySpace.getSelectNpoints(), ignore);
        });

    layer->dataMap.dataRange.x = glm::compMin(minmax.first);
    layer->dataMap.dataRange.y = glm::compMax(minmax.second);
    layer->dataMap.valueRange = layer->dataMap.dataRange;
    layer->discardHistograms();

    log::info(
        "Read HDF Layer: source dims {}, selection dims: {} type: {} data "
        "range: {}, {} file: {}",
        fmt::join(config.dataDimensions, " x "), fmt::join(config.selectedDimensions, " x "),
        layer->getDataFormat()->getString(), minmax.first, minmax.second, dataset.getFileName());

    return layer;
}

std::shared_ptr<BufferBase> getBufferAtPathAsType(const Handle& handle,
                                                  std::vector<Selection> selection,
                                                  const DataFormatBase* type) {
    auto dataset = handle.open();

    const H5::DataSpace dataSpace = dataset.getSpace();
    const auto config = getDimConfig<1>(dataSpace, selection);

    dataSpace.selectHyperslab(H5S_SELECT_SET, config.count.data(), config.start.data(),
                              config.stride.data(), nullptr);
    H5::DataSpace memorySpace(1, config.selectedDimensions.data());
    memorySpace.selectAll();

    const DataFormatBase* format = type ? type : util::getDataFormatFromDataSet(dataset);

    auto buffer =
        dispatching::singleDispatch<std::shared_ptr<BufferBase>, dispatching::filter::Scalars>(
            format->getId(), [&]<typename T>() -> std::shared_ptr<BufferBase> {
                auto repr = std::make_shared<BufferRAMPrecision<T>>(
                    static_cast<size_t>(config.selectedDimensions[0]));
                try {
                    dataset.read(repr->getDataContainer().data(), TypeMap<T>::getType(),
                                 memorySpace, dataSpace);
                } catch (H5::DataSetIException& e) {
                    throw Exception(SourceContext{}, "HDF: unable to read data: {}",
                                    e.getDetailMsg());
                }
                log::info("Read HDF buffer type: {} size: {} file: {}", DataFormat<T>::str(),
                          config.selectedDimensions[0], dataset.getFileName());
                return std::make_shared<Buffer<T>>(repr);
            });

    log::info("Read HDF Buffer: source dims {}, selection dim {} type: {} file: {}",
              fmt::join(config.dataDimensions, " x "), fmt::join(config.selectedDimensions, " x "),
              buffer->getDataFormat()->getString(), dataset.getFileName());

    return buffer;
}

}  // namespace inviwo::hdf5
