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

#include <modules/opengl/openglmoduledefine.h>  // for IVW_MODULE_OPENGL_API

#include <inviwo/core/datastructures/image/imagetypes.h>           // for LayerType, Interpolati...
#include <inviwo/core/datastructures/image/layerrepresentation.h>  // for LayerRepresentation
#include <inviwo/core/util/formats.h>                              // for DataFormat, DataVec4UInt8
#include <inviwo/core/util/glmvec.h>                               // for size2_t
#include <modules/opengl/inviwoopengl.h>                           // for GLenum

#include <memory>     // for shared_ptr
#include <typeindex>  // for type_index

namespace inviwo {

class Texture2D;
class TextureUnit;

/**
 * \ingroup datastructures
 */
class IVW_MODULE_OPENGL_API LayerGL : public LayerRepresentation {
public:
    explicit LayerGL(std::shared_ptr<Texture2D> tex, LayerType type = LayerConfig::defaultType);
    explicit LayerGL(size2_t dimensions = LayerConfig::defaultDimensions,
                     LayerType type = LayerConfig::defaultType,
                     const DataFormatBase* format = LayerConfig::defaultFormat,
                     const SwizzleMask& swizzleMask = LayerConfig::defaultSwizzleMask,
                     InterpolationType interpolation = LayerConfig::defaultInterpolation,
                     const Wrapping2D& wrapping = LayerConfig::defaultWrapping);
    explicit LayerGL(const LayerReprConfig& config);

    LayerGL(const LayerGL& rhs);
    LayerGL& operator=(const LayerGL& rhs);
    virtual ~LayerGL();
    virtual LayerGL* clone() const override;

    virtual const DataFormatBase* getDataFormat() const override;

    virtual void setDimensions(size2_t dimensions) override;
    virtual const size2_t& getDimensions() const override;

    virtual void setSwizzleMask(const SwizzleMask& mask) override;
    virtual SwizzleMask getSwizzleMask() const override;

    virtual void setInterpolation(InterpolationType interpolation) override;
    virtual InterpolationType getInterpolation() const override;

    virtual void setWrapping(const Wrapping2D& wrapping) override;
    virtual Wrapping2D getWrapping() const override;

    void bindTexture(GLenum texUnit) const;
    void bindTexture(const TextureUnit& texUnit) const;
    void bindImageTexture(const TextureUnit& texUnit, GLenum MEM_ACCESS) const;
    void unbindTexture() const;

    virtual bool copyRepresentationsTo(LayerRepresentation*) const override;

    std::shared_ptr<Texture2D> getTexture() const { return texture_; }
    virtual std::type_index getTypeIndex() const override final;

    virtual void updateResource(const ResourceMeta& meta) const override;

private:
    std::shared_ptr<Texture2D> texture_;  // Can be shared
    mutable GLenum texUnit_ = 0;
};

}  // namespace inviwo
