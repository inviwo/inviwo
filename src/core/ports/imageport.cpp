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

uvec3 ImageInport::colorCode = uvec3(90, 127, 183);

// Image Inport
ImageInport::ImageInport(std::string identifier, bool outportDeterminesSize,
                         InvalidationLevel invalidationLevel)
    : DataInport<Image>(identifier, invalidationLevel)
    , dimensions_(uvec2(256, 256))
    , resizeScale_(vec2(1.f, 1.f))
    , outportDeterminesSize_(outportDeterminesSize) {}

ImageInport::~ImageInport() {}

void ImageInport::connectTo(Outport* outport) {
    connectedOutport_ = outport;
    outport->connectTo(this);

    if (getProcessor()->isEndProcessor()) {
        // Resize outport if no outports exist (Canvas)
        ResizeEvent resizeEvent(getDimensions());
        ImageOutport* connectedImageOutport = dynamic_cast<ImageOutport*>(outport);
        connectedImageOutport->changeDataDimensions(&resizeEvent);
    } else if (!isOutportDeterminingSize()) {
        // Resize outport if any outport within the same port dependency set is connected
        std::vector<Port*> portSet =
            getProcessor()->getPortsByDependencySet(getProcessor()->getPortDependencySet(this));
        for (size_t j = 0; j < portSet.size(); j++) {
            ImageOutport* imageOutport = dynamic_cast<ImageOutport*>(portSet[j]);
            if (imageOutport && imageOutport->isConnected()) {
                ResizeEvent resizeEvent(getDimensions());
                ImageOutport* connectedImageOutport = dynamic_cast<ImageOutport*>(outport);
                connectedImageOutport->changeDataDimensions(&resizeEvent);
            }
        }
    }

    invalidate(INVALID_OUTPUT);
}

void ImageInport::changeDataDimensions(ResizeEvent* resizeEvent) {
    uvec2 dimensions = resizeEvent->size();
    // set dimensionsbased on port groups
    std::vector<std::string> portDependencySets = getProcessor()->getPortDependencySets();
    std::vector<Port*> portSet;
    uvec2 dimMax(0);
    bool hasImageOutport = false;

    for (size_t i = 0; i < portDependencySets.size(); i++) {
        portSet.clear();
        // get ports that belong to the dependency set portDependencySets[i]
        portSet = getProcessor()->getPortsByDependencySet(portDependencySets[i]);

        // check if current port belong to portSet
        if (std::find(portSet.begin(), portSet.end(), this) != portSet.end()) {
            // Find the image port with largest dimensions
            for (size_t j = 0; j < portSet.size(); j++) {
                ImageOutport* imageOutport = dynamic_cast<ImageOutport*>(portSet[j]);

                if (imageOutport && imageOutport->isConnected()) {
                    hasImageOutport = true;
                    uvec2 dim = imageOutport->getDimensions();

                    // Largest outport dimensions
                    if (dimMax.x * dimMax.y < dim.x * dim.y) dimMax = imageOutport->getDimensions();
                }
            }
        }
    }

    if (!hasImageOutport)
        dimensions_ = dimensions;
    else
        dimensions_ = dimMax;
    
    resizeEvent->setSize(dimensions_);
    propagateResizeToPredecessor(resizeEvent);
    
    invalidate(INVALID_OUTPUT);
}

void ImageInport::propagateResizeToPredecessor(ResizeEvent* resizeEvent) {
    ImageOutport* imageOutport = dynamic_cast<ImageOutport*>(getConnectedOutport());
    if (imageOutport && !isOutportDeterminingSize()) {
        imageOutport->changeDataDimensions(resizeEvent);
    }
}

void ImageInport::setResizeScale(vec2 scaling) { resizeScale_ = scaling; }

vec2 ImageInport::getResizeScale() { return resizeScale_; }

uvec2 ImageInport::getDimensions() const {
    if (isOutportDeterminingSize() && isConnected()) {
        ImageOutport* outport = dynamic_cast<ImageOutport*>(getConnectedOutport());
        return outport->getDimensions();
    } else {
        return dimensions_;
    }
}

