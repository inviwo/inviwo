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

#include <modules/hdf5/hdf5moduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <modules/hdf5/hdf5utils.h>
#include <modules/hdf5/properties/dimselectionsproperty.h>
#include <inviwo/core/datastructures/volume/temporalvolume.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/directoryproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/ordinaloptproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/stringsproperty.h>
#include <inviwo/core/datastructures/image/imagetypes.h>

#include <filesystem>
#include <vector>

namespace inviwo::hdf5 {

/**
 * @brief Loads a time-varying volume from a folder of HDF5 files as a lazily-loaded
 * TemporalVolume.
 *
 * All files in the folder matching the include/exclude regular expressions are sorted by name,
 * and the time steps of the selected dataset (rank >= 4) are concatenated across the files in that
 * order. All files are assumed to share the same structure; the dataset list, basis, and data
 * range options are taken from the first file. Files are only opened when a frame from them is
 * requested, a file with a mismatching dataset will fail at that point.
 *
 * @see HDF5ToTemporalVolume, HDF5MultiFileTemporalVolumeLoader, TemporalVolume
 */
class IVW_MODULE_HDF5_API HDF5DirectoryToTemporalVolume : public Processor {
public:
    HDF5DirectoryToTemporalVolume();
    HDF5DirectoryToTemporalVolume(const HDF5DirectoryToTemporalVolume&) = delete;
    HDF5DirectoryToTemporalVolume& operator=(const HDF5DirectoryToTemporalVolume&) = delete;
    HDF5DirectoryToTemporalVolume(HDF5DirectoryToTemporalVolume&&) = delete;
    HDF5DirectoryToTemporalVolume& operator=(HDF5DirectoryToTemporalVolume&&) = delete;
    virtual ~HDF5DirectoryToTemporalVolume();

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void process() override;

private:
    std::vector<std::filesystem::path> getFiles() const;

    std::vector<std::filesystem::path> files_;
    std::vector<DataSetInfo> volumeMatches_;
    std::vector<DataSetInfo> basisMatches_;
    std::vector<DataSetInfo> dataRangeMatches_;

    TemporalVolumeOutport outport_;

    DirectoryProperty folder_;
    StringProperty include_;
    StringProperty exclude_;
    ButtonProperty reload_;
    IntSizeTProperty fileCount_;

    OptionPropertyString volumeSelection_;

    CompositeProperty config_;

    CompositeProperty basisGroup_;
    OptionPropertyString basisSelection_;
    DoubleMat4Property customBasis_;
    DoubleVec3Property customSpacing_;
    BoolProperty transposeBasis_;
    BoolProperty swapBasisAxes_;
    DoubleMat4Property basis_;

    CompositeProperty dataRangeGroup_;
    OptionPropertyString dataRangeSelection_;
    OptionPropertyString valueRangeSelection_;
    DoubleVec2Property dataRange_;
    DoubleVec2Property valueRange_;
    OrdinalOptProperty<dvec2> customDataRange_;
    OrdinalOptProperty<dvec2> customValueRange_;

    StringProperty valueName_;
    StringProperty valueUnit_;
    OptionProperty<InterpolationType> interpolation_;
    StringsProperty<3> axesNames_;
    StringsProperty<3> axesUnits_;
    std::array<OptionProperty<Wrapping>, 3> wrapping_;

    CompositeProperty outputGroup_;
    OptionPropertyInt datatype_;
    BoolProperty adjustBasis_;
    BoolProperty adjustOffset_;
    DimSelectionsProperty selection_;

    CompositeProperty timeGroup_;
    OptionProperty<size_t> timeDimension_;
    DoubleProperty dt_;
    IntSizeTProperty cacheSize_;
};

}  // namespace inviwo::hdf5
