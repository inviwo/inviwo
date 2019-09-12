/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/image/imageram.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/util/document.h>

namespace inviwo {

Image::Image(size2_t dimensions, const DataFormatBase* format)
    : DataGroup<Image, ImageRepresentation>()
    , MetaDataOwner()
    , depthLayer_{createDepthLayer(dimensions)}
    , pickingLayer_{createPickingLayer(dimensions, format)} {
    colorLayers_.push_back(createColorLayer(dimensions, format));
}

Image::Image(std::shared_ptr<Layer> layer)
    : DataGroup<Image, ImageRepresentation>(), MetaDataOwner() {
    if (layer) {
        auto dimensions = layer->getDimensions();

        switch (layer->getLayerType()) {
            case LayerType::Color: {
                auto format = layer->getDataFormat();
                colorLayers_.push_back(layer);
                depthLayer_ = createDepthLayer(dimensions);
                pickingLayer_ = createPickingLayer(dimensions, format);
                break;
            }
            case LayerType::Depth: {
                colorLayers_.push_back(createColorLayer(dimensions));
                depthLayer_ = layer;
                pickingLayer_ = createPickingLayer(dimensions);
                break;
            }
            case LayerType::Picking: {
                colorLayers_.push_back(createColorLayer(dimensions));
                depthLayer_ = createDepthLayer(dimensions);
                pickingLayer_ = layer;
                break;
            }
        }
    } else {
        colorLayers_.push_back(createColorLayer());
        depthLayer_ = createDepthLayer();
        pickingLayer_ = createPickingLayer();
    }
}

Image::Image(const Image& rhs) : DataGroup<Image, ImageRepresentation>(rhs), MetaDataOwner(rhs) {
    for (const auto& elem : rhs.colorLayers_) {
        colorLayers_.push_back(std::shared_ptr<Layer>(elem->clone()));
    }

    if (auto depth = rhs.getDepthLayer()) depthLayer_ = std::shared_ptr<Layer>(depth->clone());

    if (auto picking = rhs.getPickingLayer())
        pickingLayer_ = std::shared_ptr<Layer>(picking->clone());
}

Image& Image::operator=(const Image& that) {
    if (this != &that) {
        DataGroup<Image, ImageRepresentation>::operator=(that);
        MetaDataOwner::operator=(that);

        std::vector<std::shared_ptr<Layer>> colorLayers;
        for (const auto& color : that.colorLayers_) {
            colorLayers.push_back(std::shared_ptr<Layer>(color->clone()));
        }

        auto depth = that.getDepthLayer();
        auto depthLayer = depth ? std::shared_ptr<Layer>(depth->clone()) : std::shared_ptr<Layer>();

        auto picking = that.getPickingLayer();
        auto pickingLayer =
            picking ? std::shared_ptr<Layer>(picking->clone()) : std::shared_ptr<Layer>();

        std::swap(colorLayers, colorLayers_);
        std::swap(depthLayer, depthLayer_);
        std::swap(pickingLayer, pickingLayer_);
    }

    return *this;
}

Image* Image::clone() const { return new Image(*this); }

std::shared_ptr<Layer> Image::createColorLayer(size2_t dimensions, const DataFormatBase* format) {
    return std::make_shared<Layer>(dimensions, format, LayerType::Color);
}
std::shared_ptr<Layer> Image::createDepthLayer(size2_t dimensions) {
    return std::make_shared<Layer>(dimensions, DataFloat32::get(), LayerType::Depth,
                                   swizzlemasks::depth);
}
std::shared_ptr<Layer> Image::createPickingLayer(size2_t dimensions, const DataFormatBase* format) {
    return std::make_shared<Layer>(dimensions, format, LayerType::Picking);
}

const Layer* Image::getLayer(LayerType type, size_t idx) const {
    switch (type) {
        case LayerType::Color:
            return getColorLayer(idx);
        case LayerType::Depth:
            return getDepthLayer();
        case LayerType::Picking:
            return getPickingLayer();
    }
    return nullptr;
}

Layer* Image::getLayer(LayerType type, size_t idx) {
    switch (type) {
        case LayerType::Color:
            return getColorLayer(idx);
        case LayerType::Depth:
            return getDepthLayer();
        case LayerType::Picking:
            return getPickingLayer();
    }
    return nullptr;
}

const Layer* Image::getColorLayer(size_t idx) const { return colorLayers_[idx].get(); }

Layer* Image::getColorLayer(size_t idx) { return colorLayers_[idx].get(); }

void Image::addColorLayer(std::shared_ptr<Layer> layer) { colorLayers_.push_back(layer); }

size_t Image::getNumberOfColorLayers() const { return colorLayers_.size(); }

const Layer* Image::getDepthLayer() const { return depthLayer_.get(); }

Layer* Image::getDepthLayer() { return depthLayer_.get(); }

const Layer* Image::getPickingLayer() const { return pickingLayer_.get(); }

Layer* Image::getPickingLayer() { return pickingLayer_.get(); }

size2_t Image::getDimensions() const { return getColorLayer()->getDimensions(); }

void Image::setDimensions(size2_t dimensions) {
    for (auto layer : colorLayers_) layer->setDimensions(dimensions);
    if (depthLayer_) depthLayer_->setDimensions(dimensions);
    if (pickingLayer_) pickingLayer_->setDimensions(dimensions);
}

std::unique_ptr<std::vector<unsigned char>> Image::getLayerAsCodedBuffer(
    LayerType layerType, const std::string& fileExtension, size_t idx) const {

    if (auto layer = this->getLayer(layerType, idx)) {
        return layer->getAsCodedBuffer(fileExtension);
    } else {
        LogError("Requested layer does not exist");
    }
    return nullptr;
}

std::unique_ptr<std::vector<unsigned char>> Image::getColorLayerAsCodedBuffer(
    const std::string& fileExtension, size_t idx) const {
    return getLayerAsCodedBuffer(LayerType::Color, fileExtension, idx);
}

std::unique_ptr<std::vector<unsigned char>> Image::getDepthLayerAsCodedBuffer(
    const std::string& fileExtension) const {
    return getLayerAsCodedBuffer(LayerType::Depth, fileExtension);
}

std::unique_ptr<std::vector<unsigned char>> Image::getPickingLayerAsCodedBuffer(
    const std::string& fileExtension) const {
    return getLayerAsCodedBuffer(LayerType::Picking, fileExtension);
}

void Image::copyRepresentationsTo(Image* targetImage) const {
    // Scheme: Only ask for one editable representations to resize
    // Thus all others can update from one resized version
    // Do the resizing on the representation with the highest priority

    auto& targets = targetImage->representations_;
    std::vector<std::pair<size_t, std::type_index>> order;
    for (const auto& elem : targets) order.emplace_back(elem.second->priority(), elem.first);
    std::sort(order.begin(), order.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });

    for (const auto& item : order) {
        const auto sIt = representations_.find(item.second);
        if (sIt != representations_.end()) {
            auto sourceRepr = sIt->second.get();
            auto targetRepr = targets[item.second].get();
            if (typeid(*sourceRepr) == typeid(*targetRepr)) {
                sourceRepr->update(false);
                targetRepr->update(true);
                if (sourceRepr->copyRepresentationsTo(targetRepr)) {
                    return;
                } else {
                    LogError("Copy representation failed!")
                }
            }
        }
    }

    // Fallback. If no representation exist, create ImageRAM one
    const ImageRAM* imageRAM = this->getRepresentation<ImageRAM>();
    if (!imageRAM->copyRepresentationsTo(targetImage->getEditableRepresentation<ImageRAM>())) {
        throw Exception("Failed to copy Image Representation", IVW_CONTEXT);
    }
}

const DataFormatBase* Image::getDataFormat() const { return getColorLayer()->getDataFormat(); }

dvec4 Image::readPixel(size2_t pos, LayerType layer, size_t index) const {
    std::vector<std::pair<size_t, ImageRepresentation*>> order;
    for (const auto& elem : representations_) {
        order.emplace_back(elem.second->priority(), elem.second.get());
    }
    std::sort(order.begin(), order.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });

    auto it = util::find_if(order, [&](const auto& elem) { return elem.second->isValid(); });
    if (it != order.end()) {
        return it->second->readPixel(pos, layer, index);
    }
    return dvec4(0.0);
}

uvec3 Image::colorCode = uvec3(90, 127, 183);
const std::string Image::classIdentifier = "org.inviwo.Image";
const std::string Image::dataName = "Image";

Document Image::getInfo() const {
    using H = utildoc::TableBuilder::Header;
    using P = Document::PathComponent;
    Document doc;
    doc.append("b", "Image", {{"style", "color:white;"}});
    utildoc::TableBuilder tb(doc.handle(), P::end());
    tb(H("Color channels"), colorLayers_.size());
    tb(H("Depth"), getDepthLayer() ? "Yes" : "No");
    tb(H("Picking"), getPickingLayer() ? "Yes" : "No");
    tb(H("Format"), getDataFormat()->getString());
    auto dims = getDimensions();
    tb(H("Dimension"), dims);
    tb(H("Aspect Ratio"),
       dims.y == 0 ? "Invalid"
                   : toString(static_cast<double>(dims.x) / static_cast<double>(dims.y)));

    return doc;
}

template class IVW_CORE_TMPL_INST DataReaderType<Image>;
template class IVW_CORE_TMPL_INST DataWriterType<Image>;

}  // namespace inviwo