const Image* ImageInport::getData() const {
    if (isConnected()) {
        ImageOutport* outport = dynamic_cast<ImageOutport*>(getConnectedOutport());
        if (isOutportDeterminingSize()) {
            return outport->getConstData();
        } else {
            return const_cast<const Image*>(outport->getResizedImageData(dimensions_));
        }
    } else {
        return NULL;
    }
}

uvec3 ImageInport::getColorCode() const { return ImageInport::colorCode; }

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
                           InvalidationLevel invalidationLevel,
                           bool handleResizeEvents)
    : DataOutport<Image>(identifier, invalidationLevel)
    , dimensions_(uvec2(256, 256))
    , mapDataInvalid_(true)
    , handleResizeEvents_(handleResizeEvents) {
    
    setData(new Image(dimensions_, format));
    dataChanged();
}

ImageOutport::~ImageOutport() {
    for (ImagePortMap::iterator it = imageDataMap_.begin(); it != imageDataMap_.end(); ++it) {
        if (isDataOwner() || it->second != data_) {
            delete it->second;
        }
    }
    data_ = NULL;  // As data_ is referenced in imageDataMap_.
}

void ImageOutport::propagateResizeEventToPredecessor(ResizeEvent* resizeEvent) {
    if (!isHandlingResizeEvents()) {
        return;
    }
    // Only propagate resize event to inports within the same port dependency group
    std::vector<Port*> portSet =
        getProcessor()->getPortsByDependencySet(getProcessor()->getPortDependencySet(this));
    bool propagationEnded = true;
    uvec2 size = resizeEvent->size();
    uvec2 prevSize = resizeEvent->previousSize();
    for (size_t j = 0; j < portSet.size(); j++) {
        ImageInport* imageInport = dynamic_cast<ImageInport*>(portSet[j]);
        if (imageInport) {
            propagationEnded = false;
            imageInport->changeDataDimensions(scaleResizeEvent(imageInport, resizeEvent));
            resizeEvent->setSize(size);
            resizeEvent->setPreviousSize(prevSize);
        } else {
            MultiDataInport<Image, ImageInport>* multiImageInport =
                dynamic_cast<MultiDataInport<Image, ImageInport>*>(portSet[j]);
            if (multiImageInport) {
                propagationEnded = false;
                std::vector<Inport*> inports = multiImageInport->getInports();
                for (size_t i = 0; i < inports.size(); ++i) {
                    ImageInport* imageInport = dynamic_cast<ImageInport*>(inports[i]);
                    if (imageInport) {
                        imageInport->changeDataDimensions(
                            scaleResizeEvent(imageInport, resizeEvent));
                        resizeEvent->setSize(size);
                        resizeEvent->setPreviousSize(prevSize);
                    }
                }
            }
        }
    }

    if (propagationEnded) {
        getProcessor()->invalidate(INVALID_OUTPUT);
    }
}

void ImageOutport::invalidate(InvalidationLevel invalidationLevel) {
    mapDataInvalid_ = true;
    Outport::invalidate(invalidationLevel);
}

Image* ImageOutport::getData() {
    return DataOutport<Image>::getData();
}

void ImageOutport::dataChanged() {
    imageDataMap_.clear();
    std::string dimensionsString = glm::to_string(data_->getDimensions());
    imageDataMap_.insert(std::make_pair(dimensionsString, data_));
}

