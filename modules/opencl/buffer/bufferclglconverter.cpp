/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

#include <modules/opencl/buffer/bufferclconverter.h>
#include <modules/opencl/buffer/bufferclglconverter.h>
#include <modules/opencl/syncclgl.h>
#include <inviwo/core/datastructures/buffer/bufferrepresentation.h>
#include <modules/opencl/inviwoopencl.h>

namespace inviwo {

BufferRAM2CLGLConverter::BufferRAM2CLGLConverter()
    : RepresentationConverterPackage<BufferCLGL>()
{
    addConverter(new BufferRAM2GLConverter());
    addConverter(new BufferGL2CLGLConverter());
}

BufferCLGL2RAMConverter::BufferCLGL2RAMConverter()
    : RepresentationConverterType<BufferRAM>()
{
}

DataRepresentation* BufferCLGL2RAMConverter::createFrom(const DataRepresentation* source) {
    DataRepresentation* destination = 0;
    const BufferCLGL* src = static_cast<const BufferCLGL*>(source);
    size_t size = src->getSize();
    destination = createBufferRAM(size, src->getDataFormat(), src->getBufferType(), src->getBufferUsage());
    BufferObject* buffer = src->getBufferGL();

    if (destination) {
        BufferRAM* dst = static_cast<BufferRAM*>(destination);
        buffer->download(dst->getData());
    } else {
        LogError("Invalid conversion or not implemented");
    }

    return destination;
}

void BufferCLGL2RAMConverter::update(const DataRepresentation* source, DataRepresentation* destination) {
    const BufferCLGL* src = static_cast<const BufferCLGL*>(source);
    BufferRAM* dst = static_cast<BufferRAM*>(destination);

    if (src->getSize() != dst->getSize()) {
        dst->setSize(src->getSize());
    }

    src->getBufferGL()->download(dst->getData());
}

DataRepresentation* BufferCLGL2GLConverter::createFrom(const DataRepresentation* source) {
    DataRepresentation* destination = 0;
    const BufferCLGL* src = static_cast<const BufferCLGL*>(source);
    BufferObject* data = const_cast<BufferObject*>(src->getBufferGL());
    destination = new BufferGL(src->getSize(), src->getDataFormat(), src->getBufferType(), src->getBufferUsage(), data);
    // Increase reference count to indicate that BufferGL is also using the buffer
    data->increaseRefCount();
    return destination;
}

void BufferCLGL2GLConverter::update(const DataRepresentation* source, DataRepresentation* destination) {
    // Do nothing since they share data
}

DataRepresentation* BufferGL2CLGLConverter::createFrom(const DataRepresentation* source)
{
    DataRepresentation* destination = 0;
    const BufferGL* src = static_cast<const BufferGL*>(source);
    destination = new BufferCLGL(src->getSize(), src->getDataFormat(), src->getBufferType(), src->getBufferUsage(),
                                 const_cast<BufferGL*>(src)->getBufferObject());
    return destination;
}

void BufferGL2CLGLConverter::update(const DataRepresentation* source, DataRepresentation* destination) {
    const BufferGL* src = static_cast<const BufferGL*>(source);
    BufferCLGL* dst = static_cast<BufferCLGL*>(destination);

    if (src->getSize() != dst->getSize()) {
        dst->setSize(src->getSize());
    }

    // FIXME: Do we need to recreate shared CLGL buffer?
}

DataRepresentation* BufferCLGL2CLConverter::createFrom(const DataRepresentation* source)
{
    DataRepresentation* destination = 0;
    const BufferCLGL* src = static_cast<const BufferCLGL*>(source);
    size_t size = src->getSize();
    destination = new BufferCL(size, src->getDataFormat(), src->getBufferType(), src->getBufferUsage());
    {SyncCLGL glSync;
    glSync.addToAquireGLObjectList(src);
    glSync.aquireAllObjects();
    OpenCL::getPtr()->getQueue().enqueueCopyBuffer(src->getBuffer(), static_cast<BufferCL*>(destination)->getBuffer(), 0, 0, src->getSize()*src->getSizeOfElement());
    }
    return destination;
}

void BufferCLGL2CLConverter::update(const DataRepresentation* source, DataRepresentation* destination) {
    const BufferCLGL* src = static_cast<const BufferCLGL*>(source);
    BufferCL* dst = static_cast<BufferCL*>(destination);

    if (src->getSize() != dst->getSize()) {
        dst->setSize(src->getSize());
    }

    {SyncCLGL glSync;
    glSync.addToAquireGLObjectList(src);
    glSync.aquireAllObjects();
    OpenCL::getPtr()->getQueue().enqueueCopyBuffer(src->getBuffer(), dst->getBuffer(), 0, 0, src->getSize()*src->getSizeOfElement());
    }
}

BufferCL2CLGLConverter::BufferCL2CLGLConverter() : RepresentationConverterPackage<BufferCLGL>()
{
    addConverter(new BufferCL2RAMConverter());
    addConverter(new BufferRAM2GLConverter());
    addConverter(new BufferGL2CLGLConverter());
}

} // namespace
