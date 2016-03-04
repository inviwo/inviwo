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
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumerepresentation.h>
#include <inviwo/core/datastructures/histogram.h>

namespace inviwo {

class IVW_CORE_API VolumeRAM : public VolumeRepresentation {
public:
    VolumeRAM(const DataFormatBase* format = DataFormatBase::get());
    VolumeRAM(const VolumeRAM& rhs);
    VolumeRAM& operator=(const VolumeRAM& that);
    virtual VolumeRAM* clone() const override = 0;
    virtual ~VolumeRAM();

    virtual void* getData() = 0;
    virtual const void* getData() const = 0;
    virtual void* getData(size_t) = 0;
    virtual const void* getData(size_t) const = 0;

    /**
     * \brief Takes ownership of data pointer
     *
     * @param data is raw volume data pointer
     * @param dimensions is the dimensions of the data.
     * @return void none
     */
    virtual void setData(void* data, size3_t dimensions) = 0;
    virtual void removeDataOwnership() = 0;

    // Histograms
    virtual bool hasHistograms() const = 0;
    virtual HistogramContainer* getHistograms(size_t bins = 2048u, size3_t sampleRate = size3_t(1)) = 0;

    virtual const HistogramContainer* getHistograms(size_t bins = 2048u,
                                                    size3_t sampleRate = size3_t(1)) const = 0;
    virtual void calculateHistograms(size_t bins, size3_t sampleRate, const bool& stop) const = 0;

    // uniform getters and setters
    virtual double getAsDouble(const size3_t& pos) const = 0;
    virtual dvec2 getAsDVec2(const size3_t& pos) const = 0;
    virtual dvec3 getAsDVec3(const size3_t& pos) const = 0;
    virtual dvec4 getAsDVec4(const size3_t& pos) const = 0;
    
    virtual void setFromDouble(const size3_t& pos, double val) = 0;
    virtual void setFromDVec2(const size3_t& pos, dvec2 val) = 0;
    virtual void setFromDVec3(const size3_t& pos, dvec3 val) = 0;
    virtual void setFromDVec4(const size3_t& pos, dvec4 val) = 0;


    virtual double getAsNormalizedDouble(const size3_t& pos) const = 0;
    virtual dvec2 getAsNormalizedDVec2(const size3_t& pos) const = 0;
    virtual dvec3 getAsNormalizedDVec3(const size3_t& pos) const = 0;
    virtual dvec4 getAsNormalizedDVec4(const size3_t& pos) const = 0;

    virtual void setFromNormalizedDouble(const size3_t& pos, double val) = 0;
    virtual void setFromNormalizedDVec2(const size3_t& pos, dvec2 val) = 0;
    virtual void setFromNormalizedDVec3(const size3_t& pos, dvec3 val) = 0;
    virtual void setFromNormalizedDVec4(const size3_t& pos, dvec4 val) = 0;

    virtual void setValuesFromVolume(const VolumeRAM* src, const size3_t& dstOffset,
                                     const size3_t& subSize, const size3_t& subOffset) = 0;
    void setValuesFromVolume(const VolumeRAM* src, const size3_t& dstOffset = size3_t(0));

    virtual size_t getNumberOfBytes() const = 0;

    template <typename T>
    static T posToIndex(const glm::detail::tvec3<T, glm::defaultp>& pos,
                        const glm::detail::tvec3<T, glm::defaultp>& dim);
    template <typename T>
    static T periodicPosToIndex(const glm::detail::tvec3<T, glm::defaultp>& posIn,
                                const glm::detail::tvec3<T, glm::defaultp>& dim);

    virtual std::type_index getTypeIndex() const override final;
};

template <>
struct representation_traits<Volume, kind::RAM> {
    using type = VolumeRAM;
};

template <typename T>
T VolumeRAM::posToIndex(const glm::detail::tvec3<T, glm::defaultp>& pos,
                        const glm::detail::tvec3<T, glm::defaultp>& dim) {
    return pos.x + (pos.y * dim.x) + (pos.z * dim.x * dim.y);
}

template <typename T>
T VolumeRAM::periodicPosToIndex(const glm::detail::tvec3<T, glm::defaultp>& posIn,
                                const glm::detail::tvec3<T, glm::defaultp>& dim) {
    glm::detail::tvec3<T, glm::defaultp> pos = posIn % dim;
    return pos.x + (pos.y * dim.x) + (pos.z * dim.x * dim.y);
}

}  // namespace

#endif  // IVW_VOLUMERAM_H
