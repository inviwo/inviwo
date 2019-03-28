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

#ifndef IVW_IMAGE_H
#define IVW_IMAGE_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/datastructures/datagroup.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/imagetypes.h>
#include <inviwo/core/datastructures/image/imagerepresentation.h>
#include <inviwo/core/metadata/metadataowner.h>
#include <inviwo/core/util/document.h>
#include <inviwo/core/io/datareader.h>
#include <inviwo/core/io/datawriter.h>

namespace inviwo {

/**
 * \ingroup datastructures
 */
class IVW_CORE_API Image : public DataGroup<Image, ImageRepresentation>, public MetaDataOwner {
public:
    using DataBuffer = std::unique_ptr<std::vector<unsigned char>>;

    Image(size2_t dimensions = size2_t(8, 8), const DataFormatBase* format = DataVec4UInt8::get());
    Image(std::shared_ptr<Layer> layer);
    Image(const Image&);
    Image& operator=(const Image& that);
    virtual Image* clone() const;
    virtual ~Image() = default;
    virtual Document getInfo() const;

    const Layer* getLayer(LayerType, size_t idx = 0) const;
    Layer* getLayer(LayerType, size_t idx = 0);

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

    static uvec3 colorCode;
    static const std::string classIdentifier;
    static const std::string dataName;

protected:
    static std::shared_ptr<Layer> createColorLayer(
        size2_t dimensions = size2_t(8, 8), const DataFormatBase* format = DataVec4UInt8::get());
    static std::shared_ptr<Layer> createDepthLayer(size2_t dimensions = size2_t(8, 8));
    static std::shared_ptr<Layer> createPickingLayer(
        size2_t dimensions = size2_t(8, 8), const DataFormatBase* format = DataVec4UInt8::get());

    std::vector<std::shared_ptr<Layer>> colorLayers_;
    std::shared_ptr<Layer> depthLayer_;
    std::shared_ptr<Layer> pickingLayer_;
};

// https://docs.microsoft.com/en-us/cpp/cpp/general-rules-and-limitations?view=vs-2017
extern template class IVW_CORE_TMPL_EXP DataReaderType<Image>;
extern template class IVW_CORE_TMPL_EXP DataWriterType<Image>;

}  // namespace inviwo

#endif  // IVW_IMAGE_H
