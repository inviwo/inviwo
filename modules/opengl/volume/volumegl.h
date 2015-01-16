/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#ifndef IVW_VOLUMEGL_H
#define IVW_VOLUMEGL_H

#include <modules/opengl/openglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/volume/volumerepresentation.h>
#include <modules/opengl/glwrap/texture3d.h>

namespace inviwo {

class Shader;
class Volume;

class IVW_MODULE_OPENGL_API VolumeGL : public VolumeRepresentation {

public:
    VolumeGL(uvec3 dimensions = uvec3(128,128,128), const DataFormatBase* format = DataFormatBase::get(), bool initializeTexture = true);
    VolumeGL(Texture3D* tex, const DataFormatBase* format);
    VolumeGL(const VolumeGL& rhs);
    VolumeGL& operator=(const VolumeGL& rhs);
    virtual ~VolumeGL();
    virtual VolumeGL* clone() const;

    void bindTexture(GLenum texUnit) const;
    void unbindTexture() const;

    void setDimensions(uvec3 dimensions);

    Texture3D* getTexture();
    const Texture3D* getTexture() const;

private:
    Texture3D* volumeTexture_;
};

} // namespace

#endif // IVW_VOLUMEGL_H
