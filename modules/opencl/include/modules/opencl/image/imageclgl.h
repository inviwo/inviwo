/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2018 Inviwo Foundation
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

#ifndef IVW_IMAGECLGL_H
#define IVW_IMAGECLGL_H

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/image/imagerepresentation.h>
#include <modules/opencl/inviwoopencl.h>
#include <modules/opencl/openclmoduledefine.h>
#include <modules/opencl/image/layerclgl.h>

namespace inviwo {

class IVW_MODULE_OPENCL_API ImageCLGL : public ImageRepresentation {
public:
    ImageCLGL();
    ImageCLGL(const ImageCLGL& other);
    virtual ~ImageCLGL();
    virtual ImageCLGL* clone() const override;

    virtual size2_t getDimensions() const override;
    virtual bool copyRepresentationsTo(ImageRepresentation*) const override;
    virtual size_t priority() const override;

    LayerCLGL* getLayerCL();
    const LayerCLGL* getLayerCL() const;

    /**
     * Read a single pixel value out of the specified layer at pos. Should only be used to read
     * single values not entire images.
     */
    virtual dvec4 readPixel(size2_t pos, LayerType layer, size_t index = 0) const override;

    virtual std::type_index getTypeIndex() const override final;
    virtual bool isValid() const override;
    virtual void update(bool) override;

private:
    LayerCLGL* layerCLGL_;
};

}  // namespace inviwo

namespace cl {

// Kernel argument specializations for ImageCLGL type
// Will set the color layer as argument.
// (enables calling cl::Queue::setArg with ImageCLGL)
template <>
IVW_MODULE_OPENCL_API cl_int Kernel::setArg(cl_uint index, const inviwo::ImageCLGL& value);

}  // namespace cl

#endif  // IVW_IMAGECLGL_H