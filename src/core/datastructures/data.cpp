/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

BaseData::BaseData() : MetaDataOwner() {
}

BaseData::BaseData(const BaseData& rhs) : MetaDataOwner(rhs)  {
}

BaseData& BaseData::operator=(const BaseData& that) {
    if (this != &that) {
        MetaDataOwner::operator=(that);
    }

    return *this;
}
BaseData::~BaseData() {}

std::string BaseData::getDataInfo() const{
    return "";
}

Data::Data()
    : BaseData()
    , validRepresentations_(0)
    , lastValidRepresentation_(NULL)
    , dataFormatBase_(DataFormatBase::get()) {
}

Data::Data(const DataFormatBase* format)
    : BaseData()
    , validRepresentations_(0)
    , lastValidRepresentation_(NULL)
    , dataFormatBase_(format) {
}

Data::Data(const Data& rhs)
    : BaseData(rhs)
    , validRepresentations_(0)
    , lastValidRepresentation_(NULL)
    , dataFormatBase_(rhs.dataFormatBase_) {
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


Data::~Data() {
    clearRepresentations();
}

void Data::clearRepresentations() {
    setAllRepresentationsAsInvalid();

    while (hasRepresentations()) {
        delete representations_.back();
        representations_.pop_back();
    }
}

void Data::copyRepresentationsTo(Data* targetData) const {
    targetData->clearRepresentations();

    if (lastValidRepresentation_) {
        DataRepresentation* rep = lastValidRepresentation_->clone();
        targetData->addRepresentation(rep);
    }
}

void Data::addRepresentation(DataRepresentation* representation) {
    representation->setOwner(this);
    representations_.push_back(representation);
    lastValidRepresentation_ = representation;
    setRepresentationAsValid(static_cast<int>(representations_.size())-1);
    newRepresentationCreated();
}

void Data::removeRepresentation(DataRepresentation* representation)
{
    std::vector<DataRepresentation*>::iterator it = std::find(representations_.begin(), representations_.end(), representation);

    if (it != representations_.end()) {
        // Update last valid representation
        if (lastValidRepresentation_ == *it) {
            lastValidRepresentation_ = NULL;

            for (int i = static_cast<int>(representations_.size())-1; i >= 0; --i) {
                // Check if this representation is valid
                // and make sure that it is not the one removed
                if (isRepresentationValid(i) && representations_[i] != representation) {
                    lastValidRepresentation_ = representations_[i];
                    break;
                }
            }
        }

        // Update valid representation bit mask
        size_t element = static_cast<size_t>(std::distance(representations_.begin(), it));

        // Start after the element that is going to be removed and update the mask.
        for (int i = static_cast<int>(element+1); i < static_cast<int>(representations_.size()); ++i) {
            if (isRepresentationValid(i))
                setRepresentationAsValid(i-1);
            else
                setRepresentationAsInvalid(i-1);
        }

        delete(*it);
        representations_.erase(it);
    }
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
