
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

#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/interaction/events/resizeevent.h>
#include <inviwo/core/util/imagecache.h>
#include <inviwo/core/util/fmtutils.h>
#include <inviwo/core/util/staticstring.h>

#include <unordered_map>

namespace inviwo {

class ImageOutport;

class IVW_CORE_API ImagePortBase {
public:
    virtual ~ImagePortBase() = default;
    virtual bool isOutportDeterminingSize() const = 0;
    virtual void setOutportDeterminesSize(bool outportDeterminesSize) = 0;
};

enum class OutportDeterminesSize { Yes, No };
enum class HandleResizeEvents { Yes, No };

IVW_CORE_API std::string_view enumToStr(OutportDeterminesSize ods);
IVW_CORE_API std::string_view enumToStr(HandleResizeEvents hre);
IVW_CORE_API std::ostream& operator<<(std::ostream& ss, OutportDeterminesSize ods);
IVW_CORE_API std::ostream& operator<<(std::ostream& ss, HandleResizeEvents hre);

/**
 * \ingroup ports
 * BaseImageInport extends  DataInport<Image> with extra functionality for handing
 * ResizeEvents. The following table explains the behaviors:
 *
 *
 * \verbatim
 *                                                 ImageOutport
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
 * \endverbatim
 *
 * @see ImageOutport
 * @see ResizeEvent
 *
 */
template <size_t N = 1>
class BaseImageInport : public DataInport<Image, N>, public ImagePortBase {
public:
    BaseImageInport(std::string_view identifier, Document help = {},
                    OutportDeterminesSize value = OutportDeterminesSize::No);
    BaseImageInport(std::string_view identifier, bool outportDeterminesSize);

    virtual ~BaseImageInport();
    virtual std::string_view getClassIdentifier() const override;

    virtual std::shared_ptr<const Image> getData() const override;
    virtual std::vector<std::shared_ptr<const Image>> getVectorData() const override;
    virtual std::vector<std::pair<Outport*, std::shared_ptr<const Image>>> getSourceVectorData()
        const override;

    virtual bool isOutportDeterminingSize() const override;
    virtual void setOutportDeterminesSize(bool outportDeterminesSize) override;
    void setOutportDeterminesSize(OutportDeterminesSize outportDeterminesSize);

    void passOnDataToOutport(ImageOutport* outport) const;

    virtual Document getInfo() const override;

    bool hasData() const override;

private:
    std::shared_ptr<const Image> getImage(ImageOutport* port) const;
    OutportDeterminesSize outportDeterminesSize_;
};

/**
 * \ingroup ports
 */
using ImageInport = BaseImageInport<1>;

/**
 * \ingroup ports
 */
using ImageMultiInport = BaseImageInport<0>;

extern template class IVW_CORE_TMPL_EXP BaseImageInport<0>;
extern template class IVW_CORE_TMPL_EXP BaseImageInport<1>;

template <>
struct PortTraits<BaseImageInport<1>> {
    static std::string_view classIdentifier() {
        constexpr auto name = DataTraits<Image>::classIdentifier();
        static constexpr auto cid = StaticString<name.size()>{name} + ".inport";
        return cid;
    }
};
template <>
struct PortTraits<BaseImageInport<0>> {
    static std::string_view classIdentifier() {
        constexpr auto name = DataTraits<Image>::classIdentifier();
        static const auto cid = StaticString<name.size()>{name} + ".multi.inport";
        return cid;
    }
};

/**
 * \ingroup ports
 * ImageOutport extends DataOutport<Image> with extra functionality for handing
 * ResizeEvents. The following table explains the behaviors:
 *
 * \verbatim
 *                                                 ImageOutport
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
 * \endverbatim
 *
 * The ImageOutport records all the requested sizes from all its connected inports. If the outport
 * 'isHandlingResizeEvents', the port will resize its data to the largest the requested size and
 * propagate that size and a new event upwards in the network. If it does not handle resize events,
 * it will propagate the resize events but not resize its data.
 *
 * When an inport is disconnected from a outport, the port will remove its requested size and
 * propagate a new resize event upwards with the new largest size, given that the port handles
 * resize events.
 *
 * As soon as the network adds or removes a connection, all the image sinks (processors that
 * consume images) are responsible for pushing a new resize event to the network to make sure that
 * all the image ports in the network above have an up-to-date view on which image sizes to use.
 * (@see ImageExport, @see CanvasProcessorWidget)
 *
 * @see BaseImageInport
 * @see ResizeEvent
 *
 */
class IVW_CORE_API ImageOutport : public DataOutport<Image> {
    template <size_t N>
    friend class BaseImageInport;

public:
    ImageOutport(std::string_view identifier, Document help,
                 const DataFormatBase* format = DataVec4UInt8::get(),
                 HandleResizeEvents value = HandleResizeEvents::Yes);
    ImageOutport(std::string_view identifier, const DataFormatBase* format = DataVec4UInt8::get(),
                 bool handleResizeEvents = true);
    ImageOutport(std::string_view identifier, bool handleResizeEvents);

    virtual ~ImageOutport() = default;
    virtual std::string_view getClassIdentifier() const override;

    virtual void setData(std::shared_ptr<const Image>) override;
    virtual void setData(const Image* data) override;  // will assume ownership of data.
    void setData(std::shared_ptr<Image>);
    void setData(Image* data);  // will assume ownership of data.

    /**
     * \copydoc inviwo::DataOutport<Image>::detachData
     */
    virtual std::shared_ptr<const Image> detachData() override;

    /**
     * \copydoc inviwo::DataOutport<Image>::clear
     */
    virtual void clear() override;

    bool hasEditableData() const;
    std::shared_ptr<Image> getEditableData() const;

