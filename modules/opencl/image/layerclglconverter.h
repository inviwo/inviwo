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

#ifndef IVW_LAYERCLGLCONVERTER_H
#define IVW_LAYERCLGLCONVERTER_H

#include <inviwo/core/datastructures/representationconverter.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layerramconverter.h>
#include <modules/opengl/image/layerglconverter.h>
#include <modules/opencl/inviwoopencl.h>
#include <modules/opencl/image/layercl.h>
#include <modules/opencl/image/layerclgl.h>

namespace inviwo {
class IVW_MODULE_OPENCL_API LayerCLGL2RAMConverter
    : public RepresentationConverterType<LayerCLGL, LayerRAM> {
public:
    virtual std::shared_ptr<LayerRAM> createFrom(
        std::shared_ptr<const LayerCLGL> source) const override;
    virtual void update(std::shared_ptr<const LayerCLGL> source,
                        std::shared_ptr<LayerRAM> destination) const override;
};

class IVW_MODULE_OPENCL_API LayerCLGL2GLConverter
    : public RepresentationConverterType<LayerCLGL, LayerGL> {
public:
    virtual std::shared_ptr<LayerGL> createFrom(
        std::shared_ptr<const LayerCLGL> source) const override;
    virtual void update(std::shared_ptr<const LayerCLGL> source,
                        std::shared_ptr<LayerGL> destination) const override;
};

class IVW_MODULE_OPENCL_API LayerCLGL2CLConverter
    : public RepresentationConverterType<LayerCLGL, LayerCL> {
public:
    virtual std::shared_ptr<LayerCL> createFrom(
        std::shared_ptr<const LayerCLGL> source) const override;
    virtual void update(std::shared_ptr<const LayerCLGL> source,
                        std::shared_ptr<LayerCL> destination) const override;
};

class IVW_MODULE_OPENCL_API LayerGL2CLGLConverter
    : public RepresentationConverterType<LayerGL, LayerCLGL> {
public:
    virtual std::shared_ptr<LayerCLGL> createFrom(
        std::shared_ptr<const LayerGL> source) const override;
    virtual void update(std::shared_ptr<const LayerGL> source,
                        std::shared_ptr<LayerCLGL> destination) const override;
};

}  // namespace

#endif  // IVW_LAYERCLGLCONVERTER_H
