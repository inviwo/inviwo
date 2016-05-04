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

#ifndef IVW_IMAGEPORT_H
#define IVW_IMAGEPORT_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/interaction/events/eventhandler.h>
#include <inviwo/core/interaction/events/resizeevent.h>
#include <inviwo/core/util/imagecache.h>

#include <unordered_map>

/*                                                 ImageOutport                              
 *                                           isHandlingResizeEvents()                        
 *                                                                                           
 *                               True (default)                          False               
 *                    ┌──────────────────────────────────┬──────────────────────────────────┐
 *                    │ Outport::Size = max(Inports      │ Outport::Size = Outport::size    │
 *                    │ requested sizes)                 │ (no resize of data)              │
 *                    │ (resize the data in the outport  │                                  │
 *             False  │ if needed)                       │                                  │
 *           (default)│                                  │                                  │
 *                    │ Inport::Size = Inport requested  │ Inport::Size = Inport requested  │
 *                    │ size                             │ size                             │
 *  ImageInport       │ (return a resized copy if        │ (return a resized copy if        │
 *                    │ needed)                          │ needed)                          │
 *   isOutport-       │                                  │                                  │
 *  Determining-      ├──────────────────────────────────┼──────────────────────────────────┤
 *     Size()         │ Outport::Size = max(all inports  │ Outport::Size = Outport::Size    │
 *                    │ requested sizes)                 │ (no resize of data)              │
 *                    │ (resize the data in the outport  │                                  │
 *              True  │ if needed)                       │                                  │
 *                    │                                  │                                  │
 *                    │ Inport::Size = Outport::size     │ Inport::Size = Outport::size     │
 *                    │ (no copy)                        │ (no copy)                        │
 *                    │                                  │                                  │
 *                    │                                  │                                  │
 *                    └──────────────────────────────────┴──────────────────────────────────┘
 */                                                                                                                                                                                                                                                                   


namespace inviwo {

class ImageOutport;

class IVW_CORE_API ImagePortBase {
public:
    virtual ~ImagePortBase() = default;
    virtual size2_t getRequestedDimensions(ImageOutport* outport) const = 0;
    virtual void propagateResizeEvent(ResizeEvent* resizeEvent,
                                      ImageOutport* target = nullptr) = 0;
    virtual bool isOutportDeterminingSize() const = 0;
    virtual void setOutportDeterminesSize(bool outportDeterminesSize) = 0;
};

template <size_t N = 1>
class BaseImageInport : public DataInport<Image, N>, public ImagePortBase {
public:
    BaseImageInport(std::string identifier, bool outportDeterminesSize = false);
    virtual ~BaseImageInport();

    /**
     * Connects this inport to the outport. Propagates the inport size to the outport if the
     * processor is an end processor (Canvas) or any of the dependent outports of this inport are
     * connected.
     *
     * @note Does not check if the outport is an ImageOutport
     * @param Outport * outport ImageOutport to connect
     */
    virtual void connectTo(Outport* outport) override;
    virtual void disconnectFrom(Outport* outport) override;
    virtual std::shared_ptr<const Image> getData() const override;
    virtual std::vector<std::shared_ptr<const Image>> getVectorData() const override;
    virtual std::vector<std::pair<Outport*, std::shared_ptr<const Image>>> getSourceVectorData()
        const override;
    virtual std::string getContentInfo() const override;

    virtual size2_t getRequestedDimensions(ImageOutport* outport) const override;

    virtual void propagateResizeEvent(ResizeEvent* resizeEvent,
                                      ImageOutport* target = nullptr) override;

    virtual bool isOutportDeterminingSize() const override;
    virtual void setOutportDeterminesSize(bool outportDeterminesSize) override;

    void passOnDataToOutport(ImageOutport* outport) const;

private:
    std::shared_ptr<const Image> getImage(ImageOutport* port) const;

    std::unordered_map<ImageOutport*, size2_t> requestedDimensionsMap_;
    bool outportDeterminesSize_;
};

using ImageInport = BaseImageInport<1>;
using ImageMultiInport = BaseImageInport<0>;

class IVW_CORE_API ImageOutport : public DataOutport<Image>, public EventHandler {
    template <size_t N> friend class BaseImageInport;

public:
    ImageOutport(std::string identifier, const DataFormatBase* format = DataVec4UInt8::get(),
        bool handleResizeEvents = true);
    ImageOutport(std::string identifier, bool handleResizeEvents);

    virtual ~ImageOutport() = default;

    virtual void setData(std::shared_ptr<const Image>) override;
    virtual void setData(const Image* data) override; // will assume ownership of data.
    void setData(std::shared_ptr<Image>);
    void setData(Image* data); // will assume ownership of data.

    bool hasEditableData() const;
    std::shared_ptr<Image> getEditableData() const;
    std::shared_ptr<const Image> getResizedImageData(size2_t dimensions) const;

    /**
     * Handle resize event
     */
    void propagateResizeEvent(ResizeEvent* resizeEvent);
    const DataFormatBase* getDataFormat() const;
    size2_t getDimensions() const;   
    /**
     * Set the dimensions of this port without propagating the size
     * through the network. Will call setDimensions on the image contained within the port.
     * This is a destructive operation. The image port has to own it's data for this to work.
     */
    void setDimensions(const size2_t& newDimension);

    bool addResizeEventListener(EventListener*);
    bool removeResizeEventListener(EventListener*);

    /**
     * Determine if the image data should be resized during a resize event.
     * We will only resize if we own the data in the port.
     * @param handleResizeEvents True if data should be resized during a resize propagation,
     * otherwise false
     */
    void setHandleResizeEvents(bool handleResizeEvents);
    bool isHandlingResizeEvents() const;

