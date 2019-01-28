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

#ifndef IVW_VOLUMECLGL_H
#define IVW_VOLUMECLGL_H

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/volume/volumerepresentation.h>
#include <modules/opencl/inviwoopencl.h>
#include <modules/opencl/openclmoduledefine.h>
#include <modules/opencl/volume/volumeclbase.h>
#include <modules/opengl/texture/texture3d.h>

namespace inviwo {

typedef std::pair<std::shared_ptr<Texture>, std::shared_ptr<cl::Image3DGL>>
    Texture3DCLImageSharingPair;
typedef std::map<std::shared_ptr<Texture>, std::shared_ptr<cl::Image3DGL>> CLTexture3DSharingMap;

/** \class VolumeCLGL
 *
 * VolumeCLGL handles shared texture3D between OpenCL and OpenGL.
 * It will make sure that the texture
 * is not released while a shared representation exist
 * and also release and reattach the shared representation
 * when the texture is resized (handled through the TexturObserver)
 *
 * @see Observable
 */
class IVW_MODULE_OPENCL_API VolumeCLGL : public VolumeCLBase,
                                         public VolumeRepresentation,
                                         public TextureObserver {
public:
    VolumeCLGL(const DataFormatBase* format = DataFormatBase::get(), Texture3D* data = nullptr);
    VolumeCLGL(const size3_t& dimensions, const DataFormatBase* format,
               std::shared_ptr<Texture3D> data);
    VolumeCLGL(const VolumeCLGL& rhs);

    virtual VolumeCLGL* clone() const override;
    virtual ~VolumeCLGL();

    virtual const size3_t& getDimensions() const override;
    virtual void setDimensions(size3_t dimensions) override;

    virtual cl::Image3DGL& getEditable() override;
    virtual const cl::Image3DGL& get() const override;
    std::shared_ptr<Texture3D> getTexture() const;

    /**
     * This method will be called before the texture is initialized.
     * Override it to add behavior.
     */
    virtual void notifyBeforeTextureInitialization() override;

    /**
     * This method will be called after the texture has been initialized.
     * Override it to add behavior.
     */
    virtual void notifyAfterTextureInitialization() override;

    void aquireGLObject(std::vector<cl::Event>* syncEvents = nullptr,
                        const cl::CommandQueue& queue = OpenCL::getPtr()->getQueue()) const;
    void releaseGLObject(std::vector<cl::Event>* syncEvents = nullptr, cl::Event* event = nullptr,
                         const cl::CommandQueue& queue = OpenCL::getPtr()->getQueue()) const;

    virtual std::type_index getTypeIndex() const override final;

protected:
    static CLTexture3DSharingMap clVolumeSharingMap_;
    void initialize();
    void deinitialize();
    size3_t dimensions_;
    std::shared_ptr<Texture3D> texture_;
    std::shared_ptr<cl::Image3DGL> clImage_;  ///< Potentially shared with other LayerCLGL
};

}  // namespace inviwo

namespace cl {

// Kernel argument specializations for VolumeCLGL type
// (enables calling cl::Queue::setArg with VolumeCLGL)
template <>
IVW_MODULE_OPENCL_API cl_int Kernel::setArg(cl_uint index, const inviwo::VolumeCLGL& value);

}  // namespace cl

#endif  // IVW_VOLUMECLGL_H
