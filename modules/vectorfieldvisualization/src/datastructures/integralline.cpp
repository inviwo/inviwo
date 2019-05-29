/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2019 Inviwo Foundation
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

#include <modules/vectorfieldvisualization/datastructures/integralline.h>
#include <inviwo/core/util/interpolation.h>

namespace inviwo {

const std::vector<dvec3> &IntegralLine::getPositions() const { return positions_; }
std::vector<dvec3> &IntegralLine::getPositions() { return positions_; }

std::shared_ptr<const BufferBase> IntegralLine::getMetaDataBuffer(const std::string &name) const {
    auto it = metaData_.find(name);
    if (it == metaData_.end()) {
        throw Exception("No meta data with name: " + name, IVW_CONTEXT);
    }
    return it->second;
}

std::shared_ptr<BufferBase> IntegralLine::getMetaDataBuffer(const std::string &name) {
    auto it = metaData_.find(name);
    if (it == metaData_.end()) {
        throw Exception("No meta data with name: " + name, IVW_CONTEXT);
    }
    return it->second;
}

void IntegralLine::addMetaDataBuffer(const std::string &name, std::shared_ptr<BufferBase> buffer) {
    auto it = metaData_.find(name);
    if (it != metaData_.end()) {
        throw Exception("Meta data with name already exists: " + name, IVW_CONTEXT);
    }
    metaData_[name] = buffer;
}

void IntegralLine::reverse() {
    std::reverse(positions_.begin(), positions_.end());
    for (auto &m : metaData_) {
        util::reverse(*m.second);
    }
}

const std::map<std::string, std::shared_ptr<BufferBase>> &IntegralLine::getMetaDataBuffers() const {
    return metaData_;
}

bool IntegralLine::hasMetaData(const std::string &name) const {
    auto it = metaData_.find(name);
    return it != metaData_.end();
}

std::vector<std::string> IntegralLine::getMetaDataKeys() const {
    std::vector<std::string> keys;
    for (auto &m : metaData_) {
        keys.push_back(m.first);
    }
    return keys;
}

double IntegralLine::getLength() const {
    if (length_ == -1) {
        length_ = 0;
        if (positions_.size() > 1) {
            auto next = positions_.begin();
            auto prev = next++;
            while (next != positions_.end()) {
                length_ += glm::distance(*prev, *next);
                prev = next++;
            }
        }
    }
    return length_;
}

double IntegralLine::distBetweenPoints(size_t a, size_t b) const {
    if (a == b) return 0;
    if (a > b) return distBetweenPoints(b, a);
    return calcLength(positions_.begin() + a, positions_.begin() + b + 1);
}

dvec3 IntegralLine::getPointAtDistance(double d) const {
    if (d < 0 || d > getLength()) {
        return dvec3(0);
    }
    if (d == 0) {
        return positions_.front();
    }

    double distPrev = 0, distNext = 0;
    auto next = positions_.begin();
    auto prev = next;
    while (distNext < d) {
        prev = next++;
        distPrev = distNext;
        distNext += glm::distance(*prev, *next);
    }

    double x = (d - distPrev) / (distNext - distPrev);
    return Interpolation<dvec3, double>::linear(*prev, *next, x);
}

size_t IntegralLine::getIndex() const { return idx_; }

void IntegralLine::setIndex(size_t idx) { idx_ = idx; }

void IntegralLine::setBackwardTerminationReason(TerminationReason terminationReason) {
    backwardTerminationReason_ = terminationReason;
}

void IntegralLine::setForwardTerminationReason(TerminationReason terminationReason) {
    forwardTerminationReason_ = terminationReason;
}

IntegralLine::TerminationReason IntegralLine::getBackwardTerminationReason() const {
    return forwardTerminationReason_;
}

IntegralLine::TerminationReason IntegralLine::getForwardTerminationReason() const {
    return backwardTerminationReason_;
}

double IntegralLine::calcLength(std::vector<dvec3>::const_iterator start,
                                std::vector<dvec3>::const_iterator end) const {
    auto length = 0.0;
    if (positions_.size() > 1) {
        auto next = start;
        auto prev = next++;
        while (next != end) {
            length += glm::distance(*prev, *next);
            prev = next++;
        }
    }
    return length;
}

}  // namespace inviwo
