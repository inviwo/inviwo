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

#ifndef IVW_LAYER_H
#define IVW_LAYER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/datastructures/data.h>
#include <inviwo/core/datastructures/spatialdata.h>
#include <inviwo/core/datastructures/image/imagetypes.h>
#include <inviwo/core/datastructures/image/layerrepresentation.h>

namespace inviwo {

class IVW_CORE_API Layer : public Data<LayerRepresentation>, public StructuredGridEntity<2> {
public:
    Layer(size2_t dimensions = size2_t(32, 32), const DataFormatBase* format = DataVec4UINT8::get(),
          LayerType type = LayerType::Color);
    Layer(std::shared_ptr<LayerRepresentation>);
    Layer(const Layer&);
    Layer& operator=(const Layer& that);
    virtual Layer* clone() const override;
    virtual ~Layer();

    virtual size2_t getDimensions() const override;

    /**
     * Resize to dimension. This is destructive, the data will not be
     * preserved. Use copyRepresentationsTo to update the data.
     * @note Resizes the last valid representation and erases all other representations.
     * Last valid representation will remain valid after changing the dimension.
     */
    virtual void setDimensions(const size2_t& dim) override;

    /**
     * Copy and resize the representation of this onto the representations of target.
     * Does not change the dimensions of target.
     */
    void copyRepresentationsTo(Layer* target);

    LayerType getLayerType() const;

protected:
    virtual std::shared_ptr<LayerRepresentation> createDefaultRepresentation() const override;

private:
    LayerType layerType_;
};

}  // namespace

#endif  // IVW_LAYER_H