void ImageOutport::changeDataDimensions(ResizeEvent* resizeEvent) {
    // This function should check which dimensionsrequest exists, by going through the successors
    // and checking registeredDimensions.
    // We do only want to propagate when there is not a registeredDimension which is larger then the
    // resizeevent size.
    // Allocates space holder, sets largest data, cleans up un-used data
    uvec2 requiredDimensions = resizeEvent->size();
    uvec2 previousDimensions = resizeEvent->previousSize();
    std::string prevDimensionString = glm::to_string(previousDimensions);
    std::string reqDimensionString = glm::to_string(requiredDimensions);

    std::vector<Inport*> inports = getConnectedInports();
    std::vector<uvec2> registeredDimensions;

    // Always save data_ dimensionsif outport determine output size
    if (!handleResizeEvents_) {
        registeredDimensions.push_back(data_->getDimensions());

        //Fix for data_ size not correct in imageDataMap
        for (std::map<std::string, Image*>::iterator it = imageDataMap_.begin();
             it != imageDataMap_.end(); ++it) {
             
            if (it->second == data_){
                std::string dataDim = glm::to_string(registeredDimensions[0]);
                if (it->first != dataDim){
                    imageDataMap_.erase(it);
                    imageDataMap_.insert(std::make_pair(dataDim, data_));
                    break;
                }
            }
        }
    }

    for (size_t i = 0; i < inports.size(); i++) {
        ImageInport* imageInport = dynamic_cast<ImageInport*>(inports[i]);
        if (imageInport)
            registeredDimensions.push_back(imageInport->getDimensions());
    }

    if (registeredDimensions.empty()) return;

    std::vector<std::string> registeredDimensionsStrings;

    for (size_t i = 0; i < registeredDimensions.size(); i++) {
        uvec2 dimensions = registeredDimensions[i];
        std::string dimensionsString = glm::to_string(dimensions);
        registeredDimensionsStrings.push_back(dimensionsString);
    }

    // If requiredDimension does not exist then do the following:
    //  If image data with previousDimensions exists in map and
    //  also does not exist in validDimensions
    //      Resize map data to required dimensions
    //  Else
    //      Clone the current data, resize it and make new entry in map
    Image* resultImage = 0;

    if (imageDataMap_.find(reqDimensionString) != imageDataMap_.end())
        resultImage = imageDataMap_[reqDimensionString];

    // requiredDimension does not exist
    if (!resultImage) {

        // Decide whether to resize data with previousDimensions
        bool canResize = false;

        if (std::find(registeredDimensionsStrings.begin(), registeredDimensionsStrings.end(),
                      prevDimensionString) == registeredDimensionsStrings.end())
            canResize = true;

        // Does data with previousDimensions exist
        if (imageDataMap_.find(prevDimensionString) != imageDataMap_.end())
            resultImage = imageDataMap_[prevDimensionString];

        // make sure not to resize data that is not owned
        if (canResize && resultImage && (isDataOwner() || ( !isDataOwner() && resultImage != data_))) {
            // previousDimensions exist. It is no longer needed. So it can be resized.
            // Remove old entry in map( later make new entry)
            imageDataMap_.erase(prevDimensionString);
        } else {
            // previousDimensions does not exist. So allocate space holder
            resultImage = static_cast<Image*>(data_->clone());
        }

        // Resize the result image, Note that this will likely destroy all the data in the image.
        // we will fill the image with data later. either by running processor::process agaion or
        // by data_->resizeRepresentations(resultImage, resultImage->getDimensions());
        resultImage->resize(requiredDimensions);
        
        // Make new entry
        imageDataMap_.insert(std::make_pair(reqDimensionString, resultImage));
    }

    // Remove unwanted map data
    std::vector<std::string> invalidImageDataStrings;

    for (ImagePortMap::iterator it = imageDataMap_.begin(); it != imageDataMap_.end(); ++it) {
        if (std::find(registeredDimensionsStrings.begin(), registeredDimensionsStrings.end(),
                      it->first) == registeredDimensionsStrings.end())
            invalidImageDataStrings.push_back(it->first);
    }

    // leave at least one data and discard others
    if (imageDataMap_.size() > 1) {
        for (size_t i = 0; i < invalidImageDataStrings.size(); i++) {
            Image* invalidImage = imageDataMap_[invalidImageDataStrings[i]];
            
            //Make sure you don't delete data thats not owned
            if (isDataOwner() || invalidImage != data_) {
                imageDataMap_.erase(invalidImageDataStrings[i]);
                delete invalidImage;
            }
        }
    }

    uvec2 outDim;

    //Don't continue is outport determine output size
    if (handleResizeEvents_) {
        outDim = getDimensions();
        // Set largest data
        setLargestImageData(resizeEvent);
    } else {
        // Send update to listeners
        if (data_->getDimensions() != dimensions_) broadcast(resizeEvent);

        dimensions_ = data_->getDimensions();
        outDim = dimensions_;
    }

    // Stop resize propagation if outport change hasn't change.
    // Invalid to output new size
    if (outDim != getDimensions()) {
        // Make sure that all ImageOutports in the same group (dependency set) that has the same size.
        std::vector<Port*> portSet = getProcessor()->getPortsByDependencySet(getProcessor()->getPortDependencySet(this));
        for (size_t j = 0; j < portSet.size(); j++) {
            ImageOutport* imageOutport = dynamic_cast<ImageOutport*>(portSet[j]);
            if (imageOutport && imageOutport != this) {
                imageOutport->setDimensions(resizeEvent->size());
            }
        }
        // Propagate the resize event
        propagateResizeEventToPredecessor(resizeEvent);
    } else if (resultImage && resultImage != data_ && data_->getDimensions() != uvec2(0)) {
        data_->resizeRepresentations(resultImage, resultImage->getDimensions());
    } 
}

