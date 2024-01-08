/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2024 Inviwo Foundation
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

#include <inviwo/core/datastructures/image/imagerepresentation.h>  // for ImageRepresentation
#include <inviwo/core/datastructures/image/imagetypes.h>           // for ImageType, ImageType::...
#include <inviwo/core/util/glmvec.h>                               // for size2_t, dvec4
#include <modules/opengl/buffer/framebufferobject.h>               // for FrameBufferObject
#include <modules/opengl/inviwoopengl.h>                           // for GLenum
#include <modules/opengl/openglutils.h>                            // for DepthMaskState

#include <cstddef>    // for size_t
#include <optional>   // for optional
#include <tuple>      // for tuple
#include <typeindex>  // for type_index
#include <vector>     // for vector

namespace inviwo {

class LayerGL;
class Shader;

/**
 * \ingroup datastructures
 * @brief OpenGL representation of an Image as a framebuffer
 *
 * Handles attaching all the image layers to the framebuffer
 * @see Image, ImageRepresentation
 */
class IVW_MODULE_OPENGL_API ImageGL : public ImageRepresentation {

public:
    ImageGL();
    ImageGL(const ImageGL& rhs);
    virtual ~ImageGL();
    virtual ImageGL* clone() const override;

    using ActiveState = std::tuple<utilgl::ActivateFBO, utilgl::GlBoolState, utilgl::DepthMaskState,
                                   utilgl::ViewportState>;

    /**
     * @brief Store the current "state" and activate the FBO.
     *
     * Stores the currently bound FBO, the current depth test, depth mask, and viewport.
     * Binds the framebuffer.
     * Sets the draw buffers. Color layer 0 will be at location 0, if picking is included in the \p
     * imageType the picking layer will be at location 1. Any additional layers will be at the
     * following locations. If \p imageType contains depth the depth test is enabled and the depth
     * mask is set to true, otherwise the depth test is disabled and the depth mask is set to false.
     * Finally the viewport is the to the image dimensions.
     * @param imageType The layers that should be active.
     * @return An RAII object that will restore the old state on destruction.
     */
    ActiveState activate(ImageType imageType = ImageType::AllLayers);

    /**
     * @brief Active the FBO.
     *
     * Binds the framebuffer, and set the draw buffers. Color layer 0 will be at location 0, if
     * picking is included in the \p imageType the picking layer will be at location 1. Any
     * additional layers will be at the following locations. If \p imageType contains depth the
     * depth test is enabled and the depth mask is set to true, otherwise the depth test is disabled
     * and the depth mask is set to false. Finally the viewport is the to the image dimensions.
     * @note In the case of nested calls where the state needs to be maintained @see
     * activate(ImageType)
     * @param imageType The layers that should be active.
     */
    void activateBuffer(ImageType imageType = ImageType::AllLayers);

    /**
     * @brief Deactivate the framebuffer (bind 0)
     */
    void deactivateBuffer();

    virtual size2_t getDimensions() const override;

    /**
     * @brief Copies this instance into the target using a copy shader
     *
     * The image content is scaled to the target dimensions. If the aspects are different the target
     * image will get padding to preserve the aspect of the source content.
     * @param target image to copy into.
     * @return true
     */
    virtual bool copyRepresentationsTo(ImageRepresentation* target) const override;

    virtual size_t priority() const override;

    /**
     * @copydoc copyRepresentationsTo(ImageRepresentation* target) const
     */
    bool copyRepresentationsTo(ImageGL* target) const;

    /**
     * @brief Copies @p source into this instance using Blitting and PBOs if needed.
     *
     * Does not care about aspect ratios, copies the source image into the this instance using
     * "nearest" interpolation. If some layers are not attached to the FBO they will be copied using
     * PBO instead.
     * @param source Image to copy from
     * @return true
     */
    bool updateFrom(const ImageGL* source);

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

    /**
     * Read a single pixel value out of the specified layer at pos. Should only be used to read
     * single values not entire images.
     */
    virtual dvec4 readPixel(size2_t pos, LayerType layer, size_t index = 0) const override;

    virtual std::type_index getTypeIndex() const override final;
    virtual bool isValid() const override;
    virtual void update(bool editable) override;

private:
    void reAttachAllLayers();

    std::vector<LayerGL*> colorLayersGL_;  //< non-owning reference
    LayerGL* depthLayerGL_;                //< non-owning reference
    LayerGL* pickingLayerGL_;              //< non-owning reference

    FrameBufferObject frameBufferObject_;

    std::vector<GLenum> colorAttachmentIDs_;
    std::optional<GLenum> pickingAttachmentID_;
    std::vector<GLenum> colorAndPickingAttachmentIDs_;

    mutable Shader* shader_ = nullptr;  //< non-owning reference
    mutable size_t colorLayerCopyCount_;
};

}  // namespace inviwo