    std::shared_ptr<const Image> getDataForPort(const Inport* port) const;

    /**
     * Handle resize event
     */
    void propagateEvent(Event* event, Inport* source) override;
    const DataFormatBase* getDataFormat() const;
    size2_t getDimensions() const;
    /**
     * Set the dimensions of this port without propagating the size
     * through the network. Will call setDimensions on the image contained within the port.
     * This is a destructive operation. The image port has to own it's data for this to work.
     */
    void setDimensions(const size2_t& newDimension);

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
    virtual void connectTo(Inport* port) override;

    virtual Document getInfo() const override;

private:
    size2_t getLargestReqDim() const;
    void pruneCache();

    std::unordered_map<const Inport*, size2_t> requestedDimensions_;
    std::shared_ptr<Image> image_;

    const DataFormatBase* format_;
    HandleResizeEvents handleResizeEvents_;  // Yes if data should be resized during a resize
                                             // propagation, otherwise No
    ImageCache cache_;
};

template <>
struct PortTraits<ImageOutport> {
    static std::string_view classIdentifier() {
        constexpr auto name = DataTraits<Image>::classIdentifier();
        static const auto cid = StaticString<name.size()>{name} + ".outport";
        return cid;
    }
};

// Image Inport
template <size_t N>
BaseImageInport<N>::BaseImageInport(std::string_view identifier, Document help,
                                    OutportDeterminesSize value)
    : DataInport<Image, N>(identifier, help), outportDeterminesSize_(value) {}

template <size_t N>
BaseImageInport<N>::BaseImageInport(std::string_view identifier, bool outportDeterminesSize)
    : BaseImageInport<N>{
          identifier,
          {},
          outportDeterminesSize ? OutportDeterminesSize::Yes : OutportDeterminesSize::No} {}

template <size_t N>
BaseImageInport<N>::~BaseImageInport() = default;

template <size_t N>
std::string_view BaseImageInport<N>::getClassIdentifier() const {
    return PortTraits<BaseImageInport<N>>::classIdentifier();
}

template <size_t N>
std::shared_ptr<const Image> BaseImageInport<N>::getData() const {
    if (this->isConnected()) {
        auto imgport = static_cast<ImageOutport*>(this->getConnectedOutport());
        return getImage(imgport);
    } else {
        return nullptr;
    }
}

template <size_t N>
std::vector<std::shared_ptr<const Image>> BaseImageInport<N>::getVectorData() const {
    std::vector<std::shared_ptr<const Image>> res;

    for (auto outport : this->connectedOutports_) {
        auto imgport = static_cast<ImageOutport*>(outport);
        if (auto img = getImage(imgport)) res.emplace_back(img);
    }

    return res;
}

template <size_t N>
std::vector<std::pair<Outport*, std::shared_ptr<const Image>>>
BaseImageInport<N>::getSourceVectorData() const {
    std::vector<std::pair<Outport*, std::shared_ptr<const Image>>> res;

    for (auto outport : this->connectedOutports_) {
        auto imgport = static_cast<ImageOutport*>(outport);
        if (auto img = getImage(imgport)) res.emplace_back(imgport, img);
    }

    return res;
}

template <size_t N>
std::shared_ptr<const Image> BaseImageInport<N>::getImage(ImageOutport* port) const {
    if (isOutportDeterminingSize()) {
        return port->getData();
    } else {
        return port->getDataForPort(this);
    }
}

template <size_t N>
bool BaseImageInport<N>::isOutportDeterminingSize() const {
    return outportDeterminesSize_ == OutportDeterminesSize::Yes;
}

template <size_t N>
void BaseImageInport<N>::setOutportDeterminesSize(OutportDeterminesSize outportDeterminesSize) {
    outportDeterminesSize_ = outportDeterminesSize;
}

template <size_t N>
void BaseImageInport<N>::setOutportDeterminesSize(bool outportDeterminesSize) {
    outportDeterminesSize_ =
        (outportDeterminesSize ? OutportDeterminesSize::Yes : OutportDeterminesSize::No);
}

template <size_t N>
void BaseImageInport<N>::passOnDataToOutport(ImageOutport* outport) const {
    if (this->hasData()) {
        std::shared_ptr<const Image> img = getData();
        std::shared_ptr<Image> out = outport->getEditableData();
        if (out) img->copyRepresentationsTo(out.get());
    }
}

template <size_t N>
Document BaseImageInport<N>::getInfo() const {
    using P = Document::PathComponent;
    using H = utildoc::TableBuilder::Header;

    auto doc = DataInport<Image, N>::getInfo();

    auto t = doc.get({P{"table"}});

    utildoc::TableBuilder tb(t);
    tb(H("Outport Determining Size"), outportDeterminesSize_);

    return doc;
}

template <size_t N>
bool BaseImageInport<N>::hasData() const {
    if constexpr (N == 0) {
        return this->isConnected() && util::any_of(this->connectedOutports_, [this](Outport* p) {
                   return getImage(static_cast<ImageOutport*>(p)) != nullptr;
               });
    } else {
        // Note: Cannot use ImageOutport::hasData() as getData()
        // depends on the ImageInport
        return this->isConnected() && util::all_of(this->connectedOutports_, [this](Outport* p) {
                   return getImage(static_cast<ImageOutport*>(p)) != nullptr;
               });
    }
}

}  // namespace inviwo

#ifndef DOXYGEN_SHOULD_SKIP_THIS
template <>
struct fmt::formatter<inviwo::OutportDeterminesSize>
    : inviwo::FlagFormatter<inviwo::OutportDeterminesSize> {};
template <>
struct fmt::formatter<inviwo::HandleResizeEvents>
    : inviwo::FlagFormatter<inviwo::HandleResizeEvents> {};
#endif