uvec2 ImageOutport::getDimensions() const { 
    return dimensions_; 
}

Image* ImageOutport::getResizedImageData(uvec2 requiredDimensions) {
    if (mapDataInvalid_) {
        // If data_ dimensionsis zero, we need to update data_ first
        uvec2 zeroDim = uvec2(0);
        if (data_->getDimensions() == zeroDim) {
            data_->getRepresentation<ImageRAM>();

            // Remove any reference to zero sized image and add reference to data_
            if (data_->getDimensions() != zeroDim) {
                std::string zeroDimString = glm::to_string(zeroDim);
                imageDataMap_.erase(zeroDimString);

                std::string dataDimString = glm::to_string(data_->getDimensions());

                if (imageDataMap_.find(dataDimString) != imageDataMap_.end())
                    imageDataMap_[dataDimString] = data_;
                else
                    imageDataMap_.insert(
                        std::make_pair(glm::to_string(data_->getDimensions()), data_));
            }
        }

        // Resize all map data once
        for (ImagePortMap::iterator it = imageDataMap_.begin(); it != imageDataMap_.end(); ++it) {
            if (it->second != data_) {
                uvec2 mapDataDimensions = it->second->getDimensions();
                data_->resizeRepresentations(it->second, mapDataDimensions);
            }
        }

        mapDataInvalid_ = false;
    }

    // TODO: Map traverse is expensive. Optimize
    for (ImagePortMap::iterator it = imageDataMap_.begin(); it != imageDataMap_.end(); ++it) {
        if (it->second->getDimensions() == requiredDimensions) {
            return it->second;
        } else if (it->first == glm::to_string(requiredDimensions)) {
            it->second->resize(requiredDimensions);
            return it->second;
        }
    }

    // Image* resultImage = new Image(requiredDimensions, data_->getImageType(),
    // data_->getDataFormat());
    Image* resultImage = data_->clone();
    resultImage->resize(requiredDimensions);
    std::string dimensionsString = glm::to_string(requiredDimensions);
    imageDataMap_.insert(std::make_pair(dimensionsString, resultImage));
    data_->resizeRepresentations(resultImage, requiredDimensions);
    return resultImage;
}

void ImageOutport::setLargestImageData(ResizeEvent* resizeEvent) {
    uvec2 maxDimensions(0);
    Image* largestImage = 0;

    for (ImagePortMap::iterator it = imageDataMap_.begin(); it != imageDataMap_.end(); ++it) {
        uvec2 mapDataDimensions = it->second->getDimensions();

        if ((maxDimensions.x * maxDimensions.y) < (mapDataDimensions.x * mapDataDimensions.y)) {
            maxDimensions = mapDataDimensions;
            largestImage = it->second;
        }
    }

    // Check if data_ is not longer largest image.
    if (largestImage && data_ != largestImage) {
        data_ = largestImage;
        mapDataInvalid_ = true;
    }

    // Send update to listeners
    if (data_->getDimensions() != dimensions_) broadcast(resizeEvent);

    dimensions_ = data_->getDimensions();
}

uvec3 ImageOutport::getColorCode() const { return ImageInport::colorCode; }

bool ImageOutport::addResizeEventListener(EventListener* el) { return addEventListener(el); }

bool ImageOutport::removeResizeEventListener(EventListener* el) { return removeEventListener(el); }

void ImageOutport::setDimensions(const uvec2& newDimension) {
    dimensions_ = newDimension;
    // Clear data
    dataChanged();
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

std::string ImageOutport::getClassIdentifier() const {
    return "org.inviwo.ImageOutport";
}

}  // namespace
