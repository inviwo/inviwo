/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2018 Inviwo Foundation
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

#ifndef IVW_LAYERGLCONVERTER_H
#define IVW_LAYERGLCONVERTER_H

#include <modules/opengl/openglmoduledefine.h>
#include <modules/opengl/image/layergl.h>
#include <inviwo/core/datastructures/image/layerramconverter.h>

namespace inviwo {

class IVW_MODULE_OPENGL_API LayerRAM2GLConverter
    : public RepresentationConverterType<LayerRepresentation, LayerRAM, LayerGL> {
public:
    virtual std::shared_ptr<LayerGL> createFrom(
        std::shared_ptr<const LayerRAM> source) const override;
    virtual void update(std::shared_ptr<const LayerRAM> source,
                        std::shared_ptr<LayerGL> destination) const override;
};

class IVW_MODULE_OPENGL_API LayerGL2RAMConverter
    : public RepresentationConverterType<LayerRepresentation, LayerGL, LayerRAM> {
public:
    virtual std::shared_ptr<LayerRAM> createFrom(
        std::shared_ptr<const LayerGL> source) const override;
    virtual void update(std::shared_ptr<const LayerGL> source,
                        std::shared_ptr<LayerRAM> destination) const override;
};

}  // namespace inviwo

#endif  // IVW_LAYERGLCONVERTER_H
