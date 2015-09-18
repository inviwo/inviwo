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

#include <inviwo/core/datastructures/data.h>

namespace inviwo {

std::string BaseData::getDataInfo() const { return ""; }

Data::Data() : BaseData(), lastValidRepresentation_(), dataFormatBase_(DataFormatBase::get()) {}

Data::Data(const DataFormatBase* format)
    : BaseData(), lastValidRepresentation_(), dataFormatBase_(format) {}

Data::Data(const Data& rhs)
    : BaseData(rhs), lastValidRepresentation_(), dataFormatBase_(rhs.dataFormatBase_) {
    rhs.copyRepresentationsTo(this);
}

Data& Data::operator=(const Data& that) {
    if (this != &that) {
        BaseData::operator=(that);
        that.copyRepresentationsTo(this);
        dataFormatBase_ = that.dataFormatBase_;
    }

    return *this;
}

Data::~Data() {}

void Data::invalidateAllOther(DataRepresentation* repr) {
    for (auto& elem : representations_) {
        if (elem.second.get() != repr) elem.second->setValid(false);
    }
}

void Data::clearRepresentations() {
    representations_.clear();
}

void Data::copyRepresentationsTo(Data* targetData) const {
    targetData->clearRepresentations();

    if (lastValidRepresentation_) {
        auto rep = std::shared_ptr<DataRepresentation>(lastValidRepresentation_->clone());
        targetData->addRepresentation(rep);
    }
}

void Data::addRepresentation(std::shared_ptr<DataRepresentation> repr) {
    repr->setOwner(this);
    representations_[repr->getTypeIndex()] = repr;
    lastValidRepresentation_ = repr;
    repr->setValid(true);
}

void Data::removeRepresentation(std::shared_ptr<DataRepresentation> representation) {
    for (auto& elem : representations_) {
        if(elem.second == representation) {
            representations_.erase(elem.first);
            break;
        }
    }

    if (lastValidRepresentation_ == representation) {
        lastValidRepresentation_.reset();

        for (auto& elem : representations_) {
            if(elem.second->isValid()) {
                lastValidRepresentation_ = elem.second;
            }
        }
    }
}

void Data::removeOtherRepresentations(std::shared_ptr<DataRepresentation> representation) {
    representations_.clear();
    representations_[representation->getTypeIndex()] = representation;

    if(lastValidRepresentation_ != representation) lastValidRepresentation_.reset();
}

bool Data::hasRepresentations() const {
    return !representations_.empty();
}

void Data::setDataFormat(const DataFormatBase* format) {
    dataFormatBase_ = format;
}

const DataFormatBase* Data::getDataFormat() const {
    return dataFormatBase_;
}


} // namespace
