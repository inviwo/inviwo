/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2025 Inviwo Foundation
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

#include <modules/vectorfieldvisualization/vectorfieldvisualizationmoduledefine.h>  // for IVW_M...

#include <inviwo/core/datastructures/buffer/buffer.h>  // for Buffer
#include <inviwo/core/util/exception.h>                // for Excep...
#include <inviwo/core/util/formats.h>                  // for DataF...
#include <inviwo/core/util/glmutils.h>                 // for same_...
#include <inviwo/core/util/glmvec.h>                   // for dvec3
#include <inviwo/core/util/interpolation.h>            // for Inter...
#include <inviwo/core/util/sourcecontext.h>            // for IVW_C...

#include <cstddef>      // for size_t
#include <cstdint>      // for uint32_t
#include <map>          // for opera...
#include <memory>       // for share...
#include <sstream>      // for opera...
#include <string>       // for string
#include <type_traits>  // for remov...
#include <utility>      // for pair
#include <vector>       // for vector

#include <glm/geometric.hpp>  // for distance
#include <glm/vec3.hpp>       // for opera...

namespace inviwo {

class IVW_MODULE_VECTORFIELDVISUALIZATION_API IntegralLine {
public:
    enum class TerminationReason { StartPoint, Steps, OutOfBounds, ZeroVelocity, Unknown };

    IntegralLine() = default;
    IntegralLine(const IntegralLine& rhs) = default;
    IntegralLine(IntegralLine&& rhs) = default;

    IntegralLine& operator=(const IntegralLine& that) = default;
    IntegralLine& operator=(IntegralLine&& that) = default;

    virtual ~IntegralLine() = default;

    const std::vector<dvec3>& getPositions() const;
    std::vector<dvec3>& getPositions();

    std::shared_ptr<const BufferBase> getMetaDataBuffer(const std::string& name) const;
    std::shared_ptr<BufferBase> getMetaDataBuffer(const std::string& name);

    void addMetaDataBuffer(const std::string& name, std::shared_ptr<BufferBase> buffer);

    template <typename T>
    std::shared_ptr<Buffer<T>> createMetaData(const std::string& name);

    void reverse();

    template <typename T>
    const std::vector<T>& getMetaData(const std::string& name) const;

    template <typename T>
    std::vector<T>& getMetaData(const std::string& name, bool create = false);

    const std::map<std::string, std::shared_ptr<BufferBase>>& getMetaDataBuffers() const;

    bool hasMetaData(const std::string& name) const;

    std::vector<std::string> getMetaDataKeys() const;

    double getLength() const;

    double distBetweenPoints(size_t a, size_t b) const;

    dvec3 getPointAtDistance(double d) const;

    template <typename T>
    T getMetaDataAtDistance(std::string md, double d) const;

    uint32_t getIndex() const;
    void setIndex(uint32_t idx);

    void setBackwardTerminationReason(TerminationReason terminationReason);
    void setForwardTerminationReason(TerminationReason terminationReason);

    TerminationReason getBackwardTerminationReason() const;
    TerminationReason getForwardTerminationReason() const;

private:
    double calcLength(std::vector<dvec3>::const_iterator start,
                      std::vector<dvec3>::const_iterator end) const;

    std::vector<dvec3> positions_;
    std::map<std::string, std::shared_ptr<BufferBase>> metaData_;

    TerminationReason forwardTerminationReason_ = TerminationReason::Unknown;
    TerminationReason backwardTerminationReason_ = TerminationReason::Unknown;

    mutable double length_ = -1;  // length is only calculated on demand hence it need to be mutable

    uint32_t idx_;
};

template <typename T>
std::shared_ptr<Buffer<T>> IntegralLine::createMetaData(const std::string& name) {
    if (hasMetaData(name)) {
        throw Exception(SourceContext{}, "Meta data with name {} already exists", name);
    }
    auto md = std::make_shared<Buffer<T>>();
    metaData_[name] = md;
    return md;
}

template <typename T>
const std::vector<T>& IntegralLine::getMetaData(const std::string& name) const {
    auto it = metaData_.find(name);
    if (it == metaData_.end()) {
        throw Exception(SourceContext{}, "No meta data with name: {}", name);
    }
    auto askedDF = DataFormat<T>::get();
    auto isDF = it->second->getDataFormat();
    if (isDF != askedDF) {
        throw Exception(SourceContext{},
                        "Incorrect dataformat for meta data {} asking for {} but is {}", name,
                        askedDF->getString(), isDF->getString());
    }

    return static_cast<Buffer<T>*>(it->second.get())->getRAMRepresentation()->getDataContainer();
}

template <typename T>
std::vector<T>& IntegralLine::getMetaData(const std::string& name, bool create) {
    auto it = metaData_.find(name);
    if (it == metaData_.end() && !create) {
        throw Exception(SourceContext{}, "No meta data with name: {}", name);
    } else if (it == metaData_.end()) {
        auto md = createMetaData<T>(name);
        return md->getEditableRAMRepresentation()->getDataContainer();
    }
    auto askedDF = DataFormat<T>::get();
    auto isDF = it->second->getDataFormat();
    if (isDF != askedDF) {
        throw Exception(SourceContext{},
                        "Incorrect dataformat for meta data {} asking for {} but is {}", name,
                        askedDF->getString(), isDF->getString());
    }

    return static_cast<Buffer<T>*>(it->second.get())
        ->getEditableRAMRepresentation()
        ->getDataContainer();
}

template <typename T>
T IntegralLine::getMetaDataAtDistance(std::string md, double d) const {
    if (d < 0 || d > getLength()) {
        return T(0);
    }

    if (!hasMetaData(md)) {
        return T(0);
    }

    auto& metaData = getMetaData<T>(md);

    if (d == 0) {
        return metaData.front();
    }

    double distPrev = 0, distNext = 0;
    auto next = positions_.begin();
    auto prev = next;
    auto nextMD = metaData.begin();
    auto prevMD = nextMD;

    while (distNext < d) {
        prev = next++;
        prevMD = nextMD++;
        distPrev = distNext;
        distNext += glm::distance(*prev, *next);
    }

    double x = (d - distPrev) / (distNext - distPrev);
    using TV = typename util::same_extent<T, double>::type;

    return static_cast<T>(
        Interpolation<TV, double>::linear(static_cast<TV>(*prevMD), static_cast<TV>(*nextMD), x));
}

IVW_MODULE_VECTORFIELDVISUALIZATION_API std::ostream& operator<<(
    std::ostream& os, IntegralLine::TerminationReason reason);

}  // namespace inviwo
