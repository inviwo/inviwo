/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#ifndef IVW_DATASEQUENCE_H
#define IVW_DATASEQUENCE_H

#include <inviwo/core/common/inviwocoredefine.h>

namespace inviwo {

template <typename T>
class DataSequence : public T {

public:
    DataSequence();
    DataSequence(const T&);
    DataSequence(const DataSequence&);
    DataSequence<T>& operator=(const DataSequence<T>&);

    virtual ~DataSequence();

    void add(T*);

    void setCurrentIndex(int);
    void setNextAsCurrent();
    
    T* getCurrent();

    virtual std::string getDataInfo() const;

    size_t getNumSequences() const;

protected:
    std::vector<T*> sequenceContainer_;
    int currentIdx_;

};

template <typename T>
std::string inviwo::DataSequence<T>::getDataInfo() const {

    std::stringstream ss;
    ss << "<table border='0' cellspacing='0' cellpadding='0' style='border-color:white;white-space:pre;'>\n"
        << "<tr><td style='color:#bbb;padding-right:8px;'>Type</td><td><nobr>Data Sequence</nobr></td></tr>\n"
        << "<tr><td style='color:#bbb;padding-right:8px;'>Size</td><td><nobr>" << sequenceContainer_.size() << "</nobr></td></tr>\n"
        << "<tr><td style='color:#bbb;padding-right:8px;'>Current</td><td><nobr>" << currentIdx_ << "</nobr></td></tr>\n"
        << "</table>\n"
        << "<br/>\n"
        << "<b>Element</b>\n";

    const BaseData* data = dynamic_cast<const BaseData*>(sequenceContainer_[currentIdx_]);
    if (data) {
        ss << data->getDataInfo();
    } else {
        ss << "Not a BaseData Object";
    }
    return ss.str();
}

template <typename T>
DataSequence<T>::DataSequence() : T(), currentIdx_(0) {}

template <typename T>
DataSequence<T>::DataSequence(const T& that) : T(that), currentIdx_(0) {}

template <typename T>
DataSequence<T>::DataSequence(const DataSequence<T>& rhs) : T(rhs), currentIdx_(rhs.currentIdx_) {}

template <typename T>
DataSequence<T>& DataSequence<T>::operator=(const DataSequence<T>& rhs){
    if (this != &rhs) {
        T::operator=(rhs);
        currentIdx_(rhs.currentIndex_);
    }
    return *this;
}

template <typename T>
DataSequence<T>::~DataSequence() {
    while (!sequenceContainer_.empty()){
        delete sequenceContainer_.back();
        sequenceContainer_.pop_back();
    }
}

template <typename T>
void DataSequence<T>::add(T* d) {
    sequenceContainer_.push_back(d);
}

template <typename T>
void DataSequence<T>::setCurrentIndex(int idx) {
    currentIdx_ = glm::clamp(idx, 0, static_cast<int>(sequenceContainer_.size()) - 1);
}

template <typename T>
void DataSequence<T>::setNextAsCurrent() {
    currentIdx_ = (currentIdx_ < static_cast<int>(sequenceContainer_.size())-1 ? currentIdx_+1 : 0);
}

template <typename T>
T* DataSequence<T>::getCurrent() {
    if(currentIdx_ < static_cast<int>(sequenceContainer_.size()))
        return sequenceContainer_[currentIdx_];

    if(!sequenceContainer_.empty())
        sequenceContainer_[0];

    return nullptr;
}

template <typename T>
size_t DataSequence<T>::getNumSequences() const {
    return sequenceContainer_.size();
}

} // namespace

#endif // IVW_DATASEQUENCE_H
