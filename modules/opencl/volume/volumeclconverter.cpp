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
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <modules/opencl/inviwoopencl.h>

namespace inviwo {

VolumeRAM2CLConverter::VolumeRAM2CLConverter()
    : RepresentationConverterType<VolumeCL>()
{}

DataRepresentation* VolumeRAM2CLConverter::createFrom(const DataRepresentation* source) {
    DataRepresentation* destination = 0;
    const VolumeRAM* volumeRAM = static_cast<const VolumeRAM*>(source);
    uvec3 dimensions = volumeRAM->getDimensions();
    const void* data = volumeRAM->getData();
    destination = new VolumeCL(dimensions, volumeRAM->getDataFormat(), data);
    return destination;
}

void VolumeRAM2CLConverter::update(const DataRepresentation* source, DataRepresentation* destination) {
    const VolumeRAM* volumeSrc = static_cast<const VolumeRAM*>(source);
    VolumeCL* volumeDst = static_cast<VolumeCL*>(destination);

    if (volumeSrc->getDimensions() != volumeDst->getDimensions()) {
        volumeDst->setDimensions(volumeSrc->getDimensions());
    }

    volumeDst->upload(volumeSrc->getData());
}

VolumeCL2RAMConverter::VolumeCL2RAMConverter()
    : RepresentationConverterType<VolumeRAM>()
{}


DataRepresentation* VolumeCL2RAMConverter::createFrom(const DataRepresentation* source) {
    DataRepresentation* destination = 0;
    const VolumeCL* volumeCL = static_cast<const VolumeCL*>(source);
    uvec3 dimensions = volumeCL->getDimensions();
    destination = createVolumeRAM(dimensions, volumeCL->getDataFormat());

    if (destination) {
        VolumeRAM* volumeRAM = static_cast<VolumeRAM*>(destination);
        volumeCL->download(volumeRAM->getData());
        //const cl::CommandQueue& queue = OpenCL::getInstance()->getQueue();
        //queue.enqueueReadImage(volumeCL->getVolume(), true, glm::svec3(0), glm::svec3(dimension), 0, 0, volumeRAM->getData());
    }

    return destination;
}

void VolumeCL2RAMConverter::update(const DataRepresentation* source, DataRepresentation* destination) {
    const VolumeCL* volumeSrc = static_cast<const VolumeCL*>(source);
    VolumeRAM* volumeDst = static_cast<VolumeRAM*>(destination);

    if (volumeSrc->getDimensions() != volumeDst->getDimensions()) {
        volumeDst->setDimensions(volumeSrc->getDimensions());
    }

    volumeSrc->download(volumeDst->getData());

    if (volumeDst->hasHistograms())
        volumeDst->getHistograms().setValid(false);
}

} // namespace
