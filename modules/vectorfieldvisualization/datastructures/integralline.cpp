/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2017 Inviwo Foundation
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

#include "integralline.h"
#include <inviwo/core/util/interpolation.h>

namespace inviwo {

IntegralLine::IntegralLine()
    : positions_(), metaData_(), terminationReason_(TerminationReason::Steps) , length_(-1)
{}

IntegralLine::IntegralLine(const IntegralLine &rhs)
    : positions_(rhs.positions_)
    , metaData_(rhs.metaData_)
    , terminationReason_(rhs.terminationReason_)
    , length_(rhs.length_)
    , idx_(rhs.idx_) {}

inviwo::IntegralLine &IntegralLine::operator=(const IntegralLine &that) {
    if (&that != this) {
        positions_ = that.positions_;
        metaData_ = that.metaData_;
        terminationReason_ = that.terminationReason_;
        length_ = that.length_;
        idx_ = that.idx_;
    }
    return *this;
}

IntegralLine::~IntegralLine() {}

const std::vector<dvec3> &IntegralLine::getPositions() const { return positions_; }
std::vector<dvec3> &IntegralLine::getPositions() { return positions_; }

const std::vector<dvec3> &IntegralLine::getMetaData(const std::string &name) const {
    auto it = metaData_.find(name);
    if (it == metaData_.end()) {
        throw Exception("No meta data with name: " + name, IvwContext);
    }
    return it->second;
}

std::vector<dvec3>& IntegralLine::getMetaData(const std::string & name) {
    auto it = metaData_.find(name);
    if (it == metaData_.end()) {
        metaData_.emplace(name, std::vector<dvec3>());
        return metaData_.find(name)->second;
    }
    return it->second;
}

std::vector<dvec3> & IntegralLine::createMetaData(const std::string &name)
{
    auto it = metaData_.find(name);
    if (it != metaData_.end()) {
        throw Exception("Meta data already exists: " + name, IvwContext);
    }
    return metaData_[name];
}

bool IntegralLine::hasMetaData(const std::string &name) const
{
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

double IntegralLine::distBetweenPoints(size_t a, size_t b)const {
    if(a==b) return 0;
    if(a>b) return distBetweenPoints(b,a);
    return calcLength( positions_.begin()+a,positions_.begin()+b+1 );
}

dvec3 IntegralLine::getPointAtDistance(double d) const {
    if (d<0 || d > getLength()) {
        return dvec3(0);
    }

    double distPrev = 0,distNext = 0;
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

}  // namespace
