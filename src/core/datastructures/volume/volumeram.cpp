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

#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>

namespace inviwo {

VolumeRAM::VolumeRAM(uvec3 dimensions, const DataFormatBase* format)
    : VolumeRepresentation(dimensions, format)
    , data_(NULL)
    , ownsDataPtr_(true)
    , histograms_(NULL)
    , calculatingHistogram_(false)
    , stopHistogramCalculation_(false) {}

VolumeRAM::VolumeRAM(const VolumeRAM& rhs)
    : VolumeRepresentation(rhs)
    , data_(NULL)
    , ownsDataPtr_(rhs.ownsDataPtr_)
    , histograms_(NULL)
    , calculatingHistogram_(false)
    , stopHistogramCalculation_(false) {
    if (rhs.histograms_) {
        histograms_ = new std::vector<NormalizedHistogram*>();
        for (std::vector<NormalizedHistogram*>::iterator it = rhs.histograms_->begin();
             it != rhs.histograms_->end(); ++it) {
            histograms_->push_back(new NormalizedHistogram(*it));
        }
    }
}
VolumeRAM& VolumeRAM::operator=(const VolumeRAM& that) {
    if (this != &that) {
        VolumeRepresentation::operator=(that);
        data_ = NULL;
        ownsDataPtr_ = that.ownsDataPtr_;
        calculatingHistogram_ = false;
        stopHistogramCalculation_ = false;

        if (histograms_) {
            for (std::vector<NormalizedHistogram*>::iterator it = that.histograms_->begin();
                 it != that.histograms_->end(); ++it) {
                delete *it;
            }
            histograms_->clear();
        }

        if (that.histograms_) {
            if (!histograms_) {
                histograms_ = new std::vector<NormalizedHistogram*>();
            }
            for (std::vector<NormalizedHistogram*>::iterator it = that.histograms_->begin();
                 it != that.histograms_->end(); ++it) {
                histograms_->push_back(new NormalizedHistogram(*it));
            }
        } else {
            histograms_ = NULL;
        }
    }

    return *this;
}
VolumeRAM::~VolumeRAM() { deinitialize(); }

void VolumeRAM::initialize() {}

void VolumeRAM::deinitialize() {
    // Make sure that data is deinitialized in
    // child class (should not delete void pointer
    // since destructor will not be called for object.
    if (histograms_) {
        for (std::vector<NormalizedHistogram*>::iterator it = histograms_->begin();
             it != histograms_->end(); ++it) {
            delete *it;
        }
        histograms_->clear();
        delete histograms_;
        histograms_ = NULL;
    }
}

void* VolumeRAM::getData() { return data_; }

const void* VolumeRAM::getData() const { return const_cast<void*>(data_); }

bool VolumeRAM::hasNormalizedHistogram() const {
    return (histograms_ != NULL && !histograms_->empty() && histograms_->at(0)->isValid());
}

NormalizedHistogram* VolumeRAM::getNormalizedHistogram(int sampleRate,
                                                       std::size_t maxNumberOfBins,
                                                       int component) {
    if (!calculatingHistogram_ && data_ && (!histograms_ || !histograms_->at(component)->isValid()))
        calculateHistogram(sampleRate, maxNumberOfBins);

    return histograms_->at(component);
}

const NormalizedHistogram* VolumeRAM::getNormalizedHistogram(int sampleRate,
                                                             std::size_t maxNumberOfBins,
                                                             int component) const {
    if (data_ && !calculatingHistogram_ && (!histograms_ || histograms_->empty() ||
                                            (static_cast<int>(histograms_->size()) > component &&
                                             !histograms_->at(component)->isValid())))
        calculateHistogram(sampleRate, maxNumberOfBins);

    if (histograms_ && static_cast<int>(histograms_->size()) > component) {
        return histograms_->at(component);
    } else {
        return NULL;
    }
}

void VolumeRAM::calculateHistogram(int sampleRate, std::size_t maxNumberOfBins) const {
    calculatingHistogram_ = true;
    stopHistogramCalculation_ = false;

    if (sampleRate < 0) {
        int maxDim = std::max(dimensions_.x, std::max(dimensions_.y, dimensions_.z));
        sampleRate = std::max(1, int(float(maxDim) / 64.0f));
    }

    histograms_ = VolumeRAMNormalizedHistogram::apply(this, histograms_, sampleRate, maxNumberOfBins);
    calculatingHistogram_ = false;
}

void VolumeRAM::setValuesFromVolume(const VolumeRAM* src, const uvec3& dstOffset){
    setValuesFromVolume(src, dstOffset, src->getDimensions(), uvec3(0));
}

size_t VolumeRAM::getNumberOfBytes() const{
    return getDataFormat()->getBytesAllocated()*dimensions_.x*dimensions_.y*dimensions_.z;
}

VolumeRAM* createVolumeRAM(const uvec3& dimensions, const DataFormatBase* format, void* dataPtr) {
    // TODO: Add more formats
    VolumeRAM* result = 0;

    switch (format->getId()) {
        case DataFormatEnums::NOT_SPECIALIZED:
            LogErrorCustom("createVolumeRAM", "Invalid format");
            break;
#define DataFormatIdMacro(i)                                                          \
    case DataFormatEnums::i:                                                          \
        return new VolumeRAMCustomPrecision<Data##i::type, Data##i::bits>(static_cast<Data##i::type*>(dataPtr), dimensions); \
        break;
#include <inviwo/core/util/formatsdefinefunc.h>

        default:
            LogErrorCustom("createVolumeRAM", "Invalid format or not implemented");
            break;
    }

    return result;
}

}  // namespace
