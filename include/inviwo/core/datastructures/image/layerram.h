/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#ifndef IVW_LAYERRAM_H
#define IVW_LAYERRAM_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/datastructures/image/layerrepresentation.h>
#include <inviwo/core/util/formats.h>

namespace inviwo {

class IVW_CORE_API LayerRAM : public LayerRepresentation {

public:
    LayerRAM(uvec2 dimensions = uvec2(32,32), LayerType type = COLOR_LAYER, const DataFormatBase* format = DataVec4UINT8::get());
    LayerRAM(const LayerRAM& rhs);
    LayerRAM& operator=(const LayerRAM& that);
    LayerRAM* clone() const = 0;
    virtual ~LayerRAM();


    virtual void setDimensions(uvec2 dimensions);
    virtual void resize(uvec2 dimensions) = 0;
    virtual bool copyAndResizeLayer(DataRepresentation*) const;

    void* getData();
    const void* getData() const;

    // Takes ownership of data pointer
    void setData(void* data);

    virtual void setValueFromSingleDouble(const uvec2& pos, double val) = 0;
    virtual void setValueFromVec2Double(const uvec2& pos, dvec2 val) = 0;
    virtual void setValueFromVec3Double(const uvec2& pos, dvec3 val) = 0;
    virtual void setValueFromVec4Double(const uvec2& pos, dvec4 val) = 0;

    virtual double getValueAsSingleDouble(const uvec2& pos) const = 0;
    virtual dvec2 getValueAsVec2Double(const uvec2& pos) const = 0;
    virtual dvec3 getValueAsVec3Double(const uvec2& pos) const = 0;
    virtual dvec4 getValueAsVec4Double(const uvec2& pos) const = 0;

    static inline unsigned int posToIndex(const uvec2& pos, const uvec2& dim) {
        return pos.x+(pos.y*dim.x);
    }

protected:
    virtual void initialize() = 0;
    virtual void deinitialize() = 0;

    void* data_;
};

/**
 * Factory for layers.
 * Creates an LayerRAM with data type specified by format.
 *
 * @param dimensionsof layer to create.
 * @param format of layer to create.
 * @return nullptr if no valid format was specified.
 */
IVW_CORE_API LayerRAM* createLayerRAM(const uvec2& dimensions, LayerType type, const DataFormatBase* format);

} // namespace

#endif // IVW_LAYERRAM_H
