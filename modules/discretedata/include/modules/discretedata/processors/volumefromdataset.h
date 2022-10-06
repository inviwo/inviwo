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
#include <inviwo/core/processors/poolprocessor.h>

#include <inviwo/core/ports/volumeport.h>
#include <modules/discretedata/ports/datasetport.h>

#include <modules/discretedata/properties/datachannelproperty.h>
#include <modules/discretedata/properties/datasamplerproperty.h>
#include <inviwo/core/properties/boolproperty.h>

#include <modules/discretedata/sampling/datasetspatialsampler.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <modules/discretedata/util/util.h>
#include <type_traits>

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

    Several methods to convert:
    UniformGrid: Requires a Structured Grid in the data set. Fails if that is not given.
    Sample: A DataSetSampler needs to be given.
*/
class IVW_MODULE_DISCRETEDATA_API VolumeFromDataSet : public PoolProcessor {
    // Construction / Deconstruction
public:
    VolumeFromDataSet();
    virtual ~VolumeFromDataSet() = default;

    // Methods
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void process() override;

public:
    DataSetInport portInDataSet_;
    VolumeOutport portOutVolume_;

    enum ConversionMethod { UniformGrid, Sample };
    TemplateOptionProperty<ConversionMethod> conversionMethod_;
    DataChannelProperty channelToConvert_;
    DataSamplerProperty sampler_;
    IntSize3Property volumeSize_;
    DoubleProperty invalidValue_;
    BoolProperty floatVolumeOutput_;
};

namespace detail {

struct VolumeDispatcher {

    template <typename T, typename To>
    static Volume* convertChannel(const Channel* channel, const StructuredGrid<3>& grid) {

        const BufferChannel<T, 1>* buffer = dynamic_cast<const BufferChannel<T, 1>*>(channel);
        if (!buffer) {
            return nullptr;
        }

        ind offset = buffer->getGridPrimitiveType() == GridPrimitive::Vertex ? 0 : -1;
        ivec3 size(grid.getNumVerticesInDimension(0) + offset,
                   grid.getNumVerticesInDimension(1) + offset,
                   grid.getNumVerticesInDimension(2) + offset);

        ivwAssert(buffer->size() == size.x * size.y * size.z, "Inconsistency with buffer sizes.");

        To* data = new To[buffer->size()];
        if constexpr (std::is_same<T, To>()) {
            memcpy(data, buffer->data().data(), buffer->size() * sizeof(T));
        } else {
            for (ind i = 0; i < buffer->size(); ++i) {
                data[i] = static_cast<To>(buffer->get(i));
                if (!glm::isfinite<float>((float)data[i])) data[i] = (To)(-1);
            }
        }

        auto ramData =
            std::make_shared<VolumeRAMPrecision<To>>(data, size3_t(size.x, size.y, size.z));

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

    template <typename T, ind N>
    Volume* sample(const std::shared_ptr<const DataChannel<T, N>>& channel,
                   const std::shared_ptr<const DataSetSampler<3>>& sampler, size3_t numVertices,
                   double invalidValue) {
        using VecTN = typename inviwo::Vector<N, T>;
        using VecMinMax = typename glm::vec<N, T>;
        using VecPos = typename glm::vec<3, double>;

        size3_t volumeSize = numVertices;
        for (size_t i = 0; i < 3; ++i) volumeSize[i] = std::max(volumeSize[i], size_t(32));

        auto ramVolume = std::make_shared<VolumeRAMPrecision<VecTN>>(volumeSize);
        VecTN* ramData = ramVolume->getDataTyped();

        DataSetSpatialSampler<3, N, T> spatialSampler(sampler, InterpolationType::Linear, channel,
                                                      invalidValue);

        // In case we do not fill the minimum volume size,
        // the untouched part should be set to the invalid value selected by the user.
        if (volumeSize[0] < size_t(32) || volumeSize[1] < size_t(32) ||
            volumeSize[2] < size_t(32)) {
            std::fill(ramData, ramData + (volumeSize[0] * volumeSize[1] * volumeSize[2]),
                      VecTN(invalidValue));
        }

        for (size_t z = 0; z < numVertices.z; ++z) {
            for (size_t y = 0; y < numVertices.y; ++y)
                for (size_t x = 0; x < numVertices.x; ++x) {
                    VecPos samplePos = {double(x) / (numVertices.x - 1),
                                        double(y) / (numVertices.y - 1),
                                        double(z) / (numVertices.z - 1)};

                    ramData[x + y * numVertices.x + z * numVertices.x * numVertices.y] =
                        spatialSampler.sampleDataSpace(samplePos);
                }
            std::cout << "++ z = " << z << std::endl;
        }

        // Make a volume.
        auto* volume = new inviwo::Volume(ramVolume);

        VecMinMax min, max;
        channel->getMinMax(min, max);
        volume->dataMap_.dataRange = dvec2(double(glm::compMin(min)), double(glm::compMax(max)));
        volume->dataMap_.valueRange = volume->dataMap_.dataRange;

        volume->setModelMatrix(sampler->getModelMatrix());
        return volume;
    }

    template <typename T, ind N>
    Volume* operator()(const std::shared_ptr<const DataChannel<T, N>> channel,
                       const std::shared_ptr<const DataSetSamplerBase>& sampler,
                       const Connectivity* grid, size3_t numVertices, double invalidValue,
                       VolumeFromDataSet::ConversionMethod method, bool toFloat) {
        const auto sampler3D = std::dynamic_pointer_cast<const DataSetSampler<3>>(sampler);
        const auto grid3D = dynamic_cast<const StructuredGrid<3>*>(grid);

        switch (method) {  // processor.conversionMethod_.get()) {
            case VolumeFromDataSet::ConversionMethod::UniformGrid:
                if constexpr (N != 1) return nullptr;
                if (!grid3D) return nullptr;

                if (toFloat)  // processor.floatVolumeOutput_.get())
                    return convertChannel<T, float>(channel.get(), *grid3D);
                else
                    return convertChannel<T, T>(channel.get(), *grid3D);

            case VolumeFromDataSet::ConversionMethod::Sample:
                if (!sampler3D || channel->getGridPrimitiveType() != GridPrimitive::Vertex)
                    return nullptr;
                return sample(channel, sampler3D, numVertices, invalidValue);

            default:
                return nullptr;
        }
    }
};

}  // namespace detail

}  // namespace discretedata
}  // namespace inviwo
