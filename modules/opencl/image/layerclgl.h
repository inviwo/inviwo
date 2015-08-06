/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#ifndef IVW_LAYERCLGL_H
#define IVW_LAYERCLGL_H

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/image/layerrepresentation.h>
#include <modules/opencl/inviwoopencl.h>
#include <modules/opencl/openclmoduledefine.h>
#include <modules/opengl/texture/texture2d.h>
#include <modules/opencl/image/layerclbase.h>

namespace inviwo {

typedef std::pair< std::shared_ptr<Texture>, std::shared_ptr<cl::Image2DGL> > TextureCLImageSharingPair;
typedef std::map<std::shared_ptr<Texture>, std::shared_ptr<cl::Image2DGL> > CLTextureSharingMap;

/** \class LayerCLGL
 *
 * LayerCLGL handles shared texture2D between OpenCL and OpenGL.
 * It will make sure that the texture
 * is not released while a shared representation exist
 * and also release and reattach the shared representation
 * when the texture is resized (handled through the TexturObserver)
 *
 * @see Observable
 */
class IVW_MODULE_OPENCL_API LayerCLGL : public LayerCLBase,
                                        public LayerRepresentation,
                                        public TextureObserver {
public:
    LayerCLGL(size2_t dimensions, LayerType type,
        const DataFormatBase* format, std::shared_ptr<Texture2D> data);
    virtual ~LayerCLGL();
    LayerCLGL(const LayerCLGL& rhs);
    virtual LayerCLGL* clone() const;

    void initialize(Texture2D* texture);
    void deinitialize();
    
    virtual void setDimensions(size2_t dimensions) override;
    virtual bool copyRepresentationsTo(DataRepresentation* target) const override;

    virtual cl::Image2DGL& getEditable() { return *clImage_; }
    virtual const cl::Image2DGL& get() const { return *clImage_; }

    std::shared_ptr<Texture2D> getTexture() const { return texture_; }
    /**
    * This method will be called before the texture is initialized.
    * Override it to add behavior.
    */
    virtual void notifyBeforeTextureInitialization();

    /**
    * This method will be called after the texture has been initialized.
    * Override it to add behavior.
    */
    virtual void notifyAfterTextureInitialization();

    void aquireGLObject(std::vector<cl::Event>* syncEvents = nullptr) const {
        std::vector<cl::Memory> syncLayers(1, *clImage_);
        OpenCL::getPtr()->getQueue().enqueueAcquireGLObjects(&syncLayers, syncEvents);
    }
    void releaseGLObject(std::vector<cl::Event>* syncEvents = nullptr,
                         cl::Event* event = nullptr) const {
        std::vector<cl::Memory> syncLayers(1, *clImage_);
        OpenCL::getPtr()->getQueue().enqueueReleaseGLObjects(&syncLayers, syncEvents, event);
    }


protected:
    static CLTextureSharingMap clImageSharingMap_;
    std::shared_ptr<Texture2D> texture_; ///< Shared with LayerGL
    std::shared_ptr<cl::Image2DGL> clImage_; ///< Potentially shared with other LayerCLGL
};

}  // namespace

namespace cl {

// Kernel argument specializations for LayerCLGL type
// (enables calling cl::Queue::setArg with LayerCLGL)
template <>
IVW_MODULE_OPENCL_API cl_int Kernel::setArg(cl_uint index, const inviwo::LayerCLGL& value);

}  // namespace cl

#endif  // IVW_LAYERCLGL_H
