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

#ifndef IVW_IMAGEREPRESENTATION_H
#define IVW_IMAGEREPRESENTATION_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/datagrouprepresentation.h>
#include <inviwo/core/datastructures/image/imagetypes.h>

namespace inviwo {

class Image;

/**
 * \ingroup datastructures
 */
class IVW_CORE_API ImageRepresentation : public DataGroupRepresentation<Image> {
public:
    ImageRepresentation() = default;
    ImageRepresentation(const ImageRepresentation& rhs) = default;
    ImageRepresentation& operator=(const ImageRepresentation& that) = default;
    virtual ImageRepresentation* clone() const override = 0;
    virtual ~ImageRepresentation() = default;

    virtual size2_t getDimensions() const = 0;

    /**
     * Copy and resize the representations of this onto the target.
     */
    virtual bool copyRepresentationsTo(ImageRepresentation* target) const = 0;

    /**
     * Returns a number representing the general efficiency of the representation.
     * Larger value means more efficient representation. Used for selection which representation
     * to operate on when resizing for example.
     */
    virtual size_t priority() const = 0;

    /**
     * Read a single pixel value out of the specified layer at pos. Should only be used to read
     * single values not entire images.
     */
    virtual dvec4 readPixel(size2_t pos, LayerType layer, size_t index = 0) const = 0;

    virtual std::type_index getTypeIndex() const override = 0;
    virtual void setOwner(Image* image) override;
    virtual Image* getOwner() override;
    virtual const Image* getOwner() const override;
    virtual bool isValid() const override = 0;
    virtual void update(bool) override = 0;

protected:
    Image* owner_;
};

}  // namespace inviwo

#endif  // IVW_IMAGEREPRESENTATION_H
