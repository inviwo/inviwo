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

#ifndef IVW_IMAGEGL_H
#define IVW_IMAGEGL_H

#include <modules/opengl/openglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/image/imagerepresentation.h>
#include <modules/opengl/image/layergl.h>
#include <modules/opengl/buffer/bufferobjectarray.h>
#include <modules/opengl/buffer/framebufferobject.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/openglutils.h>

namespace inviwo {

class Shader;
class Image;

/**
 * \ingroup datastructures
 */
class IVW_MODULE_OPENGL_API ImageGL : public ImageRepresentation {

public:
    ImageGL();
    ImageGL(const ImageGL& rhs);
    virtual ~ImageGL();

    virtual ImageGL* clone() const override;

    void reAttachAllLayers(ImageType type = ImageType::AllLayers);

    void activateBuffer(ImageType type = ImageType::AllLayers);
    void deactivateBuffer();

    virtual size2_t getDimensions() const override;
    virtual bool copyRepresentationsTo(ImageRepresentation* target) const override;
    virtual size_t priority() const override;

    bool copyRepresentationsTo(ImageGL* target) const;
    bool updateFrom(const ImageGL*);

    FrameBufferObject* getFBO();
    const FrameBufferObject* getFBO() const;

    LayerGL* getLayerGL(LayerType, size_t idx = 0);
    const LayerGL* getLayerGL(LayerType, size_t idx = 0) const;

    LayerGL* getColorLayerGL(size_t idx = 0);
    LayerGL* getDepthLayerGL();
    LayerGL* getPickingLayerGL();

    const LayerGL* getColorLayerGL(size_t idx = 0) const;
    const LayerGL* getDepthLayerGL() const;
    const LayerGL* getPickingLayerGL() const;

    size_t getNumberOfColorLayers() const;

    GLenum getPickingAttachmentID() const;

    void updateExistingLayers() const;
    void renderImagePlaneRect() const;

    /**
     * Read a single pixel value out of the specified layer at pos. Should only be used to read
     * single values not entire images.
     */
    virtual dvec4 readPixel(size2_t pos, LayerType layer, size_t index = 0) const override;

    virtual std::type_index getTypeIndex() const override final;
    virtual bool isValid() const override;
    virtual void update(bool editable) override;

private:
    std::vector<LayerGL*> colorLayersGL_;  //< non-owning reference
    LayerGL* depthLayerGL_;                //< non-owning reference
    LayerGL* pickingLayerGL_;              //< non-owning reference

    FrameBufferObject frameBufferObject_;
    GLenum pickingAttachmentID_;

    mutable Shader* shader_ = nullptr;  //< non-owning reference
    mutable size_t colorLayerCopyCount_;

    GLboolean prevDepthTest_;
    GLboolean prevDepthMask_;

    utilgl::Viewport prevViewport_;
};

}  // namespace inviwo

#endif  // IVW_IMAGEGL_H
