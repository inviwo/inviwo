/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2022 Inviwo Foundation
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

#include <modules/opengl/openglmoduledefine.h>                       // for IVW_MODULE_OPENGL_API

#include <inviwo/core/datastructures/image/imagetypes.h>             // for InterpolationType
#include <inviwo/core/datastructures/volume/volumerepresentation.h>  // for VolumeRepresentation
#include <inviwo/core/util/formats.h>                                // for DataFormatBase
#include <inviwo/core/util/glmvec.h>                                 // for size3_t
#include <modules/opengl/inviwoopengl.h>                             // for GLenum

#include <memory>                                                    // for shared_ptr
#include <typeindex>                                                 // for type_index

namespace inviwo {

class Texture3D;
class Volume;
template <typename DataType, typename Kind> struct representation_traits;

namespace kind {
struct GL {};
}  // namespace kind

/**
 * \ingroup datastructures
 */
class IVW_MODULE_OPENGL_API VolumeGL : public VolumeRepresentation {

public:
    VolumeGL(size3_t dimensions = size3_t(128, 128, 128),
             const DataFormatBase* format = DataFormatBase::get(),
             const SwizzleMask& swizzleMask = swizzlemasks::rgba,
             InterpolationType interpolation = InterpolationType::Linear,
             const Wrapping3D& wrapping = wrapping3d::clampAll, bool initializeTexture = true);

    VolumeGL(std::shared_ptr<Texture3D> tex);
    VolumeGL(const VolumeGL& rhs);
    VolumeGL& operator=(const VolumeGL& rhs);
    virtual ~VolumeGL();
    virtual VolumeGL* clone() const override;

    void bindTexture(GLenum texUnit) const;
    void unbindTexture() const;

    virtual void setDimensions(size3_t dimensions) override;
    virtual const size3_t& getDimensions() const override;

    std::shared_ptr<Texture3D> getTexture() const { return texture_; }
    virtual std::type_index getTypeIndex() const override final;

    /**
     * \brief update the swizzle mask of the color channels when sampling the volume
     *
     * @param mask new swizzle mask
     */
    virtual void setSwizzleMask(const SwizzleMask& mask) override;
    virtual SwizzleMask getSwizzleMask() const override;

    virtual void setInterpolation(InterpolationType interpolation) override;
    virtual InterpolationType getInterpolation() const override;

    virtual void setWrapping(const Wrapping3D& wrapping) override;
    virtual Wrapping3D getWrapping() const override;

private:
    std::shared_ptr<Texture3D> texture_;
};

template <>
struct representation_traits<Volume, kind::GL> {
    using type = VolumeGL;
};

}  // namespace inviwo
