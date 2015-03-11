/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#ifndef IVW_VOLUMERAM_H
#define IVW_VOLUMERAM_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/volume/volumerepresentation.h>
#include <inviwo/core/datastructures/histogram.h>

namespace inviwo {

class IVW_CORE_API VolumeRAM : public VolumeRepresentation {

public:
    VolumeRAM(uvec3 dimensions= uvec3(128,128,128), const DataFormatBase* format = DataFormatBase::get());
    VolumeRAM(const VolumeRAM& rhs);
    VolumeRAM& operator=(const VolumeRAM& that);
    virtual VolumeRAM* clone() const = 0;
    virtual ~VolumeRAM();

    virtual void performOperation(DataOperation*) const = 0;
    virtual void initialize();
    virtual void deinitialize();

    void* getData();
    const void* getData() const;

    virtual void* getData(size_t) = 0;
    virtual const void* getData(size_t) const = 0;

    /**
     * \brief Takes ownership of data pointer
     *
     * @param void * data is raw volume data pointer
     * @return void none
     */
    void setData(void* data) {
        deinitialize();
        data_ = data;
    }

    void removeDataOwnership() {
        ownsDataPtr_ = false;
    }

    bool hasNormalizedHistogram() const;
    NormalizedHistogram* getNormalizedHistogram(int delta=-1, std::size_t maxNumberOfBins = 2048u, int component = 0);
    const NormalizedHistogram* getNormalizedHistogram(int delta = -1, std::size_t maxNumberOfBins = 2048u, int component = 0) const;

    virtual void setValueFromSingleDouble(const uvec3& pos, double val) = 0;
    virtual void setValueFromVec2Double(const uvec3& pos, dvec2 val) = 0;
    virtual void setValueFromVec3Double(const uvec3& pos, dvec3 val) = 0;
    virtual void setValueFromVec4Double(const uvec3& pos, dvec4 val) = 0;

    virtual void setValuesFromVolume(const VolumeRAM* src, const uvec3& dstOffset, const uvec3& subSize, const uvec3& subOffset) = 0;
    void setValuesFromVolume(const VolumeRAM* src, const uvec3& dstOffset = uvec3(0));

    virtual double getValueAsSingleDouble(const uvec3& pos) const = 0;
    virtual dvec2 getValueAsVec2Double(const uvec3& pos) const = 0;
    virtual dvec3 getValueAsVec3Double(const uvec3& pos) const = 0;
    virtual dvec4 getValueAsVec4Double(const uvec3& pos) const = 0;

    size_t getNumberOfBytes() const;

    template <typename T>
    static inline T posToIndex(const glm::detail::tvec3<T, glm::defaultp>& pos,
                               const glm::detail::tvec3<T, glm::defaultp>& dim) {
        return pos.x + (pos.y * dim.x) + (pos.z * dim.x * dim.y);
    }
    template <typename T>
    static inline T periodicPosToIndex(const glm::detail::tvec3<T, glm::defaultp>& posIn,
                                       const glm::detail::tvec3<T, glm::defaultp>& dim) {
        glm::detail::tvec3<T, glm::defaultp> pos = posIn % dim;
        return pos.x + (pos.y * dim.x) + (pos.z * dim.x * dim.y);
    }

    bool shouldStopHistogramCalculation() const { return stopHistogramCalculation_; }
    void stopHistogramCalculation() const { stopHistogramCalculation_ = true; }

protected:
    void calculateHistogram(int delta, std::size_t maxNumberOfBins) const;

    void* data_;
    bool ownsDataPtr_;

    mutable std::vector<NormalizedHistogram*>* histograms_;
    mutable bool calculatingHistogram_;
    mutable bool stopHistogramCalculation_;
};


/**
 * Factory for volumes.
 * Creates an VolumeRAM with data type specified by format.
 *
 * @param dimensionsof volume to create.
 * @param format of volume to create.
 * @return NULL if no valid format was specified.
 */
IVW_CORE_API VolumeRAM* createVolumeRAM(const uvec3& dimensions, const DataFormatBase* format, void* dataPtr = NULL);

} // namespace

#endif // IVW_VOLUMERAM_H
