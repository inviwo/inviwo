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

#include <modules/opencl/image/layerclconverter.h>
#include <modules/opencl/image/layerclglconverter.h>
#include <modules/opencl/syncclgl.h>
#include <inviwo/core/datastructures/image/layerrepresentation.h>
#include <modules/opencl/inviwoopencl.h>

namespace inviwo {

LayerRAM2CLGLConverter::LayerRAM2CLGLConverter()
    : RepresentationConverterPackage<LayerCLGL>() {
    addConverter(new LayerRAM2GLConverter());
    addConverter(new LayerGL2CLGLConverter());
}

LayerCLGL2RAMConverter::LayerCLGL2RAMConverter()
    : RepresentationConverterType<LayerRAM>() {
}


DataRepresentation* LayerCLGL2RAMConverter::createFrom(const DataRepresentation* source) {
    DataRepresentation* destination = 0;
    const LayerCLGL* layerCLGL = static_cast<const LayerCLGL*>(source);
    uvec2 dimensions = layerCLGL->getDimensions();
    destination = createLayerRAM(dimensions, layerCLGL->getLayerType(), layerCLGL->getDataFormat());
    const Texture2D* texture = layerCLGL->getTexture();

    if (destination) {
        LayerRAM* layerRAM = static_cast<LayerRAM*>(destination);
        texture->download(layerRAM->getData());
        //const cl::CommandQueue& queue = OpenCL::getPtr()->getQueue();
        //queue.enqueueReadLayer(layerCL->getLayer(), true, glm::svec3(0), glm::svec3(dimensions, 1), 0, 0, layerRAM->getData());
    } else {
        LogError("Invalid conversion or not implemented");
    }

    return destination;
}

void LayerCLGL2RAMConverter::update(const DataRepresentation* source, DataRepresentation* destination) {
    const LayerCLGL* layerSrc = static_cast<const LayerCLGL*>(source);
    LayerRAM* layerDst = static_cast<LayerRAM*>(destination);

    if (layerSrc->getDimensions() != layerDst->getDimensions()) {
        layerDst->setDimensions(layerSrc->getDimensions());
    }

    layerSrc->getTexture()->download(layerDst->getData());
}

LayerCLGL2GLConverter::LayerCLGL2GLConverter(): RepresentationConverterType<LayerGL>() {
}

DataRepresentation* LayerCLGL2GLConverter::createFrom(const DataRepresentation* source) {
    DataRepresentation* destination = 0;
    const LayerCLGL* src = static_cast<const LayerCLGL*>(source);
    // TODO: Do we need to check if the LayerCLGL texture is valid to use?
    // It should not have been deleted since no LayerGL representation existed.
    Texture2D* tex = const_cast<Texture2D*>(src->getTexture());
    destination = new LayerGL(src->getDimensions(), src->getLayerType(), src->getDataFormat(), const_cast<Texture2D*>(src->getTexture()));
    // Increase reference count to indicate that LayerGL is also using the texture
    tex->increaseRefCount();
    return destination;
}

void LayerCLGL2GLConverter::update(const DataRepresentation* source, DataRepresentation* destination) {
    // Do nothing since they share data
}

DataRepresentation* LayerCLGL2CLConverter::createFrom(const DataRepresentation* source) {
#ifdef IVW_DEBUG
    LogWarn("Performance warning: Use shared CLGL representation instead of CL ");
#endif
    DataRepresentation* destination = 0;
    const LayerCLGL* src = static_cast<const LayerCLGL*>(source);
    destination = new LayerCL(src->getDimensions(), src->getLayerType(), src->getDataFormat());
    {   SyncCLGL glSync;
        src->aquireGLObject(glSync.getGLSyncEvent());
        OpenCL::getPtr()->getQueue().enqueueCopyImage(src->get(), static_cast<LayerCL*>(destination)->get(), glm::svec3(0), glm::svec3(0),
                glm::svec3(src->getDimensions(), 1));
        src->releaseGLObject(nullptr, glSync.getLastReleaseGLEvent());
    }
    return destination;
}

void LayerCLGL2CLConverter::update(const DataRepresentation* source, DataRepresentation* destination) {
    const LayerCLGL* src = static_cast<const LayerCLGL*>(source);
    LayerCL* dst = static_cast<LayerCL*>(destination);

    if (src->getDimensions() != dst->getDimensions()) {
        dst->setDimensions(src->getDimensions());
    }

    {   SyncCLGL glSync;
        src->aquireGLObject(glSync.getGLSyncEvent());
        OpenCL::getPtr()->getQueue().enqueueCopyImage(src->get(), dst->get(), glm::svec3(0), glm::svec3(0), glm::svec3(src->getDimensions(), 1));
        src->releaseGLObject(nullptr, glSync.getLastReleaseGLEvent());
    }
}


DataRepresentation* LayerGL2CLGLConverter::createFrom(const DataRepresentation* source) {
    DataRepresentation* destination = 0;
    const LayerGL* layerGL = static_cast<const LayerGL*>(source);
    destination = new LayerCLGL(layerGL->getDimensions(), layerGL->getLayerType(), layerGL->getDataFormat(),
                                const_cast<Texture2D*>(layerGL->getTexture()));
    return destination;
}

void LayerGL2CLGLConverter::update(const DataRepresentation* source, DataRepresentation* destination) {
    const LayerGL* layerSrc = static_cast<const LayerGL*>(source);
    LayerCLGL* layerDst = static_cast<LayerCLGL*>(destination);

    if (layerSrc->getDimensions() != layerDst->getDimensions()) {
        layerDst->setDimensions(layerSrc->getDimensions());
    }
}

LayerCL2CLGLConverter::LayerCL2CLGLConverter() : RepresentationConverterPackage<LayerCLGL>() {
    addConverter(new LayerCL2RAMConverter());
    addConverter(new LayerRAM2GLConverter());
    addConverter(new LayerGL2CLGLConverter());
}




} // namespace
