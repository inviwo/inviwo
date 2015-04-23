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
#include <inviwo/core/ports/multidatainport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/datastructures/image/imageram.h>

namespace inviwo {

// Image Inport
ImageInport::ImageInport(std::string identifier, bool outportDeterminesSize)
    : DataInport<Image>(identifier)
    , dimensions_(8, 8)
    , resizeScale_(1.f, 1.f)
    , outportDeterminesSize_(outportDeterminesSize) {}

ImageInport::~ImageInport() {}

void ImageInport::connectTo(Outport* outport) {
    ImageOutport* imageOutport = dynamic_cast<ImageOutport*>(outport);

    const uvec2 dim =
        isOutportDeterminingSize() && isConnected() ? imageOutport->getDimensions() : dimensions_;

    ResizeEvent resizeEvent(dim);
    if (getProcessor()->isEndProcessor() || isOutportDeterminingSize()) {
        if (imageOutport->isHandlingResizeEvents())
            resizeEvent.setSize(dimensions_);
        else
            dimensions_ = dim;
        imageOutport->changeDataDimensions(&resizeEvent);
    } else {
        // Resize outport if any outport within the same port dependency set is connected
        for (auto port : getProcessor()->getPortsInSameSet(this)) {
            ImageOutport* portSetImageOutport = dynamic_cast<ImageOutport*>(port);
            if (portSetImageOutport && portSetImageOutport->isConnected()) {
                imageOutport->changeDataDimensions(&resizeEvent);
            }
        }
    }

    DataInport<Image>::connectTo(outport);
}

// set dimensions based on port groups
void ImageInport::changeDataDimensions(ResizeEvent* resizeEvent) {
    dimensions_ = resizeEvent->size();

    // Find the image port with largest dimensions
    for (auto port : getProcessor()->getPortsInSameSet(this)) {
        ImageOutport* imageOutport = dynamic_cast<ImageOutport*>(port);

        if (imageOutport && imageOutport->isConnected()) {
            const uvec2 dim = imageOutport->getDimensions();

            // Largest outport dimensions
            if (dimensions_.x * dimensions_.y < dim.x * dim.y) dimensions_ = dim;
        }
    }

    resizeEvent->setSize(dimensions_);
    propagateResizeToPredecessor(resizeEvent);
}

void ImageInport::propagateResizeToPredecessor(ResizeEvent* resizeEvent) {
    ImageOutport* imageOutport = dynamic_cast<ImageOutport*>(getConnectedOutport());
    if (imageOutport) {
       imageOutport->changeDataDimensions(resizeEvent);
    }
}

void ImageInport::setResizeScale(vec2 scaling) { resizeScale_ = scaling; }

vec2 ImageInport::getResizeScale() { return resizeScale_; }

uvec2 ImageInport::getDimensions() const {
    return dimensions_;
}

const Image* ImageInport::getData() const {
    if (isConnected()) {
        ImageOutport* outport = static_cast<ImageOutport*>(getConnectedOutport());
        if (isOutportDeterminingSize() || dimensions_ == uvec2(8, 8)) {
            return outport->getConstData();
        } else {
            return const_cast<const Image*>(outport->getResizedImageData(dimensions_));
        }
    } else {
        return nullptr;
    }
}

bool ImageInport::isOutportDeterminingSize() const { return outportDeterminesSize_; }

void ImageInport::setOutportDeterminesSize(bool outportDeterminesSize) {
    outportDeterminesSize_ = outportDeterminesSize;
}

std::string ImageInport::getContentInfo() const {
    if (hasData())
        return getData()->getDataInfo();
    else
        return getClassIdentifier() + " has no data.";
}

void ImageInport::passOnDataToOutport(ImageOutport* outport) const {
    if (hasData()) {
        const Image* img = getData();
        Image* out = outport->getData();
        if (out) img->resizeRepresentations(out, out->getDimensions());
    }
}

////////////////////////////// ImageOutport ////////////////////////////////////////////

ImageOutport::ImageOutport(std::string identifier, const DataFormatBase* format,
                           bool handleResizeEvents)
    : DataOutport<Image>(identifier)
    , dimensions_(uvec2(8, 8))
    , cacheInvalid_(true)
    , handleResizeEvents_(handleResizeEvents) {

    setData(new Image(dimensions_, format));
    onSetData();
}

ImageOutport::~ImageOutport() {
    for (auto& elem : imageCache_) {
        if (elem.second != data_) {
            delete elem.second;
        }
    }
}

bool ImageOutport::propagateResizeEventToPredecessor(ResizeEvent* resizeEvent) {
    // Only propagate resize event to inports within the same port dependency group

    bool propagationEnded = true;
    const uvec2 size {resizeEvent->size()};
    const uvec2 prevSize {resizeEvent->previousSize()};

    for (auto port : getProcessor()->getPortsInSameSet(this)) {
        if (ImageInport* imageInport = dynamic_cast<ImageInport*>(port)) {
            propagationEnded = false;
            imageInport->changeDataDimensions(scaleResizeEvent(imageInport, resizeEvent));
            // reset event
            resizeEvent->setSize(size);
            resizeEvent->setPreviousSize(prevSize);
        } else if (MultiDataInport<Image, ImageInport>* multiImageInport =
                       dynamic_cast<MultiDataInport<Image, ImageInport>*>(port)) {
            propagationEnded = false;
            for (auto inport : multiImageInport->getInports()) {
                if (ImageInport* imageInport = dynamic_cast<ImageInport*>(inport)) {
                    imageInport->changeDataDimensions(scaleResizeEvent(imageInport, resizeEvent));
                    // reset event
                    resizeEvent->setSize(size);
                    resizeEvent->setPreviousSize(prevSize);
                }
            }
        }
    }

    return propagationEnded;
}

void ImageOutport::invalidate(InvalidationLevel invalidationLevel) {
    cacheInvalid_ = true;
    Outport::invalidate(invalidationLevel);
}

void ImageOutport::setData(Image* data, bool ownsData /*= true*/) {
    DataOutport<Image>::setData(data, ownsData);
    onSetData();
}

void ImageOutport::setConstData(const Image* data) {
    DataOutport<Image>::setConstData(data);
    onSetData();
}

void ImageOutport::onSetData() {
    std::string dimensionsString;

    // Remove data already deleted by base port class
    if (dimensions_ != data_->getDimensions()) {
        dimensionsString = glm::to_string(dimensions_);
        auto it = imageCache_.find(dimensionsString);
        if (it != imageCache_.end()) {
            imageCache_.erase(it);
        }

        dimensions_ = data_->getDimensions();
        dimensionsString = glm::to_string(data_->getDimensions());
        imageCache_.insert(std::make_pair(dimensionsString, data_));

    } else {
        dimensionsString = glm::to_string(data_->getDimensions());
        auto it = imageCache_.find(dimensionsString);
        if (it != imageCache_.end()) {
            if (it->second != data_) {
                imageCache_.erase(it);
                cacheInvalid_ = true;
                imageCache_.insert(std::make_pair(dimensionsString, data_));
            }
        } else {
            imageCache_.insert(std::make_pair(dimensionsString, data_));
        }
    }
}

void ImageOutport::changeDataDimensions(ResizeEvent* resizeEvent) {
    // This function should check which dimensions request exists, by going through the successors
    // and checking registeredDimensions.
    // Allocates space holder, sets largest data, cleans up un-used data
    uvec2 requiredDimensions = resizeEvent->size();

    // Avoid unwanted propagation
    if (requiredDimensions == uvec2(8, 8))
        return;

    uvec2 previousDimensions = resizeEvent->previousSize();
    std::string prevDimensionString = glm::to_string(previousDimensions);
    std::string reqDimensionString = glm::to_string(requiredDimensions);

    std::vector<Inport*> inports = getConnectedInports();
    std::vector<uvec2> registeredDimensions;

    // Always save data_ dimensions if outport determine output size
    if (!isHandlingResizeEvents()) {
        registeredDimensions.push_back(data_->getDimensions());

        //Fix for data_ size not correct in imageDataMap
        for (std::map<std::string, Image*>::iterator it = imageCache_.begin();
             it != imageCache_.end(); ++it) {
             
            if (it->second == data_){
                std::string dataDim = glm::to_string(registeredDimensions[0]);
                if (it->first != dataDim){
                    imageCache_.erase(it);
                    imageCache_.insert(std::make_pair(dataDim, data_));
                    break;
                }
            }
        }
    }

    for (auto& inport : inports) {
        ImageInport* imageInport = dynamic_cast<ImageInport*>(inport);
        if (imageInport && !imageInport->isOutportDeterminingSize())
            registeredDimensions.push_back(imageInport->getDimensions());
    }

    if (registeredDimensions.empty()){
        registeredDimensions.push_back(resizeEvent->size());
    }

    std::vector<std::string> registeredDimensionsStrings;

    for (auto dimensions : registeredDimensions) {
        std::string dimensionsString = glm::to_string(dimensions);
        registeredDimensionsStrings.push_back(dimensionsString);
    }

    // If requiredDimension does not exist then do the following:
    //  If image data with previousDimensions exists in map and
    //  also does not exist in validDimensions
    //      Resize map data to required dimensions
    //  Else
    //      Clone the current data, resize it and make new entry in map
    Image* resultImage = nullptr;

    if (imageCache_.find(reqDimensionString) != imageCache_.end())
        resultImage = imageCache_[reqDimensionString];

    // requiredDimension does not exist
    if (!resultImage) {

        // Decide whether to resize data with previousDimensions
        bool canResize = false;

        if (std::find(registeredDimensionsStrings.begin(), registeredDimensionsStrings.end(),
                      prevDimensionString) == registeredDimensionsStrings.end())
            canResize = true;

        // Does data with previousDimensions exist
        if (imageCache_.find(prevDimensionString) != imageCache_.end())
            resultImage = imageCache_[prevDimensionString];

        // make sure not to resize data that is not owned
        if (canResize && resultImage && (isDataOwner() || ( !isDataOwner() && resultImage != data_))) {
            // previousDimensions exist. It is no longer needed. So it can be resized.
            // Remove old entry in map( later make new entry)
            imageCache_.erase(prevDimensionString);
        } else {
            // previousDimensions does not exist. So allocate space holder
            resultImage = static_cast<Image*>(data_->clone());
        }

        // Resize the result image, Note that this will likely destroy all the data in the image.
        // we will fill the image with data later. either by running processor::process agaion or
        // by data_->resizeRepresentations(resultImage, resultImage->getDimensions());
        resultImage->resize(requiredDimensions);
        
        // Make new entry
        imageCache_.insert(std::make_pair(reqDimensionString, resultImage));
    }

    // Remove unwanted map data
    std::vector<std::string> invalidImageDataStrings;

    for (auto& elem : imageCache_) {
        if (std::find(registeredDimensionsStrings.begin(), registeredDimensionsStrings.end(),
                      elem.first) == registeredDimensionsStrings.end())
            invalidImageDataStrings.push_back(elem.first);
    }

    // leave at least one data and discard others
    if (imageCache_.size() > 1) {
        for (auto& invalidImageDataString : invalidImageDataStrings) {
            Image* invalidImage = imageCache_[invalidImageDataString];

            //Make sure you don't delete data_
            if (invalidImage != data_) {
                if (invalidImage == resultImage)
                    resultImage = nullptr;
                delete invalidImage;
                imageCache_.erase(invalidImageDataString);
            }
        }
    }

    uvec2 outDim;

    //Don't continue is outport determine output size
    if (isHandlingResizeEvents() || dimensions_ == uvec2(8, 8)) {
        outDim = getDimensions();
        // Set largest data
        setLargestImageData();
    } else {       
        outDim = dimensions_;
    }

    //Send update to listeners
    if (data_->getDimensions() != dimensions_){
        dimensions_ = data_->getDimensions();
        broadcast(resizeEvent);
    }
    else
        dimensions_ = data_->getDimensions();

    // Make sure that all ImageOutports in the same group (dependency set) that has the same size.
    for (auto& elem : getProcessor()->getPortsInSameSet(this)) {
        ImageOutport* imageOutport = dynamic_cast<ImageOutport*>(elem);
        if (imageOutport && imageOutport != this) {
            imageOutport->setDimensions(resizeEvent->size());
        }
    }
    // Propagate the resize event
    bool propagationEnded = propagateResizeEventToPredecessor(resizeEvent);

    if (propagationEnded){
        if (resultImage && resultImage != data_ && data_->getDimensions() != uvec2(0)) {
            data_->resizeRepresentations(resultImage, resultImage->getDimensions());
        }
        getProcessor()->invalidate(INVALID_OUTPUT);
    }
}

uvec2 ImageOutport::getDimensions() const { 
    return dimensions_; 
}

Image* ImageOutport::getResizedImageData(uvec2 requiredDimensions) {
    if (cacheInvalid_) {
        // If data_ dimensionsis zero, we need to update data_ first
        uvec2 zeroDim = uvec2(0);
        if (data_->getDimensions() == zeroDim) {
            data_->getRepresentation<ImageRAM>();

            // Remove any reference to zero sized image and add reference to data_
            if (data_->getDimensions() != zeroDim) {
                std::string zeroDimString = glm::to_string(zeroDim);
                imageCache_.erase(zeroDimString);

                std::string dataDimString = glm::to_string(data_->getDimensions());

                if (imageCache_.find(dataDimString) != imageCache_.end())
                    imageCache_[dataDimString] = data_;
                else
                    imageCache_.insert(
                        std::make_pair(glm::to_string(data_->getDimensions()), data_));
            }
        }

        // Resize all map data once
        bool delete88 = false;
        for (auto& elem : imageCache_) {
            if (elem.second != data_) {
                uvec2 mapDataDimensions = elem.second->getDimensions();
                if (elem.second != data_ && mapDataDimensions == uvec2(8, 8)){
                    delete elem.second;
                    delete88 = true;
                }
                else
                    data_->resizeRepresentations(elem.second, mapDataDimensions);
            }
        }
        if (delete88) imageCache_.erase(glm::to_string(uvec2(8, 8)));

        cacheInvalid_ = false;
    }
    auto it = imageCache_.find(glm::to_string(requiredDimensions));
    if (it != imageCache_.end()){
        return it->second;
    }

    Image* resultImage = data_->clone();
    resultImage->resize(requiredDimensions);
    std::string dimensionsString = glm::to_string(requiredDimensions);
    imageCache_.insert(std::make_pair(dimensionsString, resultImage));
    data_->resizeRepresentations(resultImage, requiredDimensions);
    return resultImage;
}

void ImageOutport::setLargestImageData() {
    uvec2 maxDimensions(0);
    Image* largestImage = nullptr;

    for (auto& elem : imageCache_) {
        uvec2 mapDataDimensions = elem.second->getDimensions();

        if ((maxDimensions.x * maxDimensions.y) < (mapDataDimensions.x * mapDataDimensions.y)) {
            maxDimensions = mapDataDimensions;
            largestImage = elem.second;
        }
    }

    // Check if data_ is not longer largest image.
    if (largestImage && data_ != largestImage) {
        data_ = largestImage;
        cacheInvalid_ = true;
    }
}

bool ImageOutport::addResizeEventListener(EventListener* el) { return addEventListener(el); }

bool ImageOutport::removeResizeEventListener(EventListener* el) { return removeEventListener(el); }

void ImageOutport::setDimensions(const uvec2& newDimension) {
    dimensions_ = newDimension;
    // Clear data
    onSetData();
    // Set new dimensions
    DataOutport<Image>::getData()->resize(newDimension);
}

ResizeEvent* ImageOutport::scaleResizeEvent(ImageInport* imageInport, ResizeEvent* sizeEvent) {
    vec2 scale = imageInport->getResizeScale();
    sizeEvent->setPreviousSize(imageInport->getDimensions());
    sizeEvent->setSize(
        uvec2(static_cast<unsigned int>(static_cast<float>(sizeEvent->size().x * scale.x)),
              static_cast<unsigned int>(static_cast<float>(sizeEvent->size().y * scale.y))));
    return sizeEvent;
}

void ImageOutport::setHandleResizeEvents(bool handleResizeEvents) {
    handleResizeEvents_ = handleResizeEvents;
}

bool ImageOutport::isHandlingResizeEvents() const {
    return handleResizeEvents_;
}

}  // namespace
