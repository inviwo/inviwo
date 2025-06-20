/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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
#include <inviwo/core/util/document.h>
#include <inviwo/core/util/exception.h>

namespace inviwo {

std::string_view enumToStr(OutportDeterminesSize ods) {
    switch (ods) {
        case OutportDeterminesSize::Yes:
            return "Yes";
        case OutportDeterminesSize::No:
            return "No";
    }
    throw Exception(SourceContext{}, "Found invalid HandleResizeEvents enum value '{}'",
                    static_cast<int>(ods));
}

std::string_view enumToStr(HandleResizeEvents hre) {
    switch (hre) {
        case HandleResizeEvents::Yes:
            return "Yes";
        case HandleResizeEvents::No:
            return "No";
    }
    throw Exception(SourceContext{}, "Found invalid HandleResizeEvents enum value '{}'",
                    static_cast<int>(hre));
}

std::ostream& operator<<(std::ostream& ss, OutportDeterminesSize ods) {
    return ss << enumToStr(ods);
}
std::ostream& operator<<(std::ostream& ss, HandleResizeEvents hre) { return ss << enumToStr(hre); }

ImageOutport::ImageOutport(std::string_view identifier, Document help, const DataFormatBase* format,
                           HandleResizeEvents handleResizeEvents)
    : DataOutport<Image>(identifier, std::move(help))
    , format_(format)
    , handleResizeEvents_(handleResizeEvents) {

    // create a default image
    if (handleResizeEvents == HandleResizeEvents::Yes) {
        setData(std::make_shared<Image>(size2_t{1, 1}, format));
    }
}

ImageOutport::ImageOutport(std::string_view identifier, const DataFormatBase* format,
                           bool handleResizeEvents)
    : ImageOutport(identifier, {}, format,
                   handleResizeEvents ? HandleResizeEvents::Yes : HandleResizeEvents::No) {}

ImageOutport::ImageOutport(std::string_view identifier, bool handleResizeEvents)
    : ImageOutport(identifier, {}, DataVec4UInt8::get(),
                   handleResizeEvents ? HandleResizeEvents::Yes : HandleResizeEvents::No) {}

std::string_view ImageOutport::getClassIdentifier() const {
    return PortTraits<ImageOutport>::classIdentifier();
}

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

std::shared_ptr<const Image> ImageOutport::detachData() {
    image_.reset();
    cache_.setMaster(nullptr);
    return DataOutport<Image>::detachData();
}

void ImageOutport::clear() {
    image_.reset();
    cache_.setMaster(nullptr);
    DataOutport<Image>::clear();
}

bool ImageOutport::hasEditableData() const { return static_cast<bool>(image_); }

size2_t ImageOutport::getLargestReqDim() const {
    if (requestedDimensions_.empty()) return size2_t(1, 1);

    return std::max_element(requestedDimensions_.begin(), requestedDimensions_.end(),
                            [](const auto& a, const auto& b) {
                                return a.second.x * a.second.y < b.second.x * b.second.y;
                            })
        ->second;
}

void ImageOutport::pruneCache() {
    std::vector<size2_t> registeredDimensions;
    std::transform(requestedDimensions_.begin(), requestedDimensions_.end(),
                   std::back_inserter(registeredDimensions),
                   [](const auto& item) { return item.second; });
    cache_.prune(registeredDimensions);
}

void ImageOutport::disconnectFrom(Inport* inport) {
    const auto oldSize = getLargestReqDim();

    requestedDimensions_.erase(inport);
    pruneCache();

    const auto newDimensions = getLargestReqDim();
    if (handleResizeEvents_ == HandleResizeEvents::Yes) {
        setDimensions(newDimensions);  // resize data.

        // Make sure that all ImageOutports in the same group that has the same size.
        for (auto port : getProcessor()->getPortsInSameGroup(this)) {
            if (port != this) {
                if (auto imageOutport = dynamic_cast<ImageOutport*>(port)) {
                    imageOutport->setDimensions(newDimensions);
                }
            }
        }
    }
    ResizeEvent newEvent{newDimensions, oldSize};
    getProcessor()->propagateEvent(&newEvent, this);

    DataOutport<Image>::disconnectFrom(inport);
}

void ImageOutport::connectTo(Inport* inport) { DataOutport<Image>::connectTo(inport); }

void ImageOutport::propagateEvent(Event* event, Inport* source) {
    if (event->hash() != ResizeEvent::chash()) {
        DataOutport<Image>::propagateEvent(event, source);
        return;
    }
    auto resizeEvent = static_cast<ResizeEvent*>(event);
    requestedDimensions_[source] = resizeEvent->size();
    pruneCache();

    const auto newDimensions = getLargestReqDim();

    if (handleResizeEvents_ == HandleResizeEvents::Yes) {
        setDimensions(newDimensions);  // resize data.

        // Make sure that all ImageOutports in the same group that has the same size.
        for (auto port : getProcessor()->getPortsInSameGroup(this)) {
            if (port != this) {
                if (auto imageOutport = dynamic_cast<ImageOutport*>(port)) {
                    imageOutport->setDimensions(newDimensions);
                }
            }
        }

        // Since we have destructively resized the output we need to invalidate.
        getProcessor()->invalidate(InvalidationLevel::InvalidOutput);
    }

    // Propagate the resize event
    ResizeEvent newEvent{*resizeEvent};
    newEvent.setSize(newDimensions);
    getProcessor()->propagateEvent(&newEvent, this);
}

const DataFormatBase* ImageOutport::getDataFormat() const { return format_; }

std::shared_ptr<const Image> ImageOutport::getDataForPort(const Inport* port) const {
    if (const auto it = requestedDimensions_.find(port); it != requestedDimensions_.end()) {
        if (auto img = cache_.getImage(it->second)) {
            img->updateResource(ResourceMeta{.source = getPath()});
            return img;
        }
    }
    return nullptr;
}

void ImageOutport::setDimensions(const size2_t& newDimension) {
    if (image_) {
        if (newDimension != image_->getDimensions()) {
            // Set new dimensions
            image_->setDimensions(newDimension);
            cache_.setInvalid();
        }
    } else {
        setData(std::make_shared<Image>(newDimension, format_));
    }
}

size2_t ImageOutport::getDimensions() const {
    if (image_)
        return image_->getDimensions();
    else if (data_)
        return data_->getDimensions();
    else
        return getLargestReqDim();
}

std::shared_ptr<Image> ImageOutport::getEditableData() const {
    if (image_) {
        return image_;
    } else {
        return nullptr;
    }
}

void ImageOutport::setHandleResizeEvents(bool handleResizeEvents) {
    handleResizeEvents_ = handleResizeEvents ? HandleResizeEvents::Yes : HandleResizeEvents::No;
}

bool ImageOutport::isHandlingResizeEvents() const {
    return (handleResizeEvents_ == HandleResizeEvents::Yes) && image_;
}

Document ImageOutport::getInfo() const {
    using P = Document::PathComponent;
    using H = utildoc::TableBuilder::Header;

    auto doc = DataOutport<Image>::getInfo();
    {
        auto t = doc.get({P{"table"}});
        utildoc::TableBuilder tb(t);
        tb(H("Has Editable Data"), hasEditableData());
        tb(H("Handle Resize Events"), handleResizeEvents_);
    }
    auto p = doc.append("p");
    p.append("b", "Requested sizes", {{"style", "color:white;"}});
    if (requestedDimensions_.empty()) {
        p += "No requested sizes";
    } else {
        const auto master = getLargestReqDim();
        utildoc::TableBuilder tb(p, P::end());
        for (const auto& [port, size] : requestedDimensions_) {
            tb(port->getProcessor()->getIdentifier(), port->getIdentifier(), size,
               size == master ? "Master" : "");
        }
    }
    return doc;
}

template class IVW_CORE_TMPL_INST BaseImageInport<0>;
template class IVW_CORE_TMPL_INST BaseImageInport<1>;

}  // namespace inviwo
