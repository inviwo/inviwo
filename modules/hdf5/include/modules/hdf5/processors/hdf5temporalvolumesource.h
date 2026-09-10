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
#include <modules/hdf5/ports/hdf5port.h>
#include <modules/hdf5/hdf5utils.h>
#include <modules/hdf5/properties/dimselectionsproperty.h>
#include <inviwo/core/datastructures/volume/temporalvolume.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/compositeproperty.h>

#include <modules/base/properties/volumeinformationproperty.h>

#include <vector>

namespace inviwo::hdf5 {

/**
 * @brief Loads a time-varying volume from a HDF5 dataset of rank >= 4 as a lazily-loaded
 * TemporalVolume.
 *
 * Exactly three dimensions of the selected dataset become the volume axes (X/Y/Z); the user picks
 * one of the remaining leading dimensions to act as the time axis. Any further leading dimensions
 * must be fixed to a single index, same as HDF5ToVolume does for datasets with rank > 3.
 *
 * @see HDF5ToVolume, TemporalVolume
 */
class IVW_MODULE_HDF5_API HDF5ToTemporalVolume : public Processor {
public:
    HDF5ToTemporalVolume();
    HDF5ToTemporalVolume(const HDF5ToTemporalVolume&) = delete;
    HDF5ToTemporalVolume& operator=(const HDF5ToTemporalVolume&) = delete;
    HDF5ToTemporalVolume(HDF5ToTemporalVolume&&) = delete;
    HDF5ToTemporalVolume& operator=(HDF5ToTemporalVolume&&) = delete;
    virtual ~HDF5ToTemporalVolume();

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void process() override;

private:
    void onSelectionChange();

    dmat4 getBasisFromMeta(const DataSetInfo& meta);
    dmat4 computeBasis(const DataSetInfo& volumeInfo);

    std::vector<DataSetInfo> volumeMatches_;
    std::vector<DataSetInfo> basisMatches_;

    Inport inport_;
    TemporalVolumeOutport outport_;

    OptionPropertyString volumeSelection_;

    CompositeProperty basisGroup_;
    OptionPropertyString basisSelection_;
    DoubleMat4Property basis_;
    DoubleVec3Property spacing_;

    CompositeProperty outputGroup_;
    OptionPropertyInt datatype_;
    BoolProperty adjustBasis_;
    BoolProperty adjustOffset_;
    DimSelectionsProperty selection_;

    CompositeProperty timeGroup_;
    OptionProperty<size_t> timeDimension_;
    DoubleProperty dt_;
    IntSizeTProperty cacheSize_;

    bool dirty_;
};

}  // namespace inviwo::hdf5
