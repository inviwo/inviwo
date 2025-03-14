/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2025 Inviwo Foundation
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
#include <inviwo/core/datastructures/datarepresentation.h>
#include <inviwo/core/datastructures/image/imagetypes.h>
#include <inviwo/core/datastructures/image/layerconfig.h>

namespace inviwo {

class Layer;

/**
 * \ingroup datastructures
 */
class IVW_CORE_API LayerRepresentation : public DataRepresentation<Layer> {
public:
    virtual LayerRepresentation* clone() const = 0;
    virtual ~LayerRepresentation() = default;

    LayerType getLayerType() const;

    virtual const DataFormatBase* getDataFormat() const = 0;
    std::string_view getDataFormatString() const { return getDataFormat()->getString(); }
    DataFormatId getDataFormatId() const { return getDataFormat()->getId(); }

    /**
     * Resize the representation to dimension. This is destructive, the data will not be
     * preserved. Use copyRepresentationsTo to update the data.
     * Needs to be overloaded by child classes.
     */
    virtual void setDimensions(size2_t dimensions) = 0;
    virtual const size2_t& getDimensions() const = 0;

    /**
     * \brief update the swizzle mask of the channels for sampling color layers
     * Needs to be overloaded by child classes.
     *
     * @param mask new swizzle mask
     */
    virtual void setSwizzleMask(const SwizzleMask& mask) = 0;
    virtual SwizzleMask getSwizzleMask() const = 0;

    /**
     * \brief update the interpolation for sampling layer
     * Needs to be overloaded by child classes.
     *
     * @param interpolation new interpolation type
     */
    virtual void setInterpolation(InterpolationType interpolation) = 0;
    virtual InterpolationType getInterpolation() const = 0;

    /**
     * \brief Update the wrapping type of the layer
     * Needs to be overloaded by child classes.
     *
     * @param wrapping new wrapping type
     */
    virtual void setWrapping(const Wrapping2D& wrapping) = 0;
    virtual Wrapping2D getWrapping() const = 0;

    /**
     * Copy and resize the representations of this onto the target.
     */
    virtual bool copyRepresentationsTo(LayerRepresentation*) const = 0;

    LayerReprConfig config() const;

protected:
    LayerRepresentation(LayerType type = LayerType::Color);
    LayerRepresentation(const LayerRepresentation& rhs) = default;
    LayerRepresentation& operator=(const LayerRepresentation& that) = default;

    LayerType layerType_;
};

}  // namespace inviwo
