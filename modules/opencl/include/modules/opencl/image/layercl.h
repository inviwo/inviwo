/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2018 Inviwo Foundation
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

#ifndef IVW_LAYERCL_H
#define IVW_LAYERCL_H

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/image/layerrepresentation.h>
#include <modules/opencl/inviwoopencl.h>
#include <modules/opencl/openclmoduledefine.h>
#include <modules/opencl/image/layerclbase.h>

namespace inviwo {

class IVW_MODULE_OPENCL_API LayerCL : public LayerCLBase, public LayerRepresentation {
public:
    LayerCL(size2_t dimensions = size2_t(128, 128), LayerType type = LayerType::Color,
            const DataFormatBase* format = DataFormatBase::get(),
            const SwizzleMask& swizzleMask = swizzlemasks::rgba, const void* data = nullptr);
    LayerCL(const LayerCL& other);
    virtual ~LayerCL();

    virtual LayerCL* clone() const override;

    void initialize(const void* texels);
    void upload(const void* data);
    /**
     * Download data to preallocated memory.
     *
     * @param data (void *) Preallocated pointer that will contain data after function returns.
     * @return (void)
     */
    void download(void* data) const;
    virtual void setDimensions(size2_t dimensions) override;
    virtual bool copyRepresentationsTo(LayerRepresentation* target) const override;
    cl::ImageFormat getFormat() const { return layerFormat_; }

    virtual cl::Image2D& getEditable() override { return *clImage_; }
    virtual const cl::Image2D& get() const override { return *clImage_; }

    virtual std::type_index getTypeIndex() const override final;

    /**
     * Read a single pixel value out of the specified layer at pos. Should only be used to read
     * single values not entire images.
     */
    virtual dvec4 readPixel(size2_t pos, LayerType layer, size_t index = 0) const;

    /**
     * \brief update the swizzle mask of the channels for sampling color layers
     * Needs to be overloaded by child classes.
     *
     * @param mask    new swizzle mask
     */
    virtual void setSwizzleMask(const SwizzleMask& mask) override;
    virtual SwizzleMask getSwizzleMask() const override;

protected:
    cl::ImageFormat layerFormat_;
    std::unique_ptr<cl::Image2D> clImage_;
    SwizzleMask swizzleMask_;
};

}  // namespace inviwo

namespace cl {

// Kernel argument specializations for LayerCL type
// (enables calling cl::Queue::setArg with LayerCL)
template <>
IVW_MODULE_OPENCL_API cl_int Kernel::setArg(cl_uint index, const inviwo::LayerCL& value);

}  // namespace cl

#endif  // IVW_LAYERCL_H
