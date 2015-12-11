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

#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/datastructures/image/imageram.h>

namespace inviwo {

ImageOutport::ImageOutport(std::string identifier, const DataFormatBase* format,
                           bool handleResizeEvents)
    : DataOutport<Image>(identifier)
    , defaultDimensions_(8, 8)
    , handleResizeEvents_(handleResizeEvents) {
 
    // create a default image
    setData(std::make_shared<Image>(defaultDimensions_, format));
}

ImageOutport::~ImageOutport() {}

void ImageOutport::invalidate(InvalidationLevel invalidationLevel) {
    if (invalidationLevel > InvalidationLevel::Valid) {
        cache_.setInvalid();
    }
    Outport::invalidate(invalidationLevel);
}

void ImageOutport::setData(std::shared_ptr<const Image> data) {
    DataOutport<Image>::setData(data);
    image_.reset();
    cache_.setMaster(data);
}

void ImageOutport::setData(const Image* data) {
    DataOutport<Image>::setData(data);
    image_.reset();
    cache_.setMaster(data_);
}

void ImageOutport::setData(std::shared_ptr<Image> data) {
    DataOutport<Image>::setData(data);
    image_ = data;
    cache_.setMaster(data);
}

void ImageOutport::setData(Image* data) {
    image_.reset(data);
    DataOutport<Image>::setData(image_); 
    cache_.setMaster(data_);
}

bool ImageOutport::hasEditableData() const {
    return static_cast<bool>(image_);
}


void ImageOutport::disconnectFrom(Inport* port) {
    DataOutport<Image>::disconnectFrom(port);

    // update image size
    ResizeEvent event(uvec2(0,0));
    propagateResizeEvent(&event);
}


void ImageOutport::propagateResizeEvent(ResizeEvent* resizeEvent) {
    // This function should check which dimensions request exists, by going through the successors
    // and checking registeredDimensions.
    // Allocates space holder, sets largest data, cleans up unused data

    std::vector<size2_t> registeredDimensions{resizeEvent->size()};
    for (auto inport : connectedInports_) {   
        if (auto imageInport = dynamic_cast<ImagePortBase*>(inport)) {
            util::push_back_unique(registeredDimensions, imageInport->getRequestedDimensions(this));
        }
    }

    // find the largest dimension.
    size2_t newDimensions =
        *std::max_element(registeredDimensions.begin(), registeredDimensions.end(),
                          [](const size2_t& a, const size2_t& b) { return a.x * a.y < b.x * b.y; });

    // fallback to default if newDim == 0
    if (newDimensions == size2_t(0,0)) newDimensions = defaultDimensions_;

    std::unique_ptr<ResizeEvent> newEvent {resizeEvent->clone()};
    newEvent->setSize(newDimensions);

    if (image_ && handleResizeEvents_ && newDimensions != image_->getDimensions()) { 
        // resize data.
        image_->setDimensions(newDimensions);
        //defaultDimensions_ = image_->getDimensions();
        cache_.setInvalid();

        broadcast(newEvent.get());
    }

    // remove unused image from cache
    cache_.prune(registeredDimensions);

    // Make sure that all ImageOutports in the same group (dependency set) that has the same size.
    // This functionality needs testing.
    for (auto port : getProcessor()->getPortsInSameSet(this)) {
        auto imageOutport = dynamic_cast<ImageOutport*>(port);
        if (imageOutport && imageOutport != this) {
            imageOutport->setDimensions(newDimensions);
        }
    }

    // Propagate the resize event
    getProcessor()->propagateResizeEvent(newEvent.get(), this);
    
    if (handleResizeEvents_) getProcessor()->invalidate(InvalidationLevel::InvalidOutput);
}

std::shared_ptr<const Image> ImageOutport::getResizedImageData(size2_t requiredDimensions) const {
    return cache_.getImage(requiredDimensions);
}

bool ImageOutport::addResizeEventListener(EventListener* el) { return addEventListener(el); }

bool ImageOutport::removeResizeEventListener(EventListener* el) { return removeEventListener(el); }

void ImageOutport::setDimensions(const size2_t& newDimension) {
    defaultDimensions_ = newDimension; // update default
    if (image_) {
        // Set new dimensions
        image_->setDimensions(newDimension);
        cache_.setInvalid();
    }
}

size2_t ImageOutport::getDimensions() const { 
    if (image_) return image_->getDimensions();
    else if(data_) return data_->getDimensions();
    else return defaultDimensions_; 
}


std::shared_ptr<Image> ImageOutport::getEditableData() const {
    if (image_) {
        return image_;
    } else {
        return std::shared_ptr<Image>();
    }
}

void ImageOutport::setHandleResizeEvents(bool handleResizeEvents) {
    handleResizeEvents_ = handleResizeEvents;
}

bool ImageOutport::isHandlingResizeEvents() const { return handleResizeEvents_ && image_; }

}  // namespace
