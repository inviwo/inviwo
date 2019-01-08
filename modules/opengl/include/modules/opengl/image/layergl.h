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

#ifndef IVW_LAYERGL_H
#define IVW_LAYERGL_H

#include <modules/opengl/openglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/texture/textureunit.h>

namespace inviwo {

class Shader;
class Texture2D;

/**
 * \ingroup datastructures
 */
class IVW_MODULE_OPENGL_API LayerGL : public LayerRepresentation {
public:
    LayerGL(size2_t dimensions = size2_t(256, 256), LayerType type = LayerType::Color,
            const DataFormatBase* format = DataVec4UInt8::get(),
            std::shared_ptr<Texture2D> tex = std::shared_ptr<Texture2D>(nullptr),
            const SwizzleMask& swizzleMask = swizzlemasks::rgba);
    LayerGL(const LayerGL& rhs);
    LayerGL& operator=(const LayerGL& rhs);
    virtual ~LayerGL();
    virtual LayerGL* clone() const override;

    virtual void setDimensions(size2_t dimensions) override;

    /**
     * \brief update the swizzle mask of the channels for sampling the layer
     * Needs to be overloaded by child classes.
     *
     * @param mask    new swizzle mask
     */
    virtual void setSwizzleMask(const SwizzleMask& mask) override;
    virtual SwizzleMask getSwizzleMask() const override;

    void bindTexture(GLenum texUnit) const;
    void bindTexture(const TextureUnit& texUnit) const;
    void unbindTexture() const;

    virtual bool copyRepresentationsTo(LayerRepresentation*) const override;

    std::shared_ptr<Texture2D> getTexture() const { return texture_; }
    virtual std::type_index getTypeIndex() const override final;

private:
    std::shared_ptr<Texture2D> texture_;  // Can be shared
    mutable GLenum texUnit_;
    SwizzleMask swizzleMask_;
};

}  // namespace inviwo

#endif  // IVW_LAYERGL_H
