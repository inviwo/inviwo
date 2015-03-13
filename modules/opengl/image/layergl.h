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

#ifndef IVW_LAYERGL_H
#define IVW_LAYERGL_H

#include <modules/opengl/openglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/image/layerrepresentation.h>
#include <modules/opengl/glwrap/texture2d.h>

namespace inviwo {

class Shader;
class IVW_MODULE_OPENGL_API LayerGL : public LayerRepresentation {

public:
    LayerGL(uvec2 dimensions = uvec2(256,256), LayerType type = COLOR_LAYER, const DataFormatBase* format = DataVec4UINT8::get(),
            Texture2D* tex = nullptr);
    LayerGL(const LayerGL& rhs);
    LayerGL& operator=(const LayerGL& rhs);
    virtual ~LayerGL();
    virtual LayerGL* clone() const;

    virtual void setDimensions(uvec2 dimensions) { resize(dimensions); }

    void bindTexture(GLenum texUnit) const;
    void unbindTexture() const;

    virtual bool copyAndResizeLayer(DataRepresentation*) const;
    virtual void resize(uvec2 dimensions);

    Texture2D* getTexture();
    const Texture2D* getTexture() const;

protected:
    void initialize();
    void deinitialize();

private:
    Texture2D* texture_;
    mutable GLenum texUnit_;
};

} // namespace

#endif // IVW_LAYERGL_H
