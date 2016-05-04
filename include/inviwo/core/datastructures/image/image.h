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

#ifndef IVW_IMAGE_H
#define IVW_IMAGE_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/datastructures/datagroup.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/imagetypes.h>
#include <inviwo/core/datastructures/image/imagerepresentation.h>

namespace inviwo {

class IVW_CORE_API Image : public DataGroup<ImageRepresentation> {
public:
    Image(size2_t dimensions = size2_t(8, 8), const DataFormatBase* format = DataVec4UInt8::get());
    Image(std::shared_ptr<Layer> layer);
    Image(const Image&);
    Image& operator=(const Image& that);
    virtual Image* clone() const;
    virtual ~Image() = default;
    virtual std::string getDataInfo() const;

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
     * Copy and resize the representation of this onto the representations of target.
     * Does not change the dimensions of target.
     */
    void copyRepresentationsTo(Image* target) const;

    const DataFormatBase* getDataFormat() const;

    static uvec3 COLOR_CODE;
    static const std::string CLASS_IDENTIFIER;

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

}  // namespace

#endif  // IVW_IMAGE_H
