/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/volumeport.h>
#include <modules/discretedata/properties/datachannelproperty.h>
#include <type_traits>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>

#include <modules/discretedata/ports/datasetport.h>

#include <inviwo/core/properties/boolproperty.h>

namespace inviwo {
namespace discretedata {

/** \docpage{org.inviwo.VolumeFromDataSet, Volume From Data Set}
    ![](org.inviwo.VolumeFromDataSet.png?classIdentifier=org.inviwo.VolumeFromDataSet)

    Converts a DataSet into a Volume.

    ### Inports
      * __InDataSet__ Input DataSet to be converted.

    ### Outports
      * __OutVolume__ Converted volume.
*/

/** \class VolumeFromDataSet
    \brief Converts a DataSet to a Volume

    Requires a Structured Grid in the data set.
    Fails if that is not given.
*/
class IVW_MODULE_DISCRETEDATA_API VolumeFromDataSet : public Processor {
    // Construction / Deconstruction
public:
    VolumeFromDataSet();
    virtual ~VolumeFromDataSet() = default;

    // Methods
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    /// Our main computation function
    virtual void process() override;

    // void updateChannelList();

    template <typename T>
    inviwo::Volume* convert(const Channel& channel, const StructuredGrid<3>& grid);

    template <typename T, typename To = T>
    inviwo::Volume* convertTo(const Channel& channel, const StructuredGrid<3>& grid);

    // Ports
public:
    /// Input dataset
    DataSetInport portInDataSet;

    /// Output volume
    VolumeOutport portOutVolume;

    DataChannelProperty channelName;

    BoolProperty floatVolumeOutput;
};

template <typename T, typename To>
inviwo::Volume* VolumeFromDataSet::convertTo(const Channel& channel,
                                             const StructuredGrid<3>& grid) {

    const BufferChannel<T, 1>* buffer = dynamic_cast<const BufferChannel<T, 1>*>(&channel);
    if (!buffer) {
        LogWarn("Not a scalar");
        return nullptr;
    }

    ind offset = channelName.gridPrimitive_.get() == GridPrimitive::Vertex ? 0 : -1;
    ivec3 size(grid.getNumVerticesInDimension(0) + offset,
               grid.getNumVerticesInDimension(1) + offset,
               grid.getNumVerticesInDimension(2) + offset);

    ivwAssert(buffer->size() == size.x * size.y * size.z, "Inconsistency with buffer sizes.");

    To* data = new To[buffer->size()];
    if (std::is_same<T, To>()) {
        memcpy(data, buffer->data().data(), buffer->size() * sizeof(T));
    } else {
        for (ind i = 0; i < buffer->size(); ++i) {
            data[i] = static_cast<To>(buffer->get(i));
            if (!glm::isfinite<float>((float)data[i])) data[i] = (To)(-1);
        }
    }

    auto ramData = std::make_shared<VolumeRAMPrecision<To>>(data, size3_t(size.x, size.y, size.z));

    auto* volume = new inviwo::Volume(ramData);

    T min, max;
    buffer->getMinMax(min, max);
    volume->dataMap_.dataRange = dvec2(min, max);
    volume->dataMap_.valueRange = dvec2(min, max);
    mat3 baseMat(size.x, 0, 0, 0, size.y, 0, 0, 0, size.z);
    baseMat /= size.x;
    volume->setBasis(baseMat);
    return volume;
}

template <typename T>
inviwo::Volume* VolumeFromDataSet::convert(const Channel& channel, const StructuredGrid<3>& grid) {

    if (floatVolumeOutput.get())
        return convertTo<T, float>(channel, grid);
    else
        return convertTo<T, T>(channel, grid);
}

}  // namespace discretedata
}  // namespace inviwo
