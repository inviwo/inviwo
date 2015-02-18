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

#include <inviwo/core/datastructures/buffer/bufferrepresentation.h>
#include <modules/opencl/buffer/bufferclconverter.h>
#include <modules/opencl/buffer/elementbufferclglconverter.h>
#include <modules/opencl/syncclgl.h>
#include <modules/opencl/inviwoopencl.h>
#include <modules/opengl/buffer/elementbufferglconverter.h>

namespace inviwo {

ElementBufferRAM2CLGLConverter::ElementBufferRAM2CLGLConverter()
    : RepresentationConverterPackage<ElementBufferCLGL>()
{
    addConverter(new ElementBufferRAM2GLConverter());
    addConverter(new ElementBufferGL2CLGLConverter());
}

DataRepresentation* ElementBufferCLGL2GLConverter::createFrom(const DataRepresentation* source) {
    DataRepresentation* destination = 0;
    const ElementBufferCLGL* src = static_cast<const ElementBufferCLGL*>(source);
    ElementBufferObject* data = const_cast<ElementBufferObject*>(static_cast<ElementBufferObject*>(src->getBufferGL()));
    destination = new ElementBufferGL(src->getSize(), src->getDataFormat(), src->getBufferType(), src->getBufferUsage(), data);
    // Increase reference count to indicate that ElementBufferGL is also using the buffer
    data->increaseRefCount();
    return destination;
}

void ElementBufferCLGL2GLConverter::update(const DataRepresentation* source, DataRepresentation* destination) {
    // Do nothing since they share data
}

DataRepresentation* ElementBufferGL2CLGLConverter::createFrom(const DataRepresentation* source)
{
    DataRepresentation* destination = 0;
    const ElementBufferGL* src = static_cast<const ElementBufferGL*>(source);
    destination = new ElementBufferCLGL(src->getSize(), src->getDataFormat(), src->getBufferType(), src->getBufferUsage(),
                                 const_cast<ElementBufferGL*>(src)->getBufferObject());
    return destination;
}

void ElementBufferGL2CLGLConverter::update(const DataRepresentation* source, DataRepresentation* destination) {
    const ElementBufferGL* src = static_cast<const ElementBufferGL*>(source);
    ElementBufferCLGL* dst = static_cast<ElementBufferCLGL*>(destination);

    if (src->getSize() != dst->getSize()) {
        dst->setSize(src->getSize());
    }
}

ElementBufferCL2CLGLConverter::ElementBufferCL2CLGLConverter() : RepresentationConverterPackage<ElementBufferCLGL>()
{
    addConverter(new BufferCL2RAMConverter());
    addConverter(new ElementBufferRAM2GLConverter());
    addConverter(new ElementBufferGL2CLGLConverter());
}

} // namespace
