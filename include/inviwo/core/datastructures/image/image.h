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
#include <inviwo/core/datastructures/datagroup.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/imagetypes.h>
#include <inviwo/core/datastructures/image/imagerepresentation.h>
#include <inviwo/core/metadata/metadataowner.h>
#include <inviwo/core/util/document.h>
#include <inviwo/core/io/datareader.h>
#include <inviwo/core/io/datawriter.h>
#include <inviwo/core/util/indirectiterator.h>

namespace inviwo {

/**
 * \ingroup datastructures
 */
class IVW_CORE_API Image : public DataGroup<Image, ImageRepresentation>, public MetaDataOwner {
public:
    using ColorLayerIterator =
        util::IndirectIterator<std::vector<std::shared_ptr<Layer>>::iterator>;
    using ConstColorLayerIterator =
        util::IndirectIterator<std::vector<std::shared_ptr<Layer>>::const_iterator>;

    using DataBuffer = std::unique_ptr<std::vector<unsigned char>>;

    /**
     * @brief Create a new image with @p dimensions and @p format.
     * The image will hold one color layer, one depth layer and one picking layer.
     * The layers will not have any representations.
     * @param dimensions of the new image
     * @param format of the new image
     */
    Image(size2_t dimensions = size2_t(8, 8), const DataFormatBase* format = DataVec4UInt8::get());

    /**
     * @brief Create an Image from the given layers.
     * Any number of color layers can be added. The color layers will be added in the same order as
     * in the list. For any LayerType not in the list default layers will added.
     * @pre The list of layers may only contain one Depth and one Picking layers.
     * @pre All Layers in the list must have the same dimensions.
     * @param layers to use
     */
    Image(std::vector<std::shared_ptr<Layer>> layers);

    /**
     * @brief Create a new image from the given layer.
     * Default layers will be added for the other two LayerTypes
     * @param layer to use
     */
    Image(std::shared_ptr<Layer> layer);

    Image(const Image& rhs);
    /**
     * Create an image based on \p rhs without copying any data. If \p colorLayerFormat is a
     * nullptr, the format of color layers matches the ones in \p rhs.
     * @param rhs             source image providing the necessary information for all layers like
     *                        dimensions, spatial transformations, etc.
     * @param colorLayerFormat   data format for color layers. If equal to nullptr, the formats of
     *                        the color layers in \p rhs are used instead.
     */
    Image(const Image& rhs, NoData, const DataFormatBase* colorLayerFormat = nullptr);
    Image& operator=(const Image& that);
    virtual Image* clone() const;
    virtual ~Image() = default;
    virtual Document getInfo() const;

    const Layer* getLayer(LayerType, size_t idx = 0) const;
    Layer* getLayer(LayerType, size_t idx = 0);

    ColorLayerIterator begin();
    ColorLayerIterator end();

    ConstColorLayerIterator begin() const;
    ConstColorLayerIterator end() const;

    ConstColorLayerIterator cbegin() const;
    ConstColorLayerIterator cend() const;

    const Layer* getColorLayer(size_t idx = 0) const;
    Layer* getColorLayer(size_t idx = 0);
    void addColorLayer(std::shared_ptr<Layer> layer);

    size_t getNumberOfColorLayers() const;

    const Layer* getDepthLayer() const;
    Layer* getDepthLayer();

    const Layer* getPickingLayer() const;
    Layer* getPickingLayer();

    size2_t getDimensions() const;

    /**
     * Resize all representation to dimension. This is destructive, the data will not be
     * preserved. Use copyRepresentationsTo to update the data.
     */
    void setDimensions(size2_t dimensions);

    /**
     * \brief encode the requested layer contents to a buffer considering the given image extension
     * @param layerType Indicate which type of layer to return. see LayerType
     * @param fileExtension file extension of the requested image format
     * @param idx In case of layerType being LayerType::ColorLayer, than return color layer at
     * index idx
     * @return encoded layer contents as std::vector
     */
    DataBuffer getLayerAsCodedBuffer(LayerType layerType, const std::string& fileExtension,
                                     size_t idx = 0) const;
    DataBuffer getColorLayerAsCodedBuffer(const std::string& fileExtension, size_t idx = 0) const;
    DataBuffer getDepthLayerAsCodedBuffer(const std::string& fileExtension) const;
    DataBuffer getPickingLayerAsCodedBuffer(const std::string& fileExtension) const;

    /**
     * Copy and resize the representation of this onto the representations of target.
     * Does not change the dimensions of target.
     */
    void copyRepresentationsTo(Image* target) const;

    const DataFormatBase* getDataFormat() const;

    /**
     * Read a single pixel value out of the specified layer at pos. Should only be used to read
     * single values not entire images.
     */
    dvec4 readPixel(size2_t pos, LayerType layer, size_t index = 0) const;

    /**
     * Call the given \p callback for each layer including depth and picking, if existing. The
     * signature of the callback is `void(Layer&)`.
     */
    template <typename C>
    void forEachLayer(C callback);
    /**
     * Call the given \p callback for each layer including depth and picking, if existing. The
     * signature of the callback is `void(const Layer&)`.
     */
    template <typename C>
    void forEachLayer(C callback) const;

    void updateResource(const ResourceMeta& meta) const;

    static constexpr uvec3 colorCode{90, 127, 183};
    static constexpr std::string_view classIdentifier{"org.inviwo.Image"};
    static constexpr std::string_view dataName{"Image"};

protected:
    static std::shared_ptr<Layer> createColorLayer(
        size2_t dimensions = size2_t(8, 8), const DataFormatBase* format = DataVec4UInt8::get());
    static std::vector<std::shared_ptr<Layer>> createColorLayers(
        const Image& srcImage, const DataFormatBase* format = nullptr);
    static std::shared_ptr<Layer> createDepthLayer(size2_t dimensions = size2_t(8, 8));
    static std::shared_ptr<Layer> createPickingLayer(size2_t dimensions = size2_t(8, 8));

    std::vector<std::shared_ptr<Layer>> colorLayers_;
    std::shared_ptr<Layer> depthLayer_;
    std::shared_ptr<Layer> pickingLayer_;
};

// https://docs.microsoft.com/en-us/cpp/cpp/general-rules-and-limitations?view=vs-2017
extern template class IVW_CORE_TMPL_EXP DataReaderType<Image>;
extern template class IVW_CORE_TMPL_EXP DataWriterType<Image>;

template <typename C>
void Image::forEachLayer(C callback) {
    for (auto& layer : colorLayers_) callback(*layer);
    if (depthLayer_) callback(*depthLayer_);
    if (pickingLayer_) callback(*pickingLayer_);
}

template <typename C>
void Image::forEachLayer(C callback) const {
    for (auto& layer : colorLayers_) callback(static_cast<const Layer&>(*layer));
    if (depthLayer_) callback(static_cast<const Layer&>(*depthLayer_));
    if (pickingLayer_) callback(static_cast<const Layer&>(*pickingLayer_));
}

}  // namespace inviwo
