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

#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/formatdispatching.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>

#include <modules/hdf5/hdf5types.h>
#include <modules/hdf5/hdf5exception.h>

#include <modules/base/algorithm/algorithmoptions.h>

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

    auto asSelection() const {
        return std::views::zip(start, count, stride) | std::views::transform([](const auto& item) {
                   return Selection{.start = std::get<0>(item),
                                    .count = std::get<1>(item),
                                    .stride = std::get<2>(item)};
               });
    }
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
    // Extend with 1 element dims to correct rank.
    config.selectedDimensions.resize(N, 1uz);

    return config;
}

VolumeConfig getVolumeConfig(const DataSet& dataset, const DimConfig<3>& config,
                             VolumeConfig volumeConfig) {
    // Reverse back the Column major
    volumeConfig.dimensions = size3_t{config.selectedDimensions[2], config.selectedDimensions[1],
                                      config.selectedDimensions[0]};
    volumeConfig.format =
        volumeConfig.format ? volumeConfig.format : util::getDataFormatFromDataSet(dataset);
    return volumeConfig;
}

LayerConfig getLayerConfig(const DataSet& dataset, const DimConfig<2>& config,
                           LayerConfig layerConfig) {

    // Reverse back the Column major
    layerConfig.dimensions = size2_t{config.selectedDimensions[1], config.selectedDimensions[0]};
    layerConfig.format =
        layerConfig.format ? layerConfig.format : util::getDataFormatFromDataSet(dataset);

    return layerConfig;
}

}  // namespace

std::pair<VolumeConfig, Selection> getTemporalVolumeConfig(const Handle& handle,
                                                           std::vector<Selection> selection,
                                                           size_t timeDimension,
                                                           VolumeConfig volumeConfig) {
    auto dataset = handle.open();

    if (timeDimension > selection.size()) {
        throw Exception{SourceContext{}, "Invalid time index, must be less than {}",
                        selection.size()};
    }
    auto timeSelection = selection[timeDimension];
    selection[timeDimension].count = 1;

    const auto config = getDimConfig<3>(dataset.getSpace(), selection);

    timeSelection = clamp(timeSelection, std::views::reverse(config.dataDimensions)[timeDimension]);
    volumeConfig = getVolumeConfig(dataset, config, std::move(volumeConfig));

    return {volumeConfig, timeSelection};
}

VolumeConfig getVolumeConfig(const Handle& handle, const std::vector<Selection>& selection,
                             VolumeConfig volumeConfig) {
    auto dataset = handle.open();
    const auto config = getDimConfig<3>(dataset.getSpace(), selection);
    return getVolumeConfig(dataset, config, std::move(volumeConfig));
}
LayerConfig getLayerConfig(const Handle& handle, const std::vector<Selection>& selection,
                           LayerConfig layerConfig) {
    auto dataset = handle.open();
    const auto config = getDimConfig<2>(dataset.getSpace(), selection);
    return getLayerConfig(dataset, config, std::move(layerConfig));
}

std::shared_ptr<Volume> getVolumeAtPathAsType(
    const Handle& handle, const std::vector<Selection>& selection, VolumeConfig volumeConfig,
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

    auto volume = getVolume(getVolumeConfig(dataset, config, std::move(volumeConfig)));
    auto* volumeRam = volume->getEditableRepresentation<VolumeRAM>();

    IgnoreValues ignore{};
    readValueAttributes(dataset, *volume, ignore);

    volumeRam->dispatch<void, dispatching::filter::Scalars>([&](auto vrprecision) {
        using ValueType = ::inviwo::util::PrecisionValueType<decltype(vrprecision)>;
        auto data = vrprecision->getView();
        try {
            dataset.read(data.data(), TypeMap<ValueType>::getType(), memorySpace, dataSpace);
        } catch (H5::DataSetIException& e) {
            throw Exception(SourceContext{}, "HDF: unable to read data: {}", e.getDetailMsg());
        }
    });
    volume->discardHistograms();

    log::info("Read HDF Volume: Dimensions {}, Selection: {}, Type: {}, File: {}",
              fmt::join(config.dataDimensions, " x "), fmt::join(config.asSelection(), " x "),
              volume->getDataFormat()->getString(), dataset.getFileName());

    return volume;
}

std::shared_ptr<Layer> getLayerAtPathAsType(const Handle& handle,
                                            const std::vector<Selection>& selection,
                                            LayerConfig layerConfig) {
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

    auto layer = std::make_shared<Layer>(getLayerConfig(dataset, config, std::move(layerConfig)));
    auto* layerRam = layer->getEditableRepresentation<LayerRAM>();

    IgnoreValues ignore{};
    readValueAttributes(dataset, *layer, ignore);

    layerRam->dispatch<void, dispatching::filter::Scalars>([&](auto lrprecision) {
        using ValueType = ::inviwo::util::PrecisionValueType<decltype(lrprecision)>;

        auto data = lrprecision->getView();
        try {
            dataset.read(data.data(), TypeMap<ValueType>::getType(), memorySpace, dataSpace);
        } catch (H5::DataSetIException& e) {
            throw Exception(SourceContext{}, "HDF: unable to read data: {}", e.getDetailMsg());
        }
    });

    layer->discardHistograms();

    log::info("Read HDF Layer: Dimensions {}, Selection: {}, Type: {}, File: {}",
              fmt::join(config.dataDimensions, " x "), fmt::join(config.asSelection(), " x "),
              layer->getDataFormat()->getString(), dataset.getFileName());

    return layer;
}

std::shared_ptr<BufferBase> getBufferAtPathAsType(const Handle& handle,
                                                  const std::vector<Selection>& selection,
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
                return std::make_shared<Buffer<T>>(repr);
            });

    log::info("Read HDF Buffer: Dimensions {}, Selection: {}, Type: {}, File: {}",
              fmt::join(config.dataDimensions, " x "), fmt::join(config.asSelection(), " x "),
              buffer->getDataFormat()->getString(), dataset.getFileName());

    return buffer;
}

glm::dmat4 getBasis(const Handle& handle) {
    glm::dmat4 basis{1.0};
    auto dataset = handle.open();
    const H5::DataSpace space = dataset.getSpace();
    const int rank = space.getSimpleExtentNdims();
    if (rank != 2) {
        throw Exception(SourceContext{}, "Could not create Basis from: {} Invalid rank",
                        handle.getPath().toString());
    }
    std::vector<hsize_t> dims(rank);
    space.getSimpleExtentDims(dims.data());

    static constexpr std::array<size_t, 2> basisDim{3, 3};
    static constexpr std::array<size_t, 2> basisAndOffsetDim{4, 4};

    if (std::ranges::equal(dims, basisDim)) {
        dmat3 bas;
        dataset.read(glm::value_ptr(bas), H5::PredType::NATIVE_DOUBLE);
        basis = dmat4{bas};
        const auto offset = -0.5 * (bas[0] + bas[1] + bas[2]);
        basis[3] = dvec4{offset, 1.0};

    } else if (std::ranges::equal(dims, basisAndOffsetDim)) {
        dataset.read(glm::value_ptr(basis), H5::PredType::NATIVE_DOUBLE);
    } else {
        throw Exception(SourceContext{}, "Could not create Basis from: {} Invalid dimensions",
                        handle.getPath().toString());
    }
    return basis;
}

}  // namespace inviwo::hdf5