    virtual void invalidate(InvalidationLevel invalidationLevel) override;

    virtual void disconnectFrom(Inport* port) override;

private:
    std::shared_ptr<Image> image_;
    size2_t defaultDimensions_;
    const DataFormatBase* format_;
    bool handleResizeEvents_;  // True if data should be resized during a resize propagation,
                               // otherwise false
    ImageCache cache_;
};

// Image Inport
template <size_t N>
BaseImageInport<N>::BaseImageInport(std::string identifier, bool outportDeterminesSize)
    : DataInport<Image, N>(identifier)
    , outportDeterminesSize_(outportDeterminesSize) {
        // A default size
        requestedDimensionsMap_[nullptr] = size2_t(0); 
    }

template <size_t N>
BaseImageInport<N>::~BaseImageInport() {}

template <size_t N>
void BaseImageInport<N>::connectTo(Outport* outport) {
    if (this->getNumberOfConnections() + 1 > this->getMaxNumberOfConnections())
        throw Exception("Trying to connect to a full port.", IvwContext);

    ImageOutport* imageOutport = dynamic_cast<ImageOutport*>(outport);
    if (requestedDimensionsMap_.find(imageOutport) != requestedDimensionsMap_.end()) {
        ResizeEvent resizeEvent(requestedDimensionsMap_[imageOutport]);
        imageOutport->propagateResizeEvent(&resizeEvent);
    } else {
        ResizeEvent resizeEvent(requestedDimensionsMap_[nullptr]);
        imageOutport->propagateResizeEvent(&resizeEvent);
    }

    DataInport<Image, N>::connectTo(outport);
}

template <size_t N>
void BaseImageInport<N>::disconnectFrom(Outport* outport) {
    ImageOutport* imageOutport = dynamic_cast<ImageOutport*>(outport);
    requestedDimensionsMap_.erase(imageOutport);
    DataInport<Image, N>::disconnectFrom(outport);
}

// set dimensions based on port groups
template <size_t N>
void BaseImageInport<N>::propagateResizeEvent(ResizeEvent* resizeEvent, ImageOutport* target) {
    if (target) {
        requestedDimensionsMap_[target] = resizeEvent->size();
        target->propagateResizeEvent(resizeEvent);
    } else {
        for (auto outport : this->connectedOutports_) {
            if (auto imageOutport = static_cast<ImageOutport*>(outport)) {
                requestedDimensionsMap_[imageOutport] = resizeEvent->size();
                imageOutport->propagateResizeEvent(resizeEvent);
            }
        }
        // Save a default size.
        requestedDimensionsMap_[nullptr] = resizeEvent->size();
    }
}

template <size_t N>
size2_t BaseImageInport<N>::getRequestedDimensions(ImageOutport* outport) const {
    auto it = requestedDimensionsMap_.find(outport);
    if (it != requestedDimensionsMap_.end()) {
        return it->second;
    } else {
        it = requestedDimensionsMap_.find(nullptr);
        if (it != requestedDimensionsMap_.end())
            return it->second;
        else
            return size2_t(0);
    }
}

template <size_t N>
std::shared_ptr<const Image> BaseImageInport<N>::getData() const {
    if (this->hasData()) {
        auto imgport = static_cast<ImageOutport*>(this->getConnectedOutport());
        return getImage(imgport);
    } else {
        return std::shared_ptr<const Image>();
    }
}

template <size_t N /*= 1*/>
std::vector<std::shared_ptr<const Image>> BaseImageInport<N>::getVectorData() const {
    std::vector<std::shared_ptr<const Image>> res(N);

    for (auto outport : this->connectedOutports_) {
        auto imgport = static_cast<ImageOutport*>(outport);
        if (imgport->hasData()) res.push_back(getImage(imgport));
    }

    return res;
}

template <size_t N>
std::vector<std::pair<Outport*, std::shared_ptr<const Image>>> BaseImageInport<N>::getSourceVectorData() const {
    std::vector<std::pair<Outport*, std::shared_ptr<const Image>>> res(N);

    for (auto outport : this->connectedOutports_) {
        auto imgport = static_cast<ImageOutport*>(outport);
        if (imgport->hasData()) res.emplace_back(imgport, getImage(imgport));
    }

    return res;
}

template <size_t N>
std::shared_ptr<const Image> BaseImageInport<N>::getImage(ImageOutport* port) const {
    if (isOutportDeterminingSize()) {
        return port->getData();
    } else {
        auto it = requestedDimensionsMap_.find(port);
        if (it != requestedDimensionsMap_.end()) {
            return port->getResizedImageData(it->second);
        } else {
            return port->getData();
        }
    }
}

template <size_t N>
bool BaseImageInport<N>::isOutportDeterminingSize() const {
    return outportDeterminesSize_;
}

template <size_t N>
void BaseImageInport<N>::setOutportDeterminesSize(bool outportDeterminesSize) {
    outportDeterminesSize_ = outportDeterminesSize;
}

template <size_t N>
std::string BaseImageInport<N>::getContentInfo() const {
    if (this->hasData())
        return getData()->getDataInfo();
    else
        return this->getClassIdentifier() + " has no data.";
}

template <size_t N>
void BaseImageInport<N>::passOnDataToOutport(ImageOutport* outport) const {
    if (this->hasData()) {
        std::shared_ptr<const Image> img = getData();
        std::shared_ptr<Image> out = outport->getEditableData();
        if (out) img->copyRepresentationsTo(out.get());
    }
}

}  // namespace

#endif  // IVW_IMAGEPORT_H
