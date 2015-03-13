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

#ifndef IVW_VOLUMECLGL_H
#define IVW_VOLUMECLGL_H

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/volume/volumerepresentation.h>
#include <modules/opencl/inviwoopencl.h>
#include <modules/opencl/openclmoduledefine.h>
#include <modules/opencl/volume/volumeclbase.h>
#include <modules/opengl/glwrap/texture3d.h>

namespace inviwo {

class IVW_MODULE_OPENCL_API VolumeCLGL : public VolumeCLBase,
                                         public VolumeRepresentation,
                                         public TextureObserver {
public:
    VolumeCLGL(const DataFormatBase* format = DataFormatBase::get(), Texture3D* data = nullptr);
    VolumeCLGL(const uvec3& dimensions, const DataFormatBase* format, Texture3D* data);
    VolumeCLGL(const VolumeCLGL& rhs);

    virtual VolumeCLGL* clone() const;
    virtual ~VolumeCLGL();
    virtual void initialize(){};
    virtual void deinitialize();

    void initialize(Texture3D* texture);
    virtual const uvec3& getDimensions() const override;
    virtual void setDimensions(uvec3 dimensions) override;

    virtual cl::Image3D& getEditable();
    virtual const cl::Image3D& get() const;
    const Texture3D* getTexture() const;

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

    void aquireGLObject(std::vector<cl::Event>* syncEvents = nullptr,
                        const cl::CommandQueue& queue = OpenCL::getPtr()->getQueue()) const;
    void releaseGLObject(std::vector<cl::Event>* syncEvents = nullptr, cl::Event* event = nullptr,
                         const cl::CommandQueue& queue = OpenCL::getPtr()->getQueue()) const;

protected:
    uvec3 dimensions_;
    Texture3D* texture_;
};

}  // namespace

namespace cl {

// Kernel argument specializations for VolumeCLGL type
// (enables calling cl::Queue::setArg with VolumeCLGL)
template <>
IVW_MODULE_OPENCL_API cl_int Kernel::setArg(cl_uint index, const inviwo::VolumeCLGL& value);

}  // namespace cl

#endif  // IVW_VOLUMECLGL_H
