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

#include <modules/opencl/volume/volumeclconverter.h>
#include <modules/opencl/volume/volumeclglconverter.h>
#include <modules/opencl/syncclgl.h>
#include <inviwo/core/datastructures/volume/volumerepresentation.h>
#include <modules/opencl/inviwoopencl.h>

namespace inviwo {

VolumeRAM2CLGLConverter::VolumeRAM2CLGLConverter()
    : RepresentationConverterPackage<VolumeCLGL>()
{
    addConverter(new VolumeRAM2GLConverter());
    addConverter(new VolumeGL2CLGLConverter());
}

VolumeCLGL2RAMConverter::VolumeCLGL2RAMConverter()
    : RepresentationConverterType<VolumeRAM>()
{
}


DataRepresentation* VolumeCLGL2RAMConverter::createFrom(const DataRepresentation* source) {
    DataRepresentation* destination = 0;
    const VolumeCLGL* volumeCLGL = static_cast<const VolumeCLGL*>(source);
    const size3_t dimensions{volumeCLGL->getDimensions()};
    destination = createVolumeRAM(dimensions, volumeCLGL->getDataFormat());
    const Texture3D* texture = volumeCLGL->getTexture();

    if (destination) {
        VolumeRAM* volumeRAM = static_cast<VolumeRAM*>(destination);
        texture->download(volumeRAM->getData());
        //const cl::CommandQueue& queue = OpenCL::getPtr()->getQueue();
        //queue.enqueueReadVolume(volumeCL->get(), true, glm::size3_t(0), glm::size3_t(dimensions, 1), 0, 0, volumeRAM->getData());
    } else {
        LogError("Invalid conversion or not implemented");
    }

    return destination;
}

void VolumeCLGL2RAMConverter::update(const DataRepresentation* source, DataRepresentation* destination) {
    const VolumeCLGL* volumeSrc = static_cast<const VolumeCLGL*>(source);
    VolumeRAM* volumeDst = static_cast<VolumeRAM*>(destination);

    if (volumeSrc->getDimensions() != volumeDst->getDimensions()) {
        volumeDst->setDimensions(volumeSrc->getDimensions());
    }

    volumeSrc->getTexture()->download(volumeDst->getData());

    if (volumeDst->hasHistograms())
        volumeDst->getHistograms()->setValid(false);
}


DataRepresentation* VolumeGL2CLGLConverter::createFrom(const DataRepresentation* source) {
    DataRepresentation* destination = 0;
    const VolumeGL* volumeGL = static_cast<const VolumeGL*>(source);
    destination = new VolumeCLGL(volumeGL->getDimensions(), volumeGL->getDataFormat(), const_cast<Texture3D*>(volumeGL->getTexture()));
    return destination;
}

void VolumeGL2CLGLConverter::update(const DataRepresentation* source, DataRepresentation* destination) {
    // Do nothing since they are sharing data
    const VolumeGL* volumeSrc = static_cast<const VolumeGL*>(source);
    VolumeCLGL* volumeDst = static_cast<VolumeCLGL*>(destination);

    if (volumeSrc->getDimensions() != volumeDst->getDimensions()) {
        volumeDst->setDimensions(volumeSrc->getDimensions());
    }
}


DataRepresentation* VolumeCLGL2CLConverter::createFrom(const DataRepresentation* source) {
#ifdef IVW_DEBUG
    LogWarn("Performance warning: Use shared CLGL representation instead of CL ");
#endif
    const VolumeCLGL* volumeCLGL = static_cast<const VolumeCLGL*>(source);
    const size3_t dimensions{volumeCLGL->getDimensions()};
    auto destination = new VolumeCL(dimensions, volumeCLGL->getDataFormat());
    {   SyncCLGL glSync;
        glSync.addToAquireGLObjectList(volumeCLGL);
        glSync.aquireAllObjects();
        OpenCL::getPtr()->getQueue().enqueueCopyImage(volumeCLGL->get(), static_cast<VolumeCL*>(destination)->get(), glm::size3_t(0), glm::size3_t(0),
                glm::size3_t(dimensions));
    }
    return destination;
}

void VolumeCLGL2CLConverter::update(const DataRepresentation* source, DataRepresentation* destination) {
    const VolumeCLGL* volumeSrc = static_cast<const VolumeCLGL*>(source);
    VolumeCL* volumeDst = static_cast<VolumeCL*>(destination);

    if (volumeSrc->getDimensions() != volumeDst->getDimensions()) {
        volumeDst->setDimensions(volumeSrc->getDimensions());
    }

    {   SyncCLGL glSync;
        glSync.addToAquireGLObjectList(volumeSrc);
        glSync.aquireAllObjects();
        OpenCL::getPtr()->getQueue().enqueueCopyImage(volumeSrc->get(), volumeDst->get(), glm::size3_t(0), glm::size3_t(0),
                glm::size3_t(volumeSrc->getDimensions()));
    }
}

DataRepresentation* VolumeCLGL2GLConverter::createFrom(const DataRepresentation* source) {
    DataRepresentation* destination = 0;
    const VolumeCLGL* src = static_cast<const VolumeCLGL*>(source);
    Texture3D* tex = const_cast<Texture3D*>(src->getTexture());
    destination = new VolumeGL(const_cast<Texture3D*>(src->getTexture()), src->getDataFormat());
    // Increase reference count to indicate that LayerGL is also using the texture
    tex->increaseRefCount();
    return destination;
}

void VolumeCLGL2GLConverter::update(const DataRepresentation* source, DataRepresentation* destination) {
    // Do nothing since they share data
}

VolumeCL2CLGLConverter::VolumeCL2CLGLConverter() : RepresentationConverterPackage<VolumeCLGL>() {
    addConverter(new VolumeCL2RAMConverter());
    addConverter(new VolumeRAM2GLConverter());
    addConverter(new VolumeGL2CLGLConverter());
}




} // namespace
