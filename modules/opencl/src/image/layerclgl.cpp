/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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

#include <modules/opencl/image/layerclgl.h>
#include <inviwo/core/util/assertion.h>
#include <modules/opencl/image/layerclresizer.h>
#include <modules/opencl/syncclgl.h>
#include <modules/opengl/openglutils.h>
#include <inviwo/core/util/formatdispatching.h>
#include <inviwo/core/util/glmutils.h>

namespace inviwo {
CLTextureSharingMap LayerCLGL::clImageSharingMap_;

LayerCLGL::LayerCLGL(std::shared_ptr<Texture2D> data, LayerType type)
    : LayerCLBase(), LayerRepresentation(type), texture_(data) {

    IVW_ASSERT(texture_, "Texture should never be nullptr");

    initialize(texture_.get());
    CLTextureSharingMap::iterator it = LayerCLGL::clImageSharingMap_.find(texture_);

    if (it == LayerCLGL::clImageSharingMap_.end()) {
        clImage_ = std::make_shared<cl::Image2DGL>(
            OpenCL::getPtr()->getContext(), CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, texture_->getID());
        LayerCLGL::clImageSharingMap_.insert(TextureCLImageSharingPair(texture_, clImage_));
    } else {
        clImage_ = it->second;
    }
}

LayerCLGL::LayerCLGL(const LayerCLGL& rhs)
    : LayerCLBase(rhs), LayerRepresentation(rhs), texture_(rhs.texture_->clone()) {

    initialize(texture_.get());
}

LayerCLGL::~LayerCLGL() { deinitialize(); }

const DataFormatBase* LayerCLGL::getDataFormat() const { return texture_->getDataFormat(); }

void LayerCLGL::initialize(Texture2D* texture) {
    ivwAssert(texture != 0, "Cannot initialize with null OpenGL texture");

    // const auto it = std::find_if(LayerCLGL::clImageSharingMap_.begin(),
    // LayerCLGL::clImageSharingMap_.end(),
    //    [texture](const TextureCLImageSharingPair& o) { return  o.first.get() == texture; });
    CLTextureSharingMap::iterator it = LayerCLGL::clImageSharingMap_.find(texture_);

    if (it == LayerCLGL::clImageSharingMap_.end()) {
        clImage_ = std::make_shared<cl::Image2DGL>(
            OpenCL::getPtr()->getContext(), CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, texture->getID());
        LayerCLGL::clImageSharingMap_.insert(TextureCLImageSharingPair(texture_, clImage_));
    } else {
        clImage_ = it->second;
    }

    texture->addObserver(this);
}

void LayerCLGL::deinitialize() {
    // Delete OpenCL image before texture
    const auto it = LayerCLGL::clImageSharingMap_.find(texture_);
    // Release reference
    clImage_.reset();
    if (it != LayerCLGL::clImageSharingMap_.end()) {
        if (it->second.use_count() == 1) {
            LayerCLGL::clImageSharingMap_.erase(it);
        }
    }
}

LayerCLGL* LayerCLGL::clone() const { return new LayerCLGL(*this); }

void LayerCLGL::setDimensions(size2_t dimensions) {
    // Make sure that the OpenCL layer is deleted before resizing the texture
    // By observing the texture we will make sure that the OpenCL layer is
    // deleted and reattached after resizing is done.
    texture_->resize(dimensions);
}

const size2_t& LayerCLGL::getDimensions() const { return texture_->getDimensions(); }

bool LayerCLGL::copyRepresentationsTo(LayerRepresentation* targetRep) const {
    // ivwAssert(false, "Not implemented");
    // Make sure that the OpenCL layer is deleted before resizing the texture
    // TODO: Implement copying in addition to the resizing
    LayerCLGL* target = dynamic_cast<LayerCLGL*>(targetRep);
    const LayerCLGL* source = this;
    try {
        SyncCLGL glSync;
        glSync.addToAquireGLObjectList(target);
        glSync.addToAquireGLObjectList(source);
        glSync.aquireAllObjects();
        LayerCLResizer::resize(source->get(), target->get(), target->getDimensions());
    } catch (const cl::Error& err) {
        LogError(getCLErrorString(err));
        return false;
    }
    return true;
}

void LayerCLGL::notifyBeforeTextureInitialization() {
    const auto it = LayerCLGL::clImageSharingMap_.find(texture_);
    clImage_.reset();
    if (it != LayerCLGL::clImageSharingMap_.end()) {
        if (it->second.use_count() == 1) {
            LayerCLGL::clImageSharingMap_.erase(it);
        }
    }
}

void LayerCLGL::notifyAfterTextureInitialization() {
    const auto it = LayerCLGL::clImageSharingMap_.find(texture_);

    if (it != LayerCLGL::clImageSharingMap_.end()) {
        clImage_ = std::static_pointer_cast<cl::Image2DGL>(it->second);
    } else {
        if (glm::all(glm::greaterThan(texture_->getDimensions(), size2_t(0)))) {
            try {
                clImage_ = std::make_shared<cl::Image2DGL>(OpenCL::getPtr()->getContext(),
                                                           CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0,
                                                           texture_->getID());
                LayerCLGL::clImageSharingMap_.insert(TextureCLImageSharingPair(texture_, clImage_));
            } catch (cl::Error& err) {
                LogOpenCLError(err.err());
                throw err;
            }
        }
    }
}

std::type_index LayerCLGL::getTypeIndex() const { return std::type_index(typeid(LayerCLGL)); }

dvec4 LayerCLGL::readPixel(size2_t pos) const {
    return dispatching::singleDispatch<dvec4, dispatching::filter::All>(
        getDataFormat()->getId(), [&]<typename T>() {
            T res{};

            if constexpr (util::rank_v<T> == 0) {
                OpenCL::getPtr()->getQueue().enqueueReadImage(*clImage_, true, glm::size3_t(pos, 0),
                                                              glm::size3_t(1, 1, 1), 0, 0,
                                                              static_cast<void*>(&res));
            } else {
                OpenCL::getPtr()->getQueue().enqueueReadImage(
                    *clImage_, true, glm::size3_t(pos, 0), glm::size3_t(1, 1, 1), 0, 0,
                    static_cast<void*>(glm::value_ptr(res)));
            }
            return util::glm_convert<dvec4>(res);
        });
}

void LayerCLGL::setSwizzleMask(const SwizzleMask& mask) { texture_->setSwizzleMask(mask); }

SwizzleMask LayerCLGL::getSwizzleMask() const { return texture_->getSwizzleMask(); }

void LayerCLGL::setInterpolation(InterpolationType interpolation) {
    texture_->setInterpolation(interpolation);
}

InterpolationType LayerCLGL::getInterpolation() const { return texture_->getInterpolation(); }

void LayerCLGL::setWrapping(const Wrapping2D& wrapping) {
    texture_->setWrapping(utilgl::convertWrappingToGL(wrapping));
}

Wrapping2D LayerCLGL::getWrapping() const {
    return utilgl::convertWrappingFromGL(texture_->getWrapping());
}

}  // namespace inviwo

namespace cl {

template <>
cl_int Kernel::setArg(cl_uint index, const inviwo::LayerCLGL& value) {
    return setArg(index, value.get());
}

}  // namespace cl
